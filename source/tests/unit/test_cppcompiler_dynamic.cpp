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

namespace {
namespace fs = std::filesystem;

class ScopedTempDirectory {
public:
    ScopedTempDirectory() {
        std::string pattern = (fs::temp_directory_path() / "genesys-cppcompiler-XXXXXX").string();
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
    writeSource(source, "extern \"C\" int genesys_test_value() { return 42; }\n");

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    auto* compiler = new CppCompiler(model, "test_cppcompiler");
    compiler->setSourceFilename(source.string());
    compiler->setOutputDir(scratch.path().string());
    compiler->setTempDir(scratch.path().string());
    compiler->setOutputFilename("test_plugin.so");
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
    writeSource(source, "int value() { return 5; }\n");

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    auto* compiler = new CppCompiler(model, "missing_compiler");
    compiler->setSourceFilename(source.string());
    compiler->setOutputDir(scratch.path().string());
    compiler->setTempDir(scratch.path().string());
    compiler->setOutputFilename("artifact.so");
    compiler->setCompilerCommand("definitely-missing-genesys-compiler");
    const auto failure = compiler->compileToDynamicLibrary();
    EXPECT_FALSE(failure.success);
    EXPECT_FALSE(failure.compilationErrOutput.empty());
    EXPECT_FALSE(fs::exists(scratch.path() / "artifact.so"));
}
