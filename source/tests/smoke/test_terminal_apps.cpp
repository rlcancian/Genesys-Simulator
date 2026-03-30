/*
 * Smoke tests for terminal applications.
 *
 * These are not unit tests of the kernel.
 * They exercise example terminal applications end-to-end enough
 * to verify that the application entry path still works.
 */

#include <gtest/gtest.h>

#include "../../applications/terminal/examples/smarts/Smart_AssignWriteSeizes.h"
#include "../../applications/terminal/examples/smarts/Smart_BatchSeparate.h"
#include "../../applications/BaseGenesysTerminalApplication.h"

TEST(TerminalApplicationSmokeTest, SmartsExamplesReturnZero) {
    BaseGenesysTerminalApplication* app = nullptr;

    app = new Smart_AssignWriteSeizes();
    EXPECT_EQ(app->main(0, nullptr), 0);
    delete app;
    app = nullptr;

    app = new Smart_BatchSeparate();
    EXPECT_EQ(app->main(0, nullptr), 0);
    delete app;
    app = nullptr;
}
