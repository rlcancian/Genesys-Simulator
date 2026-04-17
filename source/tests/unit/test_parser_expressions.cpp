#include <cmath>
#include <functional>
#include <gtest/gtest.h>

#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/Event.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/ParserDefaultImpl2.h"
#include "kernel/statistics/SamplerDefaultImpl1.h"
#include "parser/Genesys++-driver.h"
#include "plugins/data/DiscreteProcessing/Variable.h"

class ParserExpressionsTest : public ::testing::Test {
protected:
    Simulator simulator;
    Model* model = nullptr;

    void SetUp() override {
        model = simulator.getModelManager()->newModel();
        ASSERT_NE(model, nullptr);
    }
};

class ParserAttributeInternalEventOwner : public Attribute {
public:
    ParserAttributeInternalEventOwner(Model* model, const std::string& name = "")
        : Attribute(model, name) {}

    void Noop(void*) {}
};

struct ParserAttributeEventInjector {
    Model* model = nullptr;
    ParserAttributeInternalEventOwner* owner = nullptr;
    bool inserted = false;

    void OnReplicationStart(SimulationEvent*) {
        if (inserted || model == nullptr || owner == nullptr) {
            return;
        }
        Entity* entity = model->createEntity("ParserAttributeEntity", true);
        entity->setAttributeValue("ParserAttrND", 31.0, "", true);
        entity->setAttributeValue("ParserAttrND", 32.0, "1");
        entity->setAttributeValue("ParserAttrND", 33.0, "1,2");
        entity->setAttributeValue("ParserAttrND", 34.0, "1,2,3");
        entity->setAttributeValue("ParserAttrND", 35.0, "1,2,3,4,5");
        auto* event = new InternalEvent(0.0, "Parser attribute expression probe");
        event->setEntity(entity);
        event->setEventHandler(owner, &ParserAttributeInternalEventOwner::Noop, nullptr);
        model->getFutureEvents()->insert(event);
        inserted = true;
    }
};

struct ParserAttributeProcessObserver {
    std::function<void(Entity*)> onProcess;

    void OnProcessEvent(SimulationEvent* event) {
        if (event == nullptr || event->getCurrentEvent() == nullptr || event->getCurrentEvent()->getEntity() == nullptr) {
            return;
        }
        if (onProcess) {
            onProcess(event->getCurrentEvent()->getEntity());
        }
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

TEST_F(ParserExpressionsTest, VariableIndexesSupportScalarLegacyAndNDReads) {
    auto* variable = new Variable(model, "ParserVarND");
    ASSERT_NE(variable, nullptr);
    variable->setValue(10.0);
    variable->setValue(11.0, "1");
    variable->setValue(12.0, "1,2");
    variable->setValue(13.0, "1,2,3");
    variable->setValue(14.0, "1,2,3,4,5");

    EXPECT_DOUBLE_EQ(model->parseExpression("ParserVarND"), 10.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserVarND[1]"), 11.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserVarND[1,2]"), 12.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserVarND[1,2,3]"), 13.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserVarND[1,2,3,4,5]"), 14.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserVarND[1,2,3,4,6]"), 0.0);

    delete variable;
}

TEST_F(ParserExpressionsTest, AttributeIndexesSupportLegacyAndNDReadsAndAssignmentsDuringEvent) {
    auto* attribute = new ParserAttributeInternalEventOwner(model, "ParserAttrND");
    ASSERT_NE(attribute, nullptr);

    bool dispatched = false;
    double scalarRead = 0.0;
    double oneDRead = 0.0;
    double twoDRead = 0.0;
    double threeDRead = 0.0;
    double ndRead = 0.0;
    double missingRead = -1.0;
    double ndAssignmentResult = 0.0;
    double ndAssignedValue = 0.0;

    ParserAttributeProcessObserver observer;
    observer.onProcess = [&](Entity* entity) {
        ASSERT_NE(entity, nullptr);
        dispatched = true;
        scalarRead = model->parseExpression("ParserAttrND");
        oneDRead = model->parseExpression("ParserAttrND[1]");
        twoDRead = model->parseExpression("ParserAttrND[1,2]");
        threeDRead = model->parseExpression("ParserAttrND[1,2,3]");
        ndRead = model->parseExpression("ParserAttrND[1,2,3,4,5]");
        missingRead = model->parseExpression("ParserAttrND[1,2,3,4,6]");
        ndAssignmentResult = model->parseExpression("ParserAttrND[1,2,3,4,6]=36");
        ndAssignedValue = entity->getAttributeValue("ParserAttrND", "1,2,3,4,6");
    };

    ParserAttributeEventInjector injector{model, attribute};
    model->getOnEventManager()->addOnReplicationStartHandler(&injector, &ParserAttributeEventInjector::OnReplicationStart);
    model->getOnEventManager()->addOnProcessEventHandler(&observer, &ParserAttributeProcessObserver::OnProcessEvent);
    model->getSimulation()->setReplicationLength(1.0);
    model->getSimulation()->start();

    EXPECT_TRUE(dispatched);
    EXPECT_DOUBLE_EQ(scalarRead, 31.0);
    EXPECT_DOUBLE_EQ(oneDRead, 32.0);
    EXPECT_DOUBLE_EQ(twoDRead, 33.0);
    EXPECT_DOUBLE_EQ(threeDRead, 34.0);
    EXPECT_DOUBLE_EQ(ndRead, 35.0);
    EXPECT_DOUBLE_EQ(missingRead, 0.0);
    EXPECT_DOUBLE_EQ(ndAssignmentResult, 36.0);
    EXPECT_DOUBLE_EQ(ndAssignedValue, 36.0);

    delete attribute;
}

TEST_F(ParserExpressionsTest, VariableIndexesSupportScalarLegacyAndNDAssignments) {
    auto* variable = new Variable(model, "ParserAssignVarND");
    ASSERT_NE(variable, nullptr);

    EXPECT_DOUBLE_EQ(model->parseExpression("ParserAssignVarND=21"), 21.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserAssignVarND[1]=22"), 22.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserAssignVarND[1,2]=23"), 23.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserAssignVarND[1,2,3]=24"), 24.0);
    EXPECT_DOUBLE_EQ(model->parseExpression("ParserAssignVarND[1,2,3,4,5]=25"), 25.0);

    EXPECT_DOUBLE_EQ(variable->getValue(""), 21.0);
    EXPECT_DOUBLE_EQ(variable->getValue("1"), 22.0);
    EXPECT_DOUBLE_EQ(variable->getValue("1,2"), 23.0);
    EXPECT_DOUBLE_EQ(variable->getValue("1,2,3"), 24.0);
    EXPECT_DOUBLE_EQ(variable->getValue("1,2,3,4,5"), 25.0);

    delete variable;
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

    errorMessage.clear();
    const double weibValue = model->parseExpression("weib(2,3)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(weibValue));
    EXPECT_GE(weibValue, 0.0);
}

TEST_F(ParserExpressionsTest, AdditionalProbabilisticFunctionsValidExpressionsWithThrowsExceptionTrue) {
    bool success = false;
    std::string errorMessage;

    const double normValue = model->parseExpression("norm(0,1)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(normValue));

    errorMessage.clear();
    const double lognValue = model->parseExpression("logn(2,0.5)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(lognValue));
    EXPECT_GE(lognValue, 0.0);

    errorMessage.clear();
    const double gammValue = model->parseExpression("gamm(2,3)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(gammValue));
    EXPECT_GE(gammValue, 0.0);

    errorMessage.clear();
    const double erlaValue = model->parseExpression("erla(4,2)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(erlaValue));
    EXPECT_GE(erlaValue, 0.0);

    errorMessage.clear();
    const double betaValue = model->parseExpression("beta(2,3,0,1)", success, errorMessage);
    EXPECT_TRUE(success);
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_TRUE(std::isfinite(betaValue));
    EXPECT_GE(betaValue, 0.0);
    EXPECT_LE(betaValue, 1.0);
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

TEST_F(ParserExpressionsTest, AdditionalProbabilisticFunctionsInvalidExpressionsAreRecoverableWithThrowsExceptionTrue) {
    bool success = true;
    std::string errorMessage;

    (void) model->parseExpression("norm(0,-1)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("norm"), std::string::npos);

    errorMessage.clear();
    (void) model->parseExpression("logn(0,0.5)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("logn"), std::string::npos);

    errorMessage.clear();
    (void) model->parseExpression("logn(2,-0.5)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("logn"), std::string::npos);

    errorMessage.clear();
    (void) model->parseExpression("gamm(2,0)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("gamm"), std::string::npos);

    errorMessage.clear();
    (void) model->parseExpression("erla(4,0)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("erla"), std::string::npos);

    errorMessage.clear();
    (void) model->parseExpression("beta(2,3,1,0)", success, errorMessage);
    EXPECT_FALSE(success);
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("beta"), std::string::npos);
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

    EXPECT_NO_THROW({
        const int parseResult = driver.parse_str("logn(2,-0.5)");
        EXPECT_NE(parseResult, 0);
        EXPECT_EQ(driver.getResult(), -1.0);
        const std::string message = driver.getErrorMessage();
        EXPECT_FALSE(message.empty());
        EXPECT_NE(message.find("logn"), std::string::npos);
    });
}


class CountingSampler final : public Sampler_if {
public:
    explicit CountingSampler(int* destroyedCounter, bool* destroyedFlag = nullptr)
        : _destroyedCounter(destroyedCounter), _destroyedFlag(destroyedFlag) {}

    ~CountingSampler() override {
        if (_destroyedCounter != nullptr) {
            ++(*_destroyedCounter);
        }
        if (_destroyedFlag != nullptr) {
            *_destroyedFlag = true;
        }
    }

    double random() override { return 0.5; }
    double sampleBeta(double, double, double infLimit, double) override { return infLimit; }
    double sampleBeta(double, double) override { return 0.5; }
    double sampleErlang(double, int, double offset = 0.0) override { return offset; }
    double sampleExponential(double, double offset = 0.0) override { return offset; }
    double sampleGamma(double, double, double offset = 0.0) override { return offset; }
    double sampleGumbell(double mode, double) override { return mode; }
    double sampleLogNormal(double, double, double offset = 0.0) override { return offset; }
    double sampleNormal(double mean, double) override { return mean; }
    double sampleTriangular(double min, double, double) override { return min; }
    double sampleUniform(double min, double) override { return min; }
    double sampleWeibull(double, double) override { return 0.0; }
    double sampleBinomial(int, double) override { return 0.0; }
    double sampleBernoulli(double) override { return 0.0; }
    double sampleDiscrete(double, double, ...) override { return 0.0; }
    double sampleDiscrete(double*, double*, int) override { return 0.0; }
    double sampleGeometric(double) override { return 0.0; }
    void setRNGparameters(RNG_Parameters*) override {}
    RNG_Parameters* getRNGparameters() const override { return nullptr; }

private:
    int* _destroyedCounter = nullptr;
    bool* _destroyedFlag = nullptr;
};

TEST_F(ParserExpressionsTest, ParserDefaultImpl2SetSamplerDeletesPreviousOwnedSamplerAndDoesNotDeleteExternalSampler) {
    int destroyedCounter = 0;
    bool externalDestroyed = false;

    auto* initiallyOwnedSampler = new CountingSampler(&destroyedCounter);
    CountingSampler externalSampler(&destroyedCounter, &externalDestroyed);

    {
        ParserDefaultImpl2 parser(model, initiallyOwnedSampler, false);
        parser.setSampler(&externalSampler);
        EXPECT_EQ(parser.getSampler(), &externalSampler);
        EXPECT_EQ(destroyedCounter, 1);
        EXPECT_FALSE(externalDestroyed);
    }

    EXPECT_FALSE(externalDestroyed);
    EXPECT_EQ(destroyedCounter, 1);
}

TEST_F(ParserExpressionsTest, ParserDefaultImpl2SetSamplerSameExternalPointerDoesNotDeleteIt) {
    int destroyedCounter = 0;
    bool externalDestroyed = false;

    auto* initiallyOwnedSampler = new CountingSampler(&destroyedCounter);
    CountingSampler externalSampler(&destroyedCounter, &externalDestroyed);

    {
        ParserDefaultImpl2 parser(model, initiallyOwnedSampler, false);
        parser.setSampler(&externalSampler);
        parser.setSampler(&externalSampler);
        EXPECT_EQ(parser.getSampler(), &externalSampler);
        EXPECT_EQ(destroyedCounter, 1);
        EXPECT_FALSE(externalDestroyed);
    }

    EXPECT_FALSE(externalDestroyed);
    EXPECT_EQ(destroyedCounter, 1);
}
