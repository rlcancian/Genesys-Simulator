#include <gtest/gtest.h>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"

class ParserExpressionsTest : public ::testing::Test {
protected:
    Simulator simulator;
    Model* model = nullptr;

    void SetUp() override {
        model = simulator.getModelManager()->newModel();
        ASSERT_NE(model, nullptr);
    }
};

TEST_F(ParserExpressionsTest, ArithmeticPrecedenceAndParentheses) {
    EXPECT_DOUBLE_EQ(model->parseExpression("1+2*3"), 7.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("(1+2)*3"), 9.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("10/2+3"), 8.0);
}

TEST_F(ParserExpressionsTest, PowerIsRightAssociative) {
    EXPECT_DOUBLE_EQ(model->parseExpression("2^3^2"), 512.0);
}

TEST_F(ParserExpressionsTest, RelationalAndLogicalWordOperators) {
    EXPECT_DOUBLE_EQ(model->parseExpression("1<2 and 3>2"), 1.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("not(1<2)"), 0.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("1<>1 or 2==2"), 1.0);
}

TEST_F(ParserExpressionsTest, MathFunctionsRoundTruncFracAndSqrt) {
    EXPECT_DOUBLE_EQ(model->parseExpression("round(3.6)"), 4.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("trunc(3.9)"), 3.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("frac(3.9)"), 0.9);
    EXPECT_DOUBLE_EQ(model->parseExpression("sqrt(9)"), 3.0);
}
