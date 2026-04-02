#include <gtest/gtest.h>

#include "kernel/simulator/ParserManager.h"

TEST(SupportParserManagerClassTest, ResultDefaultsToFailureAndEmptyArtifacts) {
    ParserManager::GenerateNewParserResult result;

    EXPECT_FALSE(result.result);
    EXPECT_EQ(result.bisonMessages, "");
    EXPECT_EQ(result.lexMessages, "");
    EXPECT_EQ(result.compilationMessages, "");
    EXPECT_EQ(result.newParser.bisonFilename, "");
    EXPECT_EQ(result.newParser.flexFilename, "");
    EXPECT_EQ(result.newParser.compiledParserFilename, "");
}

TEST(SupportParserManagerClassTest, NewParserStartsWithEmptyPaths) {
    ParserManager::NewParser parser;

    EXPECT_EQ(parser.bisonFilename, "");
    EXPECT_EQ(parser.flexFilename, "");
    EXPECT_EQ(parser.compiledParserFilename, "");
}
