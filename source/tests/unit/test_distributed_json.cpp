#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

#include "applications/distributed/core/Json.h"

namespace json = genesys::distributed::json;

namespace {

// A representative worker job result body, matching the worker /result contract.
const std::string kWorkerResultBody =
    "{\"ok\":true,\"data\":{"
    "\"jobId\":\"job-1\",\"state\":\"finished\",\"message\":\"\","
    "\"numberOfReplications\":10,\"replicationLength\":100.5,"
    "\"statistics\":[{\"name\":\"Queue.Wait\",\"numReplications\":10,\"average\":1.5,"
    "\"variance\":0.25,\"min\":1,\"max\":2,\"numObservations\":10}],"
    "\"counters\":[{\"name\":\"Out.Count\",\"total\":100}]"
    "}}";

} // namespace

TEST(DistributedJson, ShouldReadStringFieldWhenPresent) {
    // Act
    const auto state = json::getString(kWorkerResultBody, "state");
    const auto jobId = json::getString(kWorkerResultBody, "jobId");

    // Assert
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state.value(), "finished");
    ASSERT_TRUE(jobId.has_value());
    EXPECT_EQ(jobId.value(), "job-1");
}

TEST(DistributedJson, ShouldReturnNulloptWhenStringFieldMissing) {
    // Act
    const auto missing = json::getString(kWorkerResultBody, "doesNotExist");

    // Assert
    EXPECT_FALSE(missing.has_value());
}

TEST(DistributedJson, ShouldReadIntAndBoolAndDoubleFields) {
    // Act
    const auto replications = json::getInt(kWorkerResultBody, "numberOfReplications");
    const auto ok = json::getBool(kWorkerResultBody, "ok");
    const auto length = json::getDouble(kWorkerResultBody, "replicationLength");

    // Assert
    ASSERT_TRUE(replications.has_value());
    EXPECT_EQ(replications.value(), 10);
    ASSERT_TRUE(ok.has_value());
    EXPECT_TRUE(ok.value());
    ASSERT_TRUE(length.has_value());
    EXPECT_DOUBLE_EQ(length.value(), 100.5);
}

TEST(DistributedJson, ShouldExtractAndParseStatisticsArray) {
    // Act
    const auto statisticsArray = json::getArray(kWorkerResultBody, "statistics");
    ASSERT_TRUE(statisticsArray.has_value());
    const auto objects = json::splitObjects(statisticsArray.value());

    // Assert
    ASSERT_EQ(objects.size(), 1u);
    EXPECT_EQ(json::getString(objects[0], "name").value_or(""), "Queue.Wait");
    EXPECT_EQ(json::getInt(objects[0], "numReplications").value_or(0), 10);
    EXPECT_DOUBLE_EQ(json::getDouble(objects[0], "average").value_or(0.0), 1.5);
    EXPECT_DOUBLE_EQ(json::getDouble(objects[0], "variance").value_or(0.0), 0.25);
}

TEST(DistributedJson, ShouldExtractCountersArray) {
    // Act
    const auto countersArray = json::getArray(kWorkerResultBody, "counters");
    ASSERT_TRUE(countersArray.has_value());
    const auto objects = json::splitObjects(countersArray.value());

    // Assert
    ASSERT_EQ(objects.size(), 1u);
    EXPECT_EQ(json::getString(objects[0], "name").value_or(""), "Out.Count");
    EXPECT_DOUBLE_EQ(json::getDouble(objects[0], "total").value_or(0.0), 100.0);
}

TEST(DistributedJson, ShouldSplitMultipleObjectsIncludingNestedBraces) {
    // Arrange
    const std::string arrayBody = "{\"a\":1,\"nested\":{\"x\":2}},{\"a\":3}";

    // Act
    const auto objects = json::splitObjects(arrayBody);

    // Assert
    ASSERT_EQ(objects.size(), 2u);
    EXPECT_EQ(json::getInt(objects[0], "a").value_or(0), 1);
    EXPECT_EQ(json::getInt(objects[1], "a").value_or(0), 3);
}

TEST(DistributedJson, ShouldEscapeQuotesAndBackslashes) {
    // Act
    const std::string escaped = json::escape("a\"b\\c");

    // Assert
    EXPECT_EQ(escaped, "a\\\"b\\\\c");
}

TEST(DistributedJson, ShouldFormatDoublesWithRoundTripPrecision) {
    // Arrange
    const double value = 1.0 / 3.0;

    // Act
    const std::string formatted = json::number(value);

    // Assert: the formatted text must round-trip back to the exact same double.
    EXPECT_DOUBLE_EQ(std::strtod(formatted.c_str(), nullptr), value);
}
