#include <gtest/gtest.h>
#include "kernel/util/Util.h"
#include "kernel/util/List.h"
#include <map>
#include <list>
#include <string>

TEST(UtilTest, DirSeparatorMatchesPlatformExpectation) {
#ifdef __linux__
    EXPECT_EQ(Util::DirSeparator(), '/');
#elif _WIN32
    EXPECT_EQ(Util::DirSeparator(), '\\');
#else
    GTEST_SKIP() << "Unsupported platform branch for this test";
#endif
}

TEST(UtilTest, TimeUnitConvertSecondToMinute) {
    EXPECT_DOUBLE_EQ(
        Util::TimeUnitConvert(Util::TimeUnit::second, Util::TimeUnit::minute),
        1.0 / 60.0
    );
}

TEST(UtilTest, TimeUnitStringConversionsKnownAndUnknown) {
    EXPECT_EQ(Util::StrTimeUnitShort(Util::TimeUnit::picosecond), "ps");
    EXPECT_EQ(Util::StrTimeUnitShort(Util::TimeUnit::minute), "min");
    EXPECT_EQ(Util::StrTimeUnitShort(Util::TimeUnit::unknown), "");

    EXPECT_EQ(Util::StrTimeUnitLong(Util::TimeUnit::nanosecond), "nanosecond");
    EXPECT_EQ(Util::StrTimeUnitLong(Util::TimeUnit::week), "week");
    EXPECT_EQ(Util::StrTimeUnitLong(static_cast<Util::TimeUnit>(999)), "unknown");

    EXPECT_EQ(Util::convertEnumToStr(Util::TimeUnit::hour), "hour");
    EXPECT_EQ(Util::convertEnumToStr(static_cast<Util::TimeUnit>(-1)), "unknown");
}

TEST(UtilTest, AllocationStringConversionsKnownAndUnknown) {
    EXPECT_EQ(Util::StrAllocation(Util::AllocationType::ValueAdded), "ValueAdded");
    EXPECT_EQ(Util::StrAllocation(Util::AllocationType::Others), "Others");
    EXPECT_EQ(Util::StrAllocation(static_cast<Util::AllocationType>(42)), "Unknown");

    EXPECT_EQ(Util::convertEnumToStr(Util::AllocationType::Transfer), "Transfer");
    EXPECT_EQ(Util::convertEnumToStr(static_cast<Util::AllocationType>(-1)), "Unknown");
}

TEST(UtilTest, TimeUnitConvertAdditionalPairsAreCoherent) {
    EXPECT_DOUBLE_EQ(
        Util::TimeUnitConvert(Util::TimeUnit::minute, Util::TimeUnit::second),
        60.0
    );
    EXPECT_DOUBLE_EQ(
        Util::TimeUnitConvert(Util::TimeUnit::hour, Util::TimeUnit::minute),
        60.0
    );
    EXPECT_DOUBLE_EQ(
        Util::TimeUnitConvert(Util::TimeUnit::day, Util::TimeUnit::hour),
        24.0
    );
    EXPECT_DOUBLE_EQ(
        Util::TimeUnitConvert(Util::TimeUnit::week, Util::TimeUnit::day),
        7.0
    );

    const double secondsToHours = Util::TimeUnitConvert(Util::TimeUnit::second, Util::TimeUnit::hour);
    const double hoursToSeconds = Util::TimeUnitConvert(Util::TimeUnit::hour, Util::TimeUnit::second);
    EXPECT_DOUBLE_EQ(secondsToHours * hoursToSeconds, 1.0);
}

TEST(UtilTest, GlobalIdGenerationIsMonotonic) {
    const auto id1 = Util::GenerateNewId();
    const auto id2 = Util::GenerateNewId();
    const auto id3 = Util::GenerateNewId();

    EXPECT_EQ(id2, id1 + 1);
    EXPECT_EQ(id3, id2 + 1);
}

TEST(UtilTest, TypeScopedIdsCanBeResetIndependently) {
    Util::ResetAllIds();

    EXPECT_EQ(Util::GetLastIdOfType("Entity"), 0u);
    EXPECT_EQ(Util::GenerateNewIdOfType("Entity"), 1u);
    EXPECT_EQ(Util::GenerateNewIdOfType("Entity"), 2u);
    EXPECT_EQ(Util::GetLastIdOfType("Entity"), 2u);

    EXPECT_EQ(Util::GenerateNewIdOfType("Resource"), 1u);
    EXPECT_EQ(Util::GetLastIdOfType("Resource"), 1u);

    Util::ResetIdOfType("Entity");
    EXPECT_EQ(Util::GetLastIdOfType("Entity"), 0u);
    EXPECT_EQ(Util::GenerateNewIdOfType("Entity"), 1u);
    EXPECT_EQ(Util::GetLastIdOfType("Resource"), 1u);
}

TEST(UtilTest, ResetAllIdsClearsAllTypeScopedCounters) {
    Util::ResetAllIds();
    EXPECT_EQ(Util::GenerateNewIdOfType("A"), 1u);
    EXPECT_EQ(Util::GenerateNewIdOfType("B"), 1u);

    Util::ResetAllIds();
    EXPECT_EQ(Util::GetLastIdOfType("A"), 0u);
    EXPECT_EQ(Util::GetLastIdOfType("B"), 0u);
    EXPECT_EQ(Util::GenerateNewIdOfType("A"), 1u);
}

TEST(UtilTest, StringHelpersCoverNominalAndEdgeBehavior) {
    std::string key;
    std::string value;
    Util::SepKeyVal("name=value=tail", key, value);
    EXPECT_EQ(key, "name");
    EXPECT_EQ(value, "value=tail");

    EXPECT_EQ(Util::SetW("abc", 5), "abc  ");
    EXPECT_EQ(Util::SetW("abcdef", 3), "abc");

    EXPECT_EQ(Util::StrTruncIfInt(15.0), "15.0");
    EXPECT_EQ(Util::StrTruncIfInt(15.25), "15.250000");
    EXPECT_EQ(Util::StrTruncIfInt(std::string("12.000000")), "12");
    EXPECT_EQ(Util::StrTruncIfInt(std::string("12.500000")), "12.500000");

    EXPECT_EQ(Util::Trim(" \t hello world \n"), "hello world");
    EXPECT_EQ(Util::StrReplace("a-b-c-b", "b", "x"), "a-x-c-x");
    EXPECT_EQ(Util::StrReplaceSpecialChars("A B::C*\"D\""), "A_B_C_D");
    EXPECT_EQ(Util::StrIndex(42), "[42]");

    std::string trimWithin = "a b  c";
    Util::Trimwithin(trimWithin);
    EXPECT_EQ(trimWithin, "abc");
}

TEST(UtilTest, CollectionToStringHelpersForNonEmptyContainers) {
    std::map<std::string, std::string> stringMap{
        {"beta", "2"},
        {"alpha", "1"}
    };
    EXPECT_EQ(Util::Map2str(&stringMap), "alpha=1 beta=2");

    std::map<std::string, double> numericMap{
        {"zeta", 2.0},
        {"eta", 3.5}
    };
    EXPECT_EQ(Util::Map2str(&numericMap), "eta=3.500000 zeta=2");

    std::list<unsigned int> values{3, 7, 11};
    EXPECT_EQ(Util::List2str(&values), "3, 7, 11");
}

TEST(ListTest, CopyConstructorCreatesIndependentContainerState) {
    int a = 1;
    int b = 2;
    int c = 3;

    List<int*> original;
    original.insert(&a);
    original.insert(&b);

    List<int*> copy(original);
    copy.insert(&c);

    EXPECT_EQ(original.size(), 2u);
    EXPECT_EQ(copy.size(), 3u);
}

TEST(ListTest, CopyAssignmentCreatesIndependentContainerState) {
    int a = 1;
    int b = 2;
    int c = 3;

    List<int*> original;
    original.insert(&a);

    List<int*> assigned;
    assigned.insert(&b);
    assigned = original;
    assigned.insert(&c);

    EXPECT_EQ(original.size(), 1u);
    EXPECT_EQ(assigned.size(), 2u);
}
