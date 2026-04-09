#include <gtest/gtest.h>
#include "kernel/util/Util.h"
#include "kernel/util/List.h"

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
