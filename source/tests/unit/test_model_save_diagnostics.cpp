// Failure-stage characterization for the kernel model-save pipeline.
// All filesystem mutations are confined to a private temporary directory.
#include <gtest/gtest.h>

#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <system_error>
#include <vector>
#include <unistd.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/persistence/GenSerializer.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/Transfer/Label.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/Synchronization/Wait.h"
#include "plugins/components/Synchronization/Signal.h"
#include "plugins/components/ExternalIntegration/CppForG.h"
#include "plugins/components/Transfer/Goto.h"
#include "plugins/data/Transfer/LabelDefinition.h"

namespace {
namespace fs = std::filesystem;

struct SaveSandbox {
    fs::path root;
    SaveSandbox() {
        // mkdtemp creates a private directory atomically (0700); do not use
        // predictable PID-only paths under a shared temporary directory.
        std::string pattern = (fs::temp_directory_path() / "genesys-save-diagnostic-XXXXXX").string();
        std::vector<char> buffer(pattern.begin(), pattern.end());
        buffer.push_back('\0');
        const char* created = ::mkdtemp(buffer.data());
        if (created == nullptr) {
            throw std::system_error(errno, std::generic_category(), "mkdtemp");
        }
        root = created;
    }
    ~SaveSandbox() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
};

void writeFile(const fs::path& path, const std::string& contents) {
    std::ofstream file(path, std::ios::binary);
    ASSERT_TRUE(file.is_open());
    file << contents;
    ASSERT_TRUE(file.good());
}

std::string readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

bool hasDiagnostic(Model* model, const std::string& code) {
    for (const auto& message : *model->getTracer()->errorMessages()->list()) {
        if (message.find(code) != std::string::npos) return true;
    }
    return false;
}

// Fault-injection probe: a Resource subtype that fails inside the existing
// ModelDataDefinition::SaveInstance exception-catching boundary.
class ThrowingResource final : public Resource {
public:
    using Resource::Resource;
protected:
    void _saveInstance(PersistenceRecord*, bool) override {
        throw std::runtime_error("injected SaveInstance failure");
    }
};

class RejectingStreamBuf final : public std::streambuf {
protected:
    int_type overflow(int_type) override { return traits_type::eof(); }
    std::streamsize xsputn(const char*, std::streamsize) override { return 0; }
};
} // namespace

TEST(ModelSaveDiagnostics, ExistingTemporaryDirectoryIsReportedAsOpenFailure) {
    SaveSandbox sandbox;
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    const fs::path destination = sandbox.root / "previous.gen";
    writeFile(destination, "PREVIOUS_VALID_SAVE");
    const fs::path temporary = destination.string() + ".tmp";
    fs::create_directories(temporary);
    writeFile(temporary / "keep-me", "unrelated");
    EXPECT_FALSE(model->save(destination.string()));
    EXPECT_TRUE(hasDiagnostic(model, "SAVE_DIAGNOSTIC code=FILE_OPEN_FAILED"));
    EXPECT_EQ(readFile(destination), "PREVIOUS_VALID_SAVE");
    EXPECT_TRUE(fs::exists(temporary / "keep-me"));
}

TEST(ModelSaveDiagnostics, MissingParentDirectoryIsOpenFailure) {
    SaveSandbox sandbox;
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EXPECT_FALSE(model->save((sandbox.root / "missing" / "model.gen").string()));
    EXPECT_TRUE(hasDiagnostic(model, "SAVE_DIAGNOSTIC code=FILE_OPEN_FAILED"));
}

TEST(ModelSaveDiagnostics, RenameFailurePreservesExistingDestinationDirectory) {
    SaveSandbox sandbox;
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    const fs::path destination = sandbox.root / "already-directory.gen";
    fs::create_directories(destination);
    writeFile(destination / "keep-me", "PREVIOUS_CONTENT");
    EXPECT_FALSE(model->save(destination.string()));
    EXPECT_TRUE(hasDiagnostic(model, "SAVE_DIAGNOSTIC code=FILE_RENAME_FAILED"));
    EXPECT_EQ(readFile(destination / "keep-me"), "PREVIOUS_CONTENT");
    EXPECT_FALSE(fs::exists(destination.string() + ".tmp"));
}

TEST(ModelSaveDiagnostics, MissingPluginIsLogicalFailureNotFilesystemFailure) {
    SaveSandbox sandbox;
    Simulator simulator;
    // Intentionally do not auto-register plugins: Resource exists in the model
    // but its typename has no corresponding serializer plugin in this fixture.
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    new Resource(model, "WithoutRegisteredResourcePlugin");
    const fs::path destination = sandbox.root / "existing.gen";
    writeFile(destination, "PREVIOUS_VALID_SAVE");
    EXPECT_FALSE(model->save(destination.string()));
    EXPECT_TRUE(hasDiagnostic(model, "SERIALIZER_PLUGIN_NOT_FOUND"));
    EXPECT_TRUE(hasDiagnostic(model, "SAVE_DIAGNOSTIC code=SERIALIZER_DUMP_FAILED"));
    EXPECT_FALSE(hasDiagnostic(model, "SAVE_DIAGNOSTIC code=FILE_OPEN_FAILED"));
    EXPECT_EQ(readFile(destination), "PREVIOUS_VALID_SAVE");
    EXPECT_FALSE(fs::exists(destination.string() + ".tmp"));
}

TEST(ModelSaveDiagnostics, UnknownTypenameIsReportedByGenSerializer) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GenSerializer serializer(model);
    auto record = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
    record->saveField("name", "unknown");
    ASSERT_TRUE(serializer.put("unknown", "DeliberatelyMissingPlugin", 999u, record.get()));
    std::ostringstream output;
    EXPECT_FALSE(serializer.dump(output));
    EXPECT_TRUE(output.good()) << "serializer failure is logical, not a stream failure";
    EXPECT_TRUE(hasDiagnostic(model, "SERIALIZER_PLUGIN_NOT_FOUND"));
}

TEST(ModelSaveDiagnostics, RejectedOutputStreamIsReportedAsWriteFailure) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GenSerializer serializer(model);
    RejectingStreamBuf sink;
    std::ostream output(&sink);
    EXPECT_FALSE(serializer.dump(output));
    EXPECT_FALSE(output.good());
    EXPECT_TRUE(hasDiagnostic(model, "SERIALIZER_STREAM_WRITE_FAILED"));
}

TEST(ModelSaveDiagnostics, RepeatedIncompleteSaveAndGuiKernelIntermediateWork) {
    SaveSandbox sandbox;
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    new Resource(model, "IndependentResource");
    auto* incomplete = new Decide(model, "IncompleteDecision");
    incomplete->addConditions("unfinished +");
    const fs::path destination = sandbox.root / "nome especial á [1].gui.tmp";
    ASSERT_TRUE(model->save(destination.string()));
    const std::string first = readFile(destination);
    EXPECT_NE(first.find("IndependentResource"), std::string::npos);
    EXPECT_NE(first.find("IncompleteDecision"), std::string::npos);
    EXPECT_FALSE(fs::exists(destination.string() + ".tmp"));
    ASSERT_TRUE(model->save(destination.string()));
    EXPECT_EQ(readFile(destination), first);
    EXPECT_FALSE(fs::exists(destination.string() + ".tmp"));
}

TEST(ModelSaveDiagnostics, DeviceFullInducesStreamFailureWithoutFillingDisk) {
    if (!fs::exists("/dev/full")) GTEST_SKIP() << "Linux /dev/full is unavailable";
    SaveSandbox sandbox;
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    const fs::path destination = sandbox.root / "existing.gen";
    writeFile(destination, "PREVIOUS_VALID_SAVE");
    const fs::path temporary = destination.string() + ".tmp";
    std::error_code ec;
    fs::create_symlink("/dev/full", temporary, ec);
    if (ec) GTEST_SKIP() << "Symlink creation unavailable: " << ec.message();
    EXPECT_FALSE(model->save(destination.string()));
    EXPECT_TRUE(hasDiagnostic(model, "SAVE_DIAGNOSTIC code=FILE_WRITE_FAILED") ||
                hasDiagnostic(model, "SAVE_DIAGNOSTIC code=FILE_FLUSH_FAILED"));
    EXPECT_EQ(readFile(destination), "PREVIOUS_VALID_SAVE");
}


TEST(ModelSaveDiagnostics, TransferGraphDeletionAndRecreationRemainsSavableWithoutModelCheck) {
    SaveSandbox sandbox;
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    const fs::path destination = sandbox.root / "transfer-graph.gen";

    auto* receiver = new Label(model, "Receiver");
    auto* sender = new Goto(model, "Sender");
    auto* definition = receiver->getLabelDefinition();
    ASSERT_NE(definition, nullptr);
    sender->setLabelDefinition(definition);
    ASSERT_TRUE(model->save(destination.string()));
    EXPECT_NE(readFile(destination).find("Receiver"), std::string::npos);

    delete receiver;
    ASSERT_EQ(sender->getLabelDefinition(), nullptr);
    // The model is intentionally incomplete, not invalid for persistence.
    ASSERT_TRUE(model->save(destination.string()));
    EXPECT_EQ(readFile(destination).find("\"Receiver\""), std::string::npos);

    auto* recreated = new Label(model, "ReceiverRecreated");
    ASSERT_NE(recreated->getLabelDefinition(), nullptr);
    sender->setLabelDefinition(recreated->getLabelDefinition());
    ASSERT_TRUE(model->save(destination.string()));
    ASSERT_TRUE(model->save(destination.string()));
    EXPECT_NE(readFile(destination).find("ReceiverRecreated"), std::string::npos);
    EXPECT_FALSE(fs::exists(destination.string() + ".tmp"));
}


TEST(ModelSaveDiagnostics, ConsolePluginInsertRemoveReinsertAcrossMultipleSaves) {
    SaveSandbox sandbox;
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    const fs::path destination = sandbox.root / "console-sequence.gen";

    auto* source = new Create(model, "CreateSource");
    auto* sink = new Dispose(model, "DisposeSink");
    source->connectTo(sink);
    ASSERT_TRUE(model->save(destination.string()));
    EXPECT_NE(readFile(destination).find("CreateSource"), std::string::npos);

    auto* wait = new Wait(model, "WaitTemporary");
    auto* signal = new Signal(model, "SignalTemporary");
    auto* cpp = new CppForG(model, "CppForGTemporary");
    cpp->setOnDispatchEventCode("// source is not executed on save\nint unused = 1;");
    ASSERT_TRUE(model->save(destination.string()));
    EXPECT_NE(readFile(destination).find("WaitTemporary"), std::string::npos);
    EXPECT_NE(readFile(destination).find("SignalTemporary"), std::string::npos);

    // Remove only standalone elements, without deliberately leaving a kernel
    // connection dangling (that is a different, ownership-sensitive test).
    delete cpp;
    delete wait;
    delete signal;
    ASSERT_TRUE(model->save(destination.string()));
    const std::string withoutRemoved = readFile(destination);
    EXPECT_EQ(withoutRemoved.find("\"WaitTemporary\""), std::string::npos);
    EXPECT_EQ(withoutRemoved.find("\"SignalTemporary\""), std::string::npos);
    EXPECT_EQ(withoutRemoved.find("\"CppForGTemporary\""), std::string::npos);

    wait = new Wait(model, "WaitRecreated");
    signal = new Signal(model, "SignalRecreated");
    cpp = new CppForG(model, "CppForGRecreated");
    ASSERT_TRUE(model->save(destination.string()));
    const std::string recreated = readFile(destination);
    EXPECT_NE(recreated.find("WaitRecreated"), std::string::npos);
    EXPECT_NE(recreated.find("SignalRecreated"), std::string::npos);
    EXPECT_NE(recreated.find("CppForGRecreated"), std::string::npos);
    EXPECT_FALSE(fs::exists(destination.string() + ".tmp"));
}


TEST(ModelSaveDiagnostics, CharacterizesSwallowedSaveInstanceErrorWithoutMaskingIt) {
    SaveSandbox sandbox;
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    new ThrowingResource(model, "InjectedFailingResource");
    const fs::path destination = sandbox.root / "prior.gen";
    writeFile(destination, "PREVIOUS_VALID_SAVE");

    // CHARACTERIZATION OF KNOWN DEFECT, not a successful-save contract:
    // SaveInstance catches the injected exception and returns void. Existing
    // persistence historically treats the resulting partial record as valid.
    const bool returnedSuccess = model->save(destination.string());
    EXPECT_TRUE(hasDiagnostic(model, "SERIALIZER_RECORD_SAVE_REPORTED_ERROR"));
    EXPECT_TRUE(returnedSuccess) << "The bug is that a swallowed field-save error "
                                  "can be reported as successful persistence.";
    EXPECT_NE(readFile(destination), "PREVIOUS_VALID_SAVE")
        << "Current code replaces a prior save after a swallowed record error.";
    // Phase 2 must invert this contract, preserve the destination, and revise
    // this characterization into a normal regression test.
}
