#include <gtest/gtest.h>

#include "applications/distributed/core/DistributedConfigLoader.h"

#include <string>
#include <vector>

using namespace genesys::distributed;

TEST(DistributedConfigLoader, ShouldParseValidJsonConfig) {
    // Arrange
    const std::string json =
        "{\"modelFile\":\"model.txt\",\"outputFile\":\"out.json\",\"totalReplications\":30,"
        "\"includeLocal\":true,\"maxRetries\":2,\"baseSeed\":123,\"httpTimeoutSeconds\":7,"
        "\"workers\":[{\"host\":\"127.0.0.1\",\"port\":8080},{\"host\":\"10.0.0.2\",\"port\":8081}]}";
    std::string error;

    // Act
    const auto config = DistributedConfigLoader::fromJson(json, error);

    // Assert
    ASSERT_TRUE(config.has_value()) << error;
    EXPECT_EQ(config->modelFile, "model.txt");
    EXPECT_EQ(config->outputFile, "out.json");
    EXPECT_EQ(config->totalReplications, 30u);
    EXPECT_TRUE(config->includeLocal);
    EXPECT_EQ(config->maxRetries, 2);
    EXPECT_EQ(config->baseSeed, 123u);
    EXPECT_EQ(config->httpTimeoutSeconds, 7);
    ASSERT_EQ(config->workers.size(), 2u);
    EXPECT_EQ(config->workers[0].host, "127.0.0.1");
    EXPECT_EQ(config->workers[0].port, 8080);
    EXPECT_EQ(config->workers[1].port, 8081);
}

TEST(DistributedConfigLoader, ShouldRejectJsonWithoutModelFile) {
    // Arrange
    const std::string json = "{\"totalReplications\":10,\"includeLocal\":true}";
    std::string error;

    // Act
    const auto config = DistributedConfigLoader::fromJson(json, error);

    // Assert
    EXPECT_FALSE(config.has_value());
    EXPECT_FALSE(error.empty());
}

TEST(DistributedConfigLoader, ShouldRejectWhenNoWorkersAndNoLocal) {
    // Arrange
    const std::string json = "{\"modelFile\":\"m.txt\",\"totalReplications\":10}";
    std::string error;

    // Act
    const auto config = DistributedConfigLoader::fromJson(json, error);

    // Assert
    EXPECT_FALSE(config.has_value());
    EXPECT_FALSE(error.empty());
}

TEST(DistributedConfigLoader, ShouldParseArgsWithRepeatedWorkers) {
    // Arrange
    const std::vector<std::string> args = {
        "--model", "m.txt", "--replications", "20", "--local",
        "--worker", "127.0.0.1:8080", "--worker", "host-b:9090", "--max-retries", "3"};
    std::string error;

    // Act
    const auto config = DistributedConfigLoader::fromArgs(args, error);

    // Assert
    ASSERT_TRUE(config.has_value()) << error;
    EXPECT_EQ(config->modelFile, "m.txt");
    EXPECT_EQ(config->totalReplications, 20u);
    EXPECT_TRUE(config->includeLocal);
    EXPECT_EQ(config->maxRetries, 3);
    ASSERT_EQ(config->workers.size(), 2u);
    EXPECT_EQ(config->workers[0].host, "127.0.0.1");
    EXPECT_EQ(config->workers[0].port, 8080);
    EXPECT_EQ(config->workers[1].host, "host-b");
    EXPECT_EQ(config->workers[1].port, 9090);
}

TEST(DistributedConfigLoader, ShouldRejectMalformedWorkerArgument) {
    // Arrange
    const std::vector<std::string> args = {
        "--model", "m.txt", "--replications", "5", "--worker", "no-port"};
    std::string error;

    // Act
    const auto config = DistributedConfigLoader::fromArgs(args, error);

    // Assert
    EXPECT_FALSE(config.has_value());
    EXPECT_FALSE(error.empty());
}

TEST(DistributedConfigLoader, ShouldRejectUnknownArgument) {
    // Arrange
    const std::vector<std::string> args = {"--model", "m.txt", "--replications", "5", "--bogus"};
    std::string error;

    // Act
    const auto config = DistributedConfigLoader::fromArgs(args, error);

    // Assert
    EXPECT_FALSE(config.has_value());
    EXPECT_FALSE(error.empty());
}
