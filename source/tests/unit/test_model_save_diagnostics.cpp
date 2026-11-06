// Failure-stage characterization for the kernel model-save pipeline.
// All filesystem mutations are confined to a private temporary directory.
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>
#include <system_error>
#include <unistd.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/persistence/GenSerializer.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/Transfer/Label.h"
#include "plugins/components/Transfer/Goto.h"
#include "plugins/data/Transfer/LabelDefinition.h"

namespace {
namespace fs = std::filesystem;

struct SaveSandbox {
    fs::path root;
    SaveSandbox() {
        static unsigned counter = 0;
        root = fs::temp_directory_path() /
            ("genesys-save-diagnostic-" + std::to_string(::getpid()) +
             "-" + std::to_string(++counter));
        fs::create_directories(root);
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
