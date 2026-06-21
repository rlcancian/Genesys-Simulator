#include <gtest/gtest.h>
#include "kernel/simulator/Simulator.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"

class Test_PetriComponents : public ::testing::Test {
protected:
    Simulator* genesys;
    Model* model;

    void SetUp() override {
        genesys = new Simulator();
        model = genesys->getModelManager()->newModel();
    }
        void TearDown() override {
        delete genesys;
    }
};

// SUÍTE 1: PETRI PLACE
TEST_F(Test_PetriComponents, AdicaoRemocaoBasica) {
    PetriPlace* p = new PetriPlace(model, "P1");

    p->addTokens(3, "blue");
    p->addTokens(2, "red");

    EXPECT_EQ(p->getTokens("blue"), 3);
    EXPECT_EQ(p->getTokens("red"), 2);

    bool removed = p->removeTokens(1, "blue");
    EXPECT_TRUE(removed);
    EXPECT_EQ(p->getTokens("blue"), 2); // 3 - 1 = 2
}

TEST_F(Test_PetriComponents, CorInexistente) {
    PetriPlace* p = new PetriPlace(model, "P1");

    // Nunca adicionei "green"
    EXPECT_EQ(p->getTokens("green"), 0);
}

TEST_F(Test_PetriComponents, RemocaoAcimaDoSaldo) {
    PetriPlace* p = new PetriPlace(model, "P1");
    p->addTokens(2, "blue");

    // Tento remover 5, mas só tenho 2.
    bool removed = p->removeTokens(5, "blue");

    EXPECT_FALSE(removed);
    EXPECT_EQ(p->getTokens("blue"), 2);
}

// SUÍTE 2: PETRI TRANSITION

TEST_F(Test_PetriComponents, AtualizacaoDePesos) {
    PetriPlace* p1 = new PetriPlace(model, "P1");
    PetriTransition* t = new PetriTransition(p1, nullptr, "T1");

    t->setInputArcWeight(p1, "blue", 2);
    EXPECT_EQ(t->getInputArcWeight(p1, "blue"), 2);

    t->setInputArcWeight(p1, "blue", 5);
    EXPECT_EQ(t->getInputArcWeight(p1, "blue"), 5);
}

TEST_F(Test_PetriComponents, CanFireCondicoes) {
    PetriPlace* in1 = new PetriPlace(model, "In1");
    PetriPlace* in2 = new PetriPlace(model, "In2");
    PetriTransition* t = new PetriTransition(in1, nullptr, "T_Condicoes");

    t->setInputArcWeight(in1, "blue", 2);
    t->setInputArcWeight(in2, "red", 1);

    // Cenário 1: Tudo zerado
    EXPECT_FALSE(t->canFire(model, nullptr));

    // Cenário 2: Tem azul, mas falta vermelho
    in1->addTokens(5, "blue");
    EXPECT_FALSE(t->canFire(model, nullptr));

    // Cenário 3: Tem vermelho, mas falta 1 azul
    in1->removeTokens(4, "blue");
    in2->addTokens(1, "red");
    EXPECT_FALSE(t->canFire(model, nullptr));

    // Cenário 4: Condição exata (Sucesso)
    in1->addTokens(1, "blue");
    EXPECT_TRUE(t->canFire(model, nullptr));
}

TEST_F(Test_PetriComponents, ExecuteMultiCorNoMesmoLugar) {
    PetriPlace* in = new PetriPlace(model, "P_Entrada");
    PetriPlace* out = new PetriPlace(model, "P_Saida");
    PetriTransition* t = new PetriTransition(in, out, "T_Mista");

    t->setInputArcWeight(in, "blue", 1);
    t->setInputArcWeight(in, "red", 1);

    t->setOutputArcWeight(out, "purple", 2);
    t->setOutputArcWeight(out, "yellow", 1);

    in->addTokens(1, "blue");
    in->addTokens(1, "red");

    EXPECT_TRUE(t->canFire(model, nullptr));

    t->execute(model, nullptr);

    EXPECT_EQ(in->getTokens("blue"), 0);
    EXPECT_EQ(in->getTokens("red"), 0);
    EXPECT_EQ(out->getTokens("purple"), 2);
    EXPECT_EQ(out->getTokens("yellow"), 1);
}
