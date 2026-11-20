#include <gtest/gtest.h>

#include "plugins/data/BiochemicalSimulation/GroProgramCompiler.h"
#include "plugins/data/BiochemicalSimulation/GroProgramParser.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::vector<std::filesystem::path> fixtureFiles() {
    const std::filesystem::path fixtureDirectory =
        std::filesystem::path(GENESYS_SOURCE_DIR) / "models" / "genesys_gro_examples";
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(fixtureDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gro") {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::string readFixture(const std::filesystem::path& path) {
    std::ifstream file(path);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

} // namespace

TEST(GenesysGroFixtures, EveryAuthoredFixtureParsesAndCompiles) {
    const std::vector<std::filesystem::path> files = fixtureFiles();
    ASSERT_EQ(files.size(), 3u);

    GroProgramParser parser;
    GroProgramCompiler compiler;
    for (const std::filesystem::path& path : files) {
        SCOPED_TRACE(path.string());
        const GroProgramParser::Result parsed = parser.parse(readFixture(path));
        ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

        const GroProgramIr ir = compiler.compile(parsed.ast);
        EXPECT_FALSE(ir.commands.empty());
        EXPECT_TRUE(ir.hasNamedProgram("main"));
    }
}
