/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/testFiles/simpletest.cpp to edit this template
 */

/* 
 * File:   testTerminalApps.cpp
 * Author: rlcancian
 *
 * Created on 3 de agosto de 2022, 21:27
 */

// based on https://micromouseonline.com/2016/02/08/googletest-with-netbeans/
// https://micromouseonline.com/2016/02/12/tdd-with-googletest-and-netbeans/

// to create tests in the source: https://www.youtube.com/watch?v=TS2CTf11k1U (compile gtest as a library)

#include <stdlib.h>
#include <iostream>
#include "../../source/gtest/gtest/gtest.h"

#include "../applications/terminal/examples/smarts/Smart_AssignWriteSeizes.h"
#include "../applications/terminal/examples/smarts/Smart_BatchSeparate.h"
#include "../applications/BaseGenesysTerminalApplication.h"
using namespace std;

TEST(TerminalApplication, Smarts) {
	BaseGenesysTerminalApplication *app;
	app = new Smart_AssignWriteSeizes();
	EXPECT_EQ(app->main(0, nullptr), 0);
	app = new Smart_BatchSeparate();
	EXPECT_EQ(app->main(0, nullptr), 0);
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
