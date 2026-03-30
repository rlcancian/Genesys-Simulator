#include <gtest/gtest.h>
#include "kernel/util/Util.h"

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
