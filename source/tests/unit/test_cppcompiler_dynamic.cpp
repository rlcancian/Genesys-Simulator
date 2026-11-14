#include <gtest/gtest.h>

#include <dlfcn.h>
#include <unistd.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/ModelManager.h"
#include "plugins/data/ExternalIntegration/CppCompiler.h"
#include "plugins/components/Continuous/DiffEquations.h"

namespace {
namespace fs = std::filesystem;

class ScopedTempDirectory {
public:
    ScopedTempDirectory() {
        std::string pattern = (fs::temp_directory_path() / "genesys-cppcompiler space-XXXXXX").string();
        std::vector<char> name(pattern.begin(), pattern.end());
        name.push_back('\0');
        char* created = ::mkdtemp(name.data());
        if (created) _path = created;
    }
    ~ScopedTempDirectory() {
        if (!_path.empty()) {
            std::error_code error;
            fs::remove_all(_path, error);
        }
    }
    const fs::path& path() const { return _path; }
private:
    fs::path _path;
};

void writeSource(const fs::path& path, const std::string& code) {
    std::ofstream output(path);
    ASSERT_TRUE(output.is_open());
    output << code;
    ASSERT_TRUE(output.good());
}
} // namespace

TEST(CppCompilerDynamicTest, CompilesLoadsResolvesAndUnloadsLibrary) {
    ScopedTempDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const fs::path source = scratch.path() / "with spaces.cpp";
    const fs::path includeDir = scratch.path() / "include files";
    fs::create_directories(includeDir);
    writeSource(includeDir / "test_value.h", "inline constexpr int TEST_VALUE = 42;\n");
    writeSource(source, "#include \"test_value.h\"\nextern \"C\" int genesys_test_value() { return TEST_VALUE; }\n");

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    auto* compiler = new CppCompiler(model, "test_cppcompiler");
    compiler->setSourceFilename(source.string());
    compiler->setOutputDir(scratch.path().string());
    compiler->setTempDir(scratch.path().string());
    compiler->setOutputFilename("test_plugin.so");
    compiler->setFlagsGeneral(compiler->getFlagsGeneral() + " -I\"" + includeDir.string() + "\"");
    const auto compilation = compiler->compileToDynamicLibrary();
    ASSERT_TRUE(compilation.success) << compilation.compilationErrOutput;
    EXPECT_TRUE(fs::is_regular_file(scratch.path() / "test_plugin.so"));

    std::string error;
    ASSERT_TRUE(compiler->loadLibrary(error)) << error;
    ASSERT_TRUE(compiler->IsLibraryLoaded());
    auto function = reinterpret_cast<int (*)()>(::dlsym(compiler->getDynamicLibraryHandler(), "genesys_test_value"));
    ASSERT_NE(function, nullptr);
    EXPECT_EQ(function(), 42);
    EXPECT_FALSE(compiler->loadLibrary(error)) << "Repeated loads must not lose the original handle";
    EXPECT_TRUE(compiler->unloadLibrary());
    EXPECT_FALSE(compiler->IsLibraryLoaded());
    EXPECT_TRUE(compiler->unloadLibrary());
}

TEST(CppCompilerDynamicTest, RejectsInvalidLibraryAndReportsMissingSymbol) {
    ScopedTempDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const fs::path source = scratch.path() / "no_export.cpp";
    writeSource(source, "extern \"C\" int present_symbol() { return 9; }\n");

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    auto* compiler = new CppCompiler(model, "symbol_failure");
    compiler->setSourceFilename(source.string());
    compiler->setOutputDir(scratch.path().string());
    compiler->setTempDir(scratch.path().string());
    compiler->setOutputFilename("library.so");
    ASSERT_TRUE(compiler->compileToDynamicLibrary().success);
    std::string error;
    ASSERT_TRUE(compiler->loadLibrary(error)) << error;
    ::dlerror();
    EXPECT_EQ(::dlsym(compiler->getDynamicLibraryHandler(), "missing_symbol"), nullptr);
    EXPECT_NE(::dlerror(), nullptr);
    ASSERT_TRUE(compiler->unloadLibrary());

    writeSource(scratch.path() / "library.so", "not an ELF shared object\n");
    error.clear();
    EXPECT_FALSE(compiler->loadLibrary(error));
    EXPECT_FALSE(error.empty());
    EXPECT_FALSE(compiler->IsLibraryLoaded());
}

TEST(CppCompilerDynamicTest, RejectsSyntaxErrorsAndNeverAcceptsStaleArtifact) {
    ScopedTempDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const fs::path source = scratch.path() / "code.cpp";
    writeSource(source, "extern \"C\" int genesys_test_value() { return 5; }\n");

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    auto* compiler = new CppCompiler(model, "syntax_test");
    compiler->setSourceFilename(source.string());
    compiler->setOutputDir(scratch.path().string());
    compiler->setTempDir(scratch.path().string());
    compiler->setOutputFilename("artifact.so");
    ASSERT_TRUE(compiler->compileToDynamicLibrary().success);

    writeSource(source, "int definitely_invalid( {\n");
    const auto failure = compiler->compileToDynamicLibrary();
    EXPECT_FALSE(failure.success);
    EXPECT_FALSE(failure.compilationErrOutput.empty());
    EXPECT_FALSE(fs::exists(scratch.path() / "artifact.so"));
}

TEST(CppCompilerDynamicTest, FailsForUnknownCompilerWithoutExecutingShell) {
    ScopedTempDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const fs::path source = scratch.path() / "code.cpp";
    const fs::path shellMarker = scratch.path() / "shell-was-used";
    writeSource(source, "int value() { return 5; }\n");

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    auto* compiler = new CppCompiler(model, "missing_compiler");
    compiler->setSourceFilename(source.string());
    compiler->setOutputDir(scratch.path().string());
    compiler->setTempDir(scratch.path().string());
    compiler->setOutputFilename("artifact.so");
    compiler->setCompilerCommand("definitely-missing-genesys-compiler; touch " + shellMarker.string());
    const auto failure = compiler->compileToDynamicLibrary();
    EXPECT_FALSE(failure.success);
    EXPECT_FALSE(failure.compilationErrOutput.empty());
    EXPECT_FALSE(fs::exists(scratch.path() / "artifact.so"));
    EXPECT_FALSE(fs::exists(shellMarker));
}


TEST(CppCompilerDynamicTest, DiffEquationsCanRecompileAfterPreviousLibraryWasLoaded) {
    ScopedTempDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const fs::path previousCwd = fs::current_path();
    struct RestoreCwd {
        fs::path previous;
        ~RestoreCwd() { std::error_code error; fs::current_path(previous, error); }
    } restore{previousCwd};
    fs::current_path(scratch.path());

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    auto* equations = new DiffEquations(model, "diff_recheck");
    equations->getEquations()->insert("-x0");
    std::string error;
    ASSERT_TRUE(ModelDataDefinition::Check(equations, error)) << error;
    ASSERT_NE(equations->getCppCompiler(), nullptr);
    ASSERT_NE(equations->getCppCompiler()->getDynamicLibraryHandler(), nullptr);

    error.clear();
    EXPECT_TRUE(ModelDataDefinition::Check(equations, error)) << error;
    EXPECT_NE(equations->getCppCompiler()->getDynamicLibraryHandler(), nullptr);
}
