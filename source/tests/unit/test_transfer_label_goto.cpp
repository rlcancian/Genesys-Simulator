// Behavioral regressions for the Transfer category's LabelDefinition/Label/Goto contract.
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <unistd.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Event.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponentManager.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/essentialPlugins/EntityType.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/Transfer/Label.h"
#include "plugins/components/Transfer/Goto.h"
#include "plugins/components/Transfer/Route.h"
#include "plugins/data/Transfer/Sequence.h"
#include "plugins/data/Transfer/LabelDefinition.h"

namespace {
struct GotoProbe : Goto {
    using Goto::Goto;
    bool check(std::string& error) { return _check(error); }
    void dispatch(Entity* entity) { _onDispatchEvent(entity, 0); }
};

struct LabelProbe : Label {
    using Label::Label;
    bool check(std::string& error) { return _check(error); }
    void dispatch(Entity* entity) { _onDispatchEvent(entity, 0); }
};


struct TransferFixture : testing::Test {
    Simulator simulator;
    Model* model = nullptr;
    void SetUp() override {
        simulator.getPluginManager()->autoInsertPlugins();
        model = simulator.getModelManager()->newModel();
        model->getSimulation()->setReplicationLength(100.0, Util::TimeUnit::second);
        model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
    }

    LabelProbe* makeLabel(const std::string& name) {
        auto* label = new LabelProbe(model, name);
        label->connectTo(new Dispose(model, name + "_Sink"));
        return label;
    }
};
} // namespace

TEST_F(TransferFixture, PluginMetadataAndUniqueIdentities) {
    std::unique_ptr<PluginInformation> defInfo(LabelDefinition::GetPluginInformation());
    std::unique_ptr<PluginInformation> labelInfo(Label::GetPluginInformation());
    std::unique_ptr<PluginInformation> gotoInfo(Goto::GetPluginInformation());
    ASSERT_NE(defInfo, nullptr);
    ASSERT_NE(labelInfo, nullptr);
    ASSERT_NE(gotoInfo, nullptr);
    EXPECT_EQ(defInfo->getCategory(), "Transfer");
    EXPECT_EQ(labelInfo->getCategory(), "Transfer");
    EXPECT_EQ(gotoInfo->getCategory(), "Transfer");
    EXPECT_EQ(defInfo->getPluginTypename(), "LabelDefinition");
    EXPECT_EQ(labelInfo->getPluginTypename(), "Label");
    EXPECT_EQ(gotoInfo->getPluginTypename(), "Goto");
    EXPECT_TRUE(labelInfo->isReceiveTransfer());
    EXPECT_EQ(labelInfo->getMinimumInputs(), 0u);
    EXPECT_EQ(labelInfo->getMaximumInputs(), 0u);
    EXPECT_EQ(labelInfo->getMinimumOutputs(), 1u);
    EXPECT_EQ(labelInfo->getMaximumOutputs(), 1u);
    EXPECT_TRUE(gotoInfo->isSendTransfer());
    EXPECT_EQ(gotoInfo->getMinimumInputs(), 1u);
    EXPECT_EQ(gotoInfo->getMaximumInputs(), 1u);
    EXPECT_EQ(gotoInfo->getMinimumOutputs(), 0u);
    EXPECT_EQ(gotoInfo->getMaximumOutputs(), 0u);
}

TEST_F(TransferFixture, LabelOwnsOneDefinitionAndRepeatedCheckDoesNotDuplicate) {
    auto* label = makeLabel("Receiver");
    auto* def = label->getLabelDefinition();
    ASSERT_NE(def, nullptr);
    EXPECT_EQ(def->getEnterIntoLabelComponent(), label);
    EXPECT_EQ(label->getInternalData("LabelDefinition"), def);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<LabelDefinition>()), 1u);
    std::string error;
    EXPECT_TRUE(label->check(error)) << error;
    EXPECT_TRUE(label->check(error)) << error;
    EXPECT_EQ(label->getLabelDefinition(), def);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<LabelDefinition>()), 1u);
}

TEST_F(TransferFixture, TwoReceiversCannotShareDefinition) {
    auto* first = makeLabel("First");
    auto* second = makeLabel("Second");
    auto* destination = first->getLabelDefinition();
    ASSERT_NE(destination, nullptr);
    second->setLabelDefinition(destination);
    EXPECT_EQ(destination->getEnterIntoLabelComponent(), first);
    EXPECT_NE(second->getLabelDefinition(), destination);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<LabelDefinition>()), 2u);
}

TEST_F(TransferFixture, LabelDefinitionReplacementRejectsRouteDirectReference) {
    auto* receiver = makeLabel("Owner");
    LabelDefinition* original = receiver->getLabelDefinition();
    ASSERT_NE(original, nullptr);
    auto* route = new Route(model, "ReferencingRoute");
    route->setLabel(original);
    auto* replacement = new LabelDefinition(model, "Replacement");

    receiver->setLabelDefinition(replacement);

    EXPECT_EQ(receiver->getLabelDefinition(), original);
    EXPECT_EQ(route->getLabel(), original);
    EXPECT_EQ(original->getEnterIntoLabelComponent(), receiver);
    delete replacement;
}

TEST_F(TransferFixture, LabelDefinitionReplacementRejectsSequenceStepDirectReference) {
    auto* receiver = makeLabel("Owner");
    LabelDefinition* original = receiver->getLabelDefinition();
    ASSERT_NE(original, nullptr);
    auto* sequence = new Sequence(model, "ReferencingSequence");
    sequence->getSteps()->insert(new SequenceStep(original));
    auto* replacement = new LabelDefinition(model, "Replacement");

    receiver->setLabelDefinition(replacement);

    EXPECT_EQ(receiver->getLabelDefinition(), original);
    ASSERT_EQ(sequence->getSteps()->size(), 1u);
    EXPECT_EQ(sequence->getSteps()->list()->front()->getLabel(), original);
    EXPECT_EQ(original->getEnterIntoLabelComponent(), receiver);
    delete replacement;
}

TEST_F(TransferFixture, GotoRequiresExistingReceiverAndHasZeroOutputs) {
    auto* sender = new GotoProbe(model, "Sender");
    std::string error;
    EXPECT_FALSE(sender->check(error));
    EXPECT_NE(error.find("LabelDefinition"), std::string::npos);
    EXPECT_EQ(sender->getLabelDefinition(), nullptr);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<LabelDefinition>()), 0u);
    auto* receiver = makeLabel("Receiver");
    sender->setLabelDefinition(receiver->getLabelDefinition());
    error.clear();
    EXPECT_TRUE(sender->check(error)) << error;
    sender->connectTo(receiver);
    error.clear();
    EXPECT_FALSE(sender->check(error));
    EXPECT_NE(error.find("conventional outputs"), std::string::npos);
}

TEST_F(TransferFixture, GotoRejectsMalformedExpressionAndInvalidUnits) {
    auto* receiver = makeLabel("Receiver");
    auto* sender = new GotoProbe(model, "Sender");
    sender->setLabelDefinition(receiver->getLabelDefinition());
    EXPECT_EQ(sender->getTimeDelayExpression(), "0.0");
    sender->setTimeDelayExpression("this is not a valid expression!!!");
    std::string error;
    EXPECT_FALSE(sender->check(error));
    sender->setTimeDelayExpression("0");
    sender->setTimeDelayTimeUnit(static_cast<Util::TimeUnit>(12345));
    error.clear();
    EXPECT_FALSE(sender->check(error));
    EXPECT_NE(error.find("TimeDelayTimeUnit"), std::string::npos);
    auto* entity = model->createEntity("UnitEntity", true);
    sender->dispatch(entity);
    EXPECT_TRUE(model->getFutureEvents()->empty());
}

TEST_F(TransferFixture, ZeroDelaySchedulesExactlyOneLabelEventAndPreservesIdentity) {
    auto* receiver = makeLabel("Receiver");
    auto* sender = new GotoProbe(model, "Sender");
    sender->setLabelDefinition(receiver->getLabelDefinition());
    auto* entity = model->createEntity("EntityA", true);
    ASSERT_NE(entity, nullptr);
    sender->dispatch(entity);
    ASSERT_EQ(model->getFutureEvents()->size(), 1u);
    Event* scheduled = model->getFutureEvents()->front();
    ASSERT_NE(scheduled, nullptr);
    EXPECT_EQ(scheduled->getComponent(), receiver);
    EXPECT_EQ(scheduled->getEntity(), entity);
    EXPECT_DOUBLE_EQ(scheduled->getTime(), 0.0);
    model->getFutureEvents()->pop_front();
    receiver->dispatch(entity);
    delete scheduled;
    ASSERT_EQ(model->getFutureEvents()->size(), 1u);
    Event* forwarded = model->getFutureEvents()->front();
    EXPECT_EQ(forwarded->getEntity(), entity);
    EXPECT_EQ(forwarded->getComponent(), receiver->getConnectionManager()->getFrontConnection()->component);
    EXPECT_DOUBLE_EQ(forwarded->getTime(), 0.0);
}

TEST_F(TransferFixture, MinuteDelayConvertsIntoSecondBaseUnit) {
    auto* receiver = makeLabel("Receiver");
    auto* sender = new GotoProbe(model, "Sender");
    sender->setLabelDefinition(receiver->getLabelDefinition());
    sender->setTimeDelayExpression("2.0");
    sender->setTimeDelayTimeUnit(Util::TimeUnit::minute);
    auto* entity = model->createEntity("DelayedEntity", true);
    sender->dispatch(entity);
    ASSERT_EQ(model->getFutureEvents()->size(), 1u);
    Event* event = model->getFutureEvents()->front();
    EXPECT_DOUBLE_EQ(event->getTime(), 120.0);
    EXPECT_EQ(event->getComponent(), receiver);
    EXPECT_EQ(event->getEntity(), entity);
}

TEST_F(TransferFixture, NegativeDelayDoesNotScheduleEvent) {
    auto* receiver = makeLabel("Receiver");
    auto* sender = new GotoProbe(model, "Sender");
    sender->setLabelDefinition(receiver->getLabelDefinition());
    sender->setTimeDelayExpression("-1");
    sender->dispatch(model->createEntity("BadDelay", true));
    EXPECT_TRUE(model->getFutureEvents()->empty());
}

TEST_F(TransferFixture, TwoGotosTargetSameDefinitionWithoutDuplication) {
    auto* receiver = makeLabel("Shared");
    auto* first = new GotoProbe(model, "Sender1");
    auto* second = new GotoProbe(model, "Sender2");
    first->setLabelDefinition(receiver->getLabelDefinition());
    second->setLabelDefinition(receiver->getLabelDefinition());
    auto* entityA = model->createEntity("A", true);
    auto* entityB = model->createEntity("B", true);
    first->dispatch(entityA);
    second->dispatch(entityB);
    ASSERT_EQ(model->getFutureEvents()->size(), 2u);
    auto it = model->getFutureEvents()->list()->begin();
    EXPECT_EQ((*it)->getEntity(), entityA);
    EXPECT_EQ((*it++)->getComponent(), receiver);
    EXPECT_EQ((*it)->getEntity(), entityB);
    EXPECT_EQ((*it)->getComponent(), receiver);
}

TEST_F(TransferFixture, RemovingLabelInvalidatesAttachedGotoDestination) {
    auto* receiver = makeLabel("TransientReceiver");
    auto* sender = new GotoProbe(model, "ObservingSender");
    auto* definition = receiver->getLabelDefinition();
    ASSERT_NE(definition, nullptr);
    sender->setLabelDefinition(definition);
    ASSERT_EQ(sender->getLabelDefinition(), definition);

    delete receiver; // Deletes the internally owned LabelDefinition once.

    EXPECT_EQ(sender->getLabelDefinition(), nullptr);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<LabelDefinition>()), 0u);
    std::string error;
    EXPECT_FALSE(sender->check(error));
    EXPECT_NE(error.find("LabelDefinition"), std::string::npos);
    EXPECT_EQ(sender->getAttachedData()->count("LabelDefinition"), 0u);
}

TEST_F(TransferFixture, RemovingLabelDetachesRouteAndSequenceDirectReferences) {
    auto* receiver = makeLabel("DisappearingReceiver");
    auto* definition = receiver->getLabelDefinition();
    ASSERT_NE(definition, nullptr);
    auto* sender = new GotoProbe(model, "GotoObserver");
    sender->setLabelDefinition(definition);
    auto* route = new Route(model, "RouteObserver");
    route->setLabel(definition);
    auto* sequence = new Sequence(model, "SequenceObserver");
    sequence->getSteps()->insert(new SequenceStep(definition));

    delete receiver;

    EXPECT_EQ(sender->getLabelDefinition(), nullptr);
    EXPECT_EQ(sender->getAttachedData()->count("LabelDefinition"), 0u);
    EXPECT_EQ(route->getLabel(), nullptr);
    ASSERT_EQ(sequence->getSteps()->size(), 1u);
    EXPECT_EQ(sequence->getSteps()->list()->front()->getLabel(), nullptr);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<LabelDefinition>()), 0u);
}

TEST_F(TransferFixture, IndependentDefinitionsRemainIsolated) {
    auto* first = makeLabel("First");
    auto* second = makeLabel("Second");
    auto* gotoA = new GotoProbe(model, "GotoA");
    auto* gotoB = new GotoProbe(model, "GotoB");
    gotoA->setLabelDefinition(first->getLabelDefinition());
    gotoB->setLabelDefinition(second->getLabelDefinition());
    gotoA->dispatch(model->createEntity("A", true));
    gotoB->dispatch(model->createEntity("B", true));
    ASSERT_EQ(model->getFutureEvents()->size(), 2u);
    auto it = model->getFutureEvents()->list()->begin();
    EXPECT_EQ((*it++)->getComponent(), first);
    EXPECT_EQ((*it)->getComponent(), second);
}

TEST_F(TransferFixture, GenRoundTripReconstructsTransferAndSimulates) {
    auto* create = new Create(model, "CreateEntity");
    auto* gotoComponent = new Goto(model, "ToLabel");
    auto* label = new Label(model, "Arrival");
    auto* dispose = new Dispose(model, "DisposeEntity");
    auto* entityType = new EntityType(model, "TransferType");
    create->setEntityType(entityType);
    create->setFirstCreation(0.0);
    create->setTimeBetweenCreationsExpression("1.0");
    create->setMaxCreations(3);
    create->connectTo(gotoComponent);
    label->connectTo(dispose);
    gotoComponent->setLabelDefinition(label->getLabelDefinition());
    gotoComponent->setTimeDelayExpression("2.0");
    gotoComponent->setTimeDelayTimeUnit(Util::TimeUnit::second);
    model->getSimulation()->setNumberOfReplications(1);

    ASSERT_TRUE(model->check());
    const std::string filename = "/tmp/genesys_transfer_label_goto_" +
        std::to_string(::getpid()) + ".gen";
    ASSERT_TRUE(model->save(filename));
    Model* restored = simulator.getModelManager()->newModel();
    ASSERT_NE(restored, nullptr);
    const bool loaded = restored->load(filename);
    std::remove(filename.c_str());
    ASSERT_TRUE(loaded);
    auto* loadedGoto = dynamic_cast<Goto*>(restored->getComponentManager()->find("ToLabel"));
    auto* loadedLabel = dynamic_cast<Label*>(restored->getComponentManager()->find("Arrival"));
    ASSERT_NE(loadedGoto, nullptr);
    ASSERT_NE(loadedLabel, nullptr);
    ASSERT_NE(loadedLabel->getLabelDefinition(), nullptr);
    EXPECT_EQ(loadedGoto->getLabelDefinition(), loadedLabel->getLabelDefinition());
    EXPECT_EQ(loadedLabel->getLabelDefinition()->getEnterIntoLabelComponent(), loadedLabel);
    EXPECT_EQ(loadedGoto->getTimeDelayExpression(), "2.0");
    EXPECT_TRUE(restored->check());
    restored->getSimulation()->start();
    EXPECT_TRUE(restored->getTracer()->errorMessages()->empty());
    EXPECT_EQ(restored->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Entity>()), 0u);
}
