#include <cmath>
#include <gtest/gtest.h>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/statistics/SamplerDefaultImpl1.h"
#include "parser/Genesys++-driver.h"

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

TEST_F(ParserExpressionsTest, ProbabilisticFunctionsValidExpressionsWithThrowsExceptionTrue) {
    bool success = false;
    std::string errorMessage;

    const double unifValue = model->parseExpression("unif(0.8,1.0)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(unifValue));
    EXPECT_GE(unifValue, 0.8);
    EXPECT_LE(unifValue, 1.0);

    errorMessage.clear();
    const double triaValue = model->parseExpression("tria(1,2,3)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(triaValue));
    EXPECT_GE(triaValue, 1.0);
    EXPECT_LE(triaValue, 3.0);

    errorMessage.clear();
    const double expoValue = model->parseExpression("expo(2.0)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(expoValue));
    EXPECT_GE(expoValue, 0.0);
}

TEST_F(ParserExpressionsTest, ProbabilisticFunctionsInvalidExpressionsAreRecoverableWithThrowsExceptionTrue) {
    bool success = true;
    std::string errorMessage;

    (void) model->parseExpression("unif(1,0.8)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("unif"), std::string::npos);

    errorMessage.clear();
    (void) model->parseExpression("tria(3,2,1)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("tria"), std::string::npos);

    errorMessage.clear();
    (void) model->parseExpression("weib(-1,2)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("weib"), std::string::npos);
}

TEST(ParserDriverThrowsFalseTest, ProbabilisticFunctionsRecoverWithoutThrowing) {
    SamplerDefaultImpl1 sampler;
    genesyspp_driver driver(nullptr, &sampler, false);

    EXPECT_NO_THROW({
        const int parseResult = driver.parse_str("unif(0.8,1.0)");
        EXPECT_EQ(parseResult, 0);
        EXPECT_TRUE(driver.getErrorMessage().empty());
        const double value = driver.getResult();
        EXPECT_TRUE(std::isfinite(value));
        EXPECT_GE(value, 0.8);
        EXPECT_LE(value, 1.0);
    });

    EXPECT_NO_THROW({
        const int parseResult = driver.parse_str("unif(1,0.8)");
        EXPECT_NE(parseResult, 0);
        EXPECT_EQ(driver.getResult(), -1.0);
        const std::string message = driver.getErrorMessage();
        EXPECT_FALSE(message.empty());
        EXPECT_NE(message.find("unif"), std::string::npos);
    });

    EXPECT_NO_THROW({
        const int parseResult = driver.parse_str("tria(3,2,1)");
        EXPECT_NE(parseResult, 0);
        EXPECT_EQ(driver.getResult(), -1.0);
        const std::string message = driver.getErrorMessage();
        EXPECT_FALSE(message.empty());
        EXPECT_NE(message.find("tria"), std::string::npos);
    });

    EXPECT_NO_THROW({
        const int parseResult = driver.parse_str("weib(-1,2)");
        EXPECT_NE(parseResult, 0);
        EXPECT_EQ(driver.getResult(), -1.0);
        const std::string message = driver.getErrorMessage();
        EXPECT_FALSE(message.empty());
        EXPECT_NE(message.find("weib"), std::string::npos);
    });
}
