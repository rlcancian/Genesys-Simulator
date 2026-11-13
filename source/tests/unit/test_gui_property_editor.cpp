#include <gtest/gtest.h>
#include <vector>

#include "mainwindow.h"
#include "actions/AddUndoCommand.h"
#include "actions/DeleteUndoCommand.h"
#include "graphicals/GraphicalModelDataDefinition.h"
#include "graphicals/ModelGraphicsView.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalModelComponent.h"
#include "propertyeditor/ObjectPropertyBrowser.h"
#include "services/ModelLanguageSynchronizer.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Assign.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/components/DiscreteProcessing/Process.h"
#include "plugins/components/DiscreteProcessing/auxiliar/SeizableItem.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/AI/AIAssistant.h"
#include "plugins/components/Continuous/ContinuousSystemComponent.h"
#include "plugins/components/Continuous/DiffEquations.h"
#include "plugins/components/Continuous/LSODE.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/data/ModalModel/GraphNetwork.h"
#include "plugins/data/ModalModel/DirectedGraphNetwork.h"
#include "plugins/data/ModalModel/DirectedAcyclicGraphNetwork.h"
#include "plugins/data/ModalModel/MarkovChainNetwork.h"
#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"
#include "plugins/components/Template/DummyComponent.h"
#include "plugins/components/ExternalIntegration/PythonForG.h"
#include "plugins/components/ExternalIntegration/CppForG.h"
#include "plugins/components/ExternalIntegration/RSimulator.h"
#include "plugins/components/Decisions/Search.h"
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/Decisions/Remove.h"
#include "plugins/components/Transfer/PickStation.h"
#include "plugins/components/Synchronization/Match.h"
#include "plugins/components/ElectronicsSimulation/SPICECircuit.h"
#include "plugins/components/Synchronization/Signal.h"
#include "plugins/components/Synchronization/Wait.h"
#include "plugins/data/BiochemicalSimulation/BacteriaSignalGrid.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/GroProgram.h"
#include "plugins/data/Synchronization/SignalData.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/data/DiscreteProcessing/Failure.h"
#include "plugins/data/Logic/Set.h"
#include "plugins/data/AI/AISupport.h"
#include "plugins/data/Continuous/ODESolver.h"
#include "plugins/data/ExternalIntegration/CppCompiler.h"
#include "plugins/data/ExternalIntegration/SPICERunner.h"
#include "plugins/data/ExternalIntegration/RSimulatorRunner.h"
#include "plugins/data/Template/DummyElement.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"
#include "plugins/data/Grouping/EntityGroup.h"

#include <functional>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QFile>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QLocale>
#include <QPlainTextEdit>
#include <QSet>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QString>
#include <QTabWidget>
#include <QTimer>
#include <QUndoStack>

namespace {

QtBrowserItem* findBrowserItemByName(const QList<QtBrowserItem*>& items, const QString& propertyName) {
    for (QtBrowserItem* item : items) {
        if (item == nullptr || item->property() == nullptr) {
            continue;
        }
        if (item->property()->propertyName() == propertyName) {
            return item;
        }
        if (QtBrowserItem* child = findBrowserItemByName(item->children(), propertyName)) {
            return child;
        }
    }
    return nullptr;
}

void drainGuiEvents() {
    for (int i = 0; i < 8; ++i) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }
}

class ScopedDefaultLocale {
public:
    explicit ScopedDefaultLocale(const QLocale& locale)
        : _previous(QLocale()) {
        QLocale::setDefault(locale);
    }

    ~ScopedDefaultLocale() {
        QLocale::setDefault(_previous);
    }

private:
    QLocale _previous;
};

class GenericDoubleDataDefinition final : public ModelDataDefinition {
public:
    explicit GenericDoubleDataDefinition(Model* model)
        : ModelDataDefinition(model, "GenericDoubleDataDefinition", "GenericDouble_PropertyEditor") {
        auto* doubleControl = new SimulationControlDouble(
            [this]() { return _value; },
            [this](double value) { _value = value; ++_valueSetCount; },
            getClassname(),
            getName(),
            "Value",
            "Generic SimulationControlDouble regression value");
        auto* boolControl = new SimulationControlBool(
            [this]() { return _enabled; },
            [this](bool enabled) { _enabled = enabled; },
            getClassname(),
            getName(),
            "Enabled",
            "Generic SimulationControlBool regression value");
        _parentModel->getControls()->insert(doubleControl);
        _parentModel->getControls()->insert(boolControl);
        _addSimulationControl(doubleControl);
        _addSimulationControl(boolControl);
    }

    double value() const {
        return _value;
    }

    bool enabled() const {
        return _enabled;
    }

    int valueSetCount() const {
        return _valueSetCount;
    }

private:
    double _value = 0.0;
    bool _enabled = false;
    int _valueSetCount = 0;
};

} // namespace

void bindEditableKernelObject(ObjectPropertyBrowser& browser,
                              ModelDataDefinition* object,
                              int* modelChangedCount = nullptr) {
    ASSERT_NE(object, nullptr);
    if (modelChangedCount != nullptr) {
        browser.setModelChangedCallback([modelChangedCount]() {
            ++(*modelChangedCount);
        });
    }

    QSet<QString> editableObjects;
    editableObjects.insert(QString::fromStdString(object->getName()));
    browser.setActiveObject(
        nullptr,
        object,
        {},
        editableObjects,
        nullptr,
        nullptr,
        nullptr,
        nullptr);
    drainGuiEvents();
}

void commitTextWithEnter(QLineEdit* lineEdit, const QString& text) {
    ASSERT_NE(lineEdit, nullptr);
    lineEdit->setFocus();
    lineEdit->selectAll();
    lineEdit->setText(text);

    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(lineEdit, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(lineEdit, &release);
    drainGuiEvents();
}

void selectListItemAction(ObjectPropertyBrowser& browser, const QString& listName,
                          const QString& itemName, const QString& action) {
    QtBrowserItem* list = findBrowserItemByName(browser.topLevelItems(), listName);
    ASSERT_NE(list, nullptr);
    browser.setExpanded(list, true);
    QtBrowserItem* item = findBrowserItemByName(list->children(), itemName);
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    QComboBox* selector = nullptr;
    for (QComboBox* candidate : browser.findChildren<QComboBox*>()) {
        if (candidate->isVisible() && candidate->findText(action) >= 0) { selector = candidate; break; }
    }
    ASSERT_NE(selector, nullptr);
    selector->setCurrentIndex(selector->findText(action));
    drainGuiEvents();
}

TEST(PropertyEditorLists, MovesSimpleAndObjectListItemsByPosition) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    LSODE lsode(model, "LSODE_MoveList");
    lsode.addDiffEquation("same");
    lsode.addDiffEquation("same");
    lsode.addDiffEquation("last");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &lsode);
    selectListItemAction(browser, "DiffEquations", "DiffEquation 2", "Mover para cima");
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(0), "same");
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(1), "last");
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(2), "same");
    ASSERT_NE(browser.currentItem(), nullptr);
    EXPECT_EQ(browser.currentItem()->property()->propertyName(), "DiffEquation 1");

    Assign assign(model, "Assign_MoveList");
    auto* first = new Assignment("Entity.first", "1");
    auto* second = new Assignment("Entity.second", "2");
    assign.addAssignment(first);
    assign.addAssignment(second);
    bindEditableKernelObject(browser, &assign);
    selectListItemAction(browser, "Assignments", "Item 1 - Entity.second", "Mover para cima");
    EXPECT_EQ(assign.getAssignments()->getAtRank(0), second);
    EXPECT_EQ(assign.getAssignments()->getAtRank(1), first);
    selectListItemAction(browser, "Assignments", "Item 0 - Entity.second", "Mover para baixo");
    EXPECT_EQ(assign.getAssignments()->getAtRank(0), first);
    EXPECT_EQ(assign.getAssignments()->getAtRank(1), second);
}

TEST(PropertyEditorLists, ListItemActionsRespectBoundariesAndReadOnly) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    LSODE lsode(model, "LSODE_ReadOnlyList");
    lsode.addDiffEquation("first");
    lsode.addDiffEquation("second");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &lsode);
    QtBrowserItem* list = findBrowserItemByName(browser.topLevelItems(), "DiffEquations");
    ASSERT_NE(list, nullptr);
    browser.setExpanded(list, true);
    QtBrowserItem* first = findBrowserItemByName(list->children(), "DiffEquation 0");
    ASSERT_NE(first, nullptr);
    browser.setCurrentItem(first);
    browser.editItem(first);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int upIndex = selector->findText("Mover para cima");
    ASSERT_GE(upIndex, 0);
    auto* selectorModel = qobject_cast<QStandardItemModel*>(selector->model());
    ASSERT_NE(selectorModel, nullptr);
    EXPECT_FALSE(selectorModel->item(upIndex)->isEnabled());
    EXPECT_GE(selector->findText("Mover para baixo"), 0);
    QtBrowserItem* last = findBrowserItemByName(list->children(), "DiffEquation 1");
    ASSERT_NE(last, nullptr);
    browser.setCurrentItem(last);
    browser.editItem(last);
    drainGuiEvents();
    QComboBox* lastSelector = nullptr;
    for (QComboBox* candidate : browser.findChildren<QComboBox*>()) {
        if (candidate->isVisible()) { lastSelector = candidate; break; }
    }
    ASSERT_NE(lastSelector, nullptr);
    auto* lastModel = qobject_cast<QStandardItemModel*>(lastSelector->model());
    ASSERT_NE(lastModel, nullptr);
    const int downIndex = lastSelector->findText("Mover para baixo");
    ASSERT_GE(downIndex, 0);
    EXPECT_FALSE(lastModel->item(downIndex)->isEnabled());
    Set resourceSet(model, "ReadOnlySet");
    auto* resource = new Resource(model, "ReadOnlyResource");
    resourceSet.addElementSet(resource);
    browser.setActiveObject(nullptr, &resourceSet, {}, {});
    drainGuiEvents();
    list = findBrowserItemByName(browser.topLevelItems(), "ElementSet");
    ASSERT_NE(list, nullptr);
    first = findBrowserItemByName(list->children(), "Item 0 - ReadOnlyResource");
    ASSERT_NE(first, nullptr);
    EXPECT_FALSE(first->property()->isEnabled());
    EXPECT_EQ(resourceSet.getElementSet()->getAtRank(0), resource);
}

TEST(PropertyEditorLists, ContextMenuMovesSelectedItemAndDisablesBoundary) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    LSODE lsode(model, "LSODE_ContextMove");
    lsode.addDiffEquation("first");
    lsode.addDiffEquation("second");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &lsode);
    QtBrowserItem* list = findBrowserItemByName(browser.topLevelItems(), "DiffEquations");
    ASSERT_NE(list, nullptr);
    browser.setExpanded(list, true);
    QtBrowserItem* first = findBrowserItemByName(list->children(), "DiffEquation 0");
    ASSERT_NE(first, nullptr);
    browser.setCurrentItem(first);
    bool checkedMenu = false;
    QTimer::singleShot(0, [&checkedMenu]() {
        auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
        if (menu == nullptr) return;
        QAction* up = nullptr;
        QAction* down = nullptr;
        for (QAction* action : menu->actions()) {
            if (action->text() == "Mover para cima") up = action;
            if (action->text() == "Mover para baixo") down = action;
        }
        checkedMenu = up != nullptr && !up->isEnabled() && down != nullptr && down->isEnabled();
        if (down == nullptr) { menu->close(); return; }
        menu->setActiveAction(down);
        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(menu, &enter);
    });
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(5, 5), browser.mapToGlobal(QPoint(5, 5)));
    QApplication::sendEvent(&browser, &event);
    drainGuiEvents();
    EXPECT_TRUE(checkedMenu);
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(0), "second");
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(1), "first");
}

TEST(PropertyEditorLists, IgnoresStaleActionsBeforeDeferredRebuild) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    LSODE lsode(model, "LSODE_DeferredMove");
    lsode.addDiffEquation("first");
    lsode.addDiffEquation("second");
    lsode.addDiffEquation("third");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &lsode);
    QtBrowserItem* list = findBrowserItemByName(browser.topLevelItems(), "DiffEquations");
    ASSERT_NE(list, nullptr);
    browser.setExpanded(list, true);
    QtBrowserItem* item = findBrowserItemByName(list->children(), "DiffEquation 1");
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int down = selector->findText("Mover para baixo");
    const int up = selector->findText("Mover para cima");
    ASSERT_GE(down, 0);
    ASSERT_GE(up, 0);
    selector->setCurrentIndex(down);
    selector->setCurrentIndex(up);
    drainGuiEvents();
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(0), "first");
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(1), "third");
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(2), "second");
}

TEST(PropertyEditorLists, MovesSeizeAndReleaseRequests) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Seize seize(model, "Seize_MoveList");
    auto* first = new SeizableItem(model, "ResourceFirst");
    auto* second = new SeizableItem(model, "ResourceSecond");
    seize.addRequest(first);
    seize.addRequest(second);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &seize);
    selectListItemAction(browser, "Requests", "Item 1 - ResourceSecond", "Mover para cima");
    EXPECT_EQ(seize.getSeizeRequests()->getAtRank(0), second);
    EXPECT_EQ(seize.getSeizeRequests()->getAtRank(1), first);

    Release release(model, "Release_MoveList");
    auto* releaseFirst = new SeizableItem(model, "ResourceFirst");
    auto* releaseSecond = new SeizableItem(model, "ResourceSecond");
    release.addReleaseRequests(releaseFirst);
    release.addReleaseRequests(releaseSecond);
    bindEditableKernelObject(browser, &release);
    selectListItemAction(browser, "ReleaseRequests", "Item 1 - ResourceSecond", "Mover para cima");
    EXPECT_EQ(release.getReleaseRequests()->getAtRank(0), releaseSecond);
    EXPECT_EQ(release.getReleaseRequests()->getAtRank(1), releaseFirst);
}

TEST(PropertyEditorLists, MovesNonOwningDataDefinitionListsWithoutDeletingMembers) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* first = new Resource(model, "ListMemberFirst");
    auto* second = new Resource(model, "ListMemberSecond");
    Set resourceSet(model, "EditableSet");
    resourceSet.addElementSet(first);
    resourceSet.addElementSet(second);
    ModelGraphicsScene scene(0, 0, 1000, 800);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    browser.setModelScene(&scene);
    bindEditableKernelObject(browser, &resourceSet);
    selectListItemAction(browser, "ElementSet", "Item 1 - ListMemberSecond", "Mover para cima");
    EXPECT_EQ(resourceSet.getElementSet()->getAtRank(0), second);
    ASSERT_TRUE(undoStack.canUndo());
    undoStack.undo();
    drainGuiEvents();
    drainGuiEvents();
    EXPECT_EQ(resourceSet.getElementSet()->getAtRank(0), first);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), "ListMemberSecond"), second);

    Failure failure(model, "EditableFailure");
    failure.addResource(first);
    failure.addResource(second);
    bindEditableKernelObject(browser, &failure);
    selectListItemAction(browser, "FalingResources", "Item 1 - ListMemberSecond", "Mover para cima");
    EXPECT_EQ(failure.falingResources()->getAtRank(0), second);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), "ListMemberFirst"), first);
}

TEST(PropertyEditorLists, MovesPickStationItemsWithMatchingNames) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    PickStation pick(model, "PickStation_MoveList");
    auto* first = new PickableStationItem(static_cast<Station*>(nullptr), std::string("1"));
    auto* second = new PickableStationItem(static_cast<Station*>(nullptr), std::string("2"));
    pick.addPickableStationItem(first);
    pick.addPickableStationItem(second);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &pick);
    selectListItemAction(browser, "PickableStationItens", "Item 1", "Mover para cima");
    EXPECT_EQ(pick.getPickableStationItens()->getAtRank(0), second);
    EXPECT_EQ(pick.getPickableStationItens()->getAtRank(1), first);
}

TEST(PropertyEditorLists, ProcessMoveReconcilesInternalReleaseOrder) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Process process(model, "Process_MoveList");
    auto* firstResource = new Resource(model, "ProcessResourceFirst");
    auto* secondResource = new Resource(model, "ProcessResourceSecond");
    auto* first = new SeizableItem(firstResource);
    auto* second = new SeizableItem(secondResource);
    process.addSeizeRequest(first);
    process.addSeizeRequest(second);
    ModelDataDefinition::CreateInternalData(&process);
    auto* release = dynamic_cast<Release*>(process.getInternalData("Release"));
    ASSERT_NE(release, nullptr);
    ASSERT_EQ(release->getReleaseRequests()->size(), 2u);
    auto* firstRelease = release->getReleaseRequests()->getAtRank(0);
    auto* secondRelease = release->getReleaseRequests()->getAtRank(1);
    Plugin* plugin = simulator.getPluginManager()->find(Util::TypeOf<Process>());
    ASSERT_NE(plugin, nullptr);
    ModelGraphicsScene scene(0, 0, 1000, 800);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    auto* graphical = new GraphicalModelComponent(plugin, &process, QPointF(100, 100), Qt::blue);
    scene.addItem(graphical);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    browser.setActiveObject(graphical, &process, {}, {QString::fromStdString(process.getName())});
    drainGuiEvents();
    selectListItemAction(browser, "SeizeRequests", "Item 1 - ProcessResourceSecond", "Mover para cima");
    EXPECT_EQ(process.getSeizeRequests()->getAtRank(0), second);
    EXPECT_EQ(process.getSeizeRequests()->getAtRank(1), first);
    ASSERT_EQ(release->getReleaseRequests()->size(), 2u);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(0)->getResource(), secondResource);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(1)->getResource(), firstResource);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(0), secondRelease);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(1), firstRelease);
    ASSERT_TRUE(undoStack.canUndo());
    undoStack.undo();
    drainGuiEvents();
    EXPECT_EQ(process.getSeizeRequests()->getAtRank(0), first);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(0)->getResource(), firstResource);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(0), firstRelease);
    undoStack.redo();
    drainGuiEvents();
    drainGuiEvents();
    EXPECT_EQ(process.getSeizeRequests()->getAtRank(0), second);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(0)->getResource(), secondResource);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(0), secondRelease);
}

TEST(PropertyEditorLists, ProcessMoveReconcilesAnInitiallyUnbuiltReleaseList) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Process process(model, "Process_MoveUnbuiltList");
    auto* firstResource = new Resource(model, "UnbuiltFirst");
    auto* secondResource = new Resource(model, "UnbuiltSecond");
    auto* first = new SeizableItem(firstResource);
    auto* second = new SeizableItem(secondResource);
    process.addSeizeRequest(first);
    process.addSeizeRequest(second);
    auto* release = dynamic_cast<Release*>(process.getInternalData("Release"));
    ASSERT_NE(release, nullptr);
    ASSERT_EQ(release->getReleaseRequests()->size(), 0u);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &process);
    selectListItemAction(browser, "SeizeRequests", "Item 1 - UnbuiltSecond", "Mover para cima");
    EXPECT_EQ(process.getSeizeRequests()->getAtRank(0), second);
    ASSERT_EQ(release->getReleaseRequests()->size(), 2u);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(0)->getResource(), secondResource);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(1)->getResource(), firstResource);
    second->setQuantityExpression("3");
    selectListItemAction(browser, "SeizeRequests", "Item 0 - UnbuiltSecond", "Mover para baixo");
    EXPECT_EQ(process.getSeizeRequests()->getAtRank(1), second);
    EXPECT_EQ(release->getReleaseRequests()->getAtRank(1)->getQuantityExpression(), "3");
}

TEST(PropertyEditorLists, DecideMovesGraphicalPortsAndConnectionsWithUndoRedo) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* decide = new Decide(model, "Decide_MoveList");
    decide->addConditions("first");
    decide->addConditions("second");
    auto* target = new Dispose(model, "Move_Target");
    auto* otherwise = new Dispose(model, "Move_Else");
    Plugin* decidePlugin = simulator.getPluginManager()->find(Util::TypeOf<Decide>());
    Plugin* disposePlugin = simulator.getPluginManager()->find(Util::TypeOf<Dispose>());
    ASSERT_NE(decidePlugin, nullptr);
    ASSERT_NE(disposePlugin, nullptr);
    ModelGraphicsScene scene(0, 0, 1000, 800);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    auto* graphical = new GraphicalModelComponent(decidePlugin, decide, QPointF(100, 100), Qt::blue);
    auto* targetGraphical = new GraphicalModelComponent(disposePlugin, target, QPointF(400, 100), Qt::blue);
    auto* elseGraphical = new GraphicalModelComponent(disposePlugin, otherwise, QPointF(400, 300), Qt::blue);
    scene.addItem(graphical);
    scene.addItem(targetGraphical);
    scene.addItem(elseGraphical);
    scene.getAllComponents()->append(graphical);
    scene.getAllComponents()->append(targetGraphical);
    scene.getAllComponents()->append(elseGraphical);
    auto* firstConnection = new Connection{target, {0}};
    auto* elseConnection = new Connection{otherwise, {0}};
    decide->getConnectionManager()->insertAtPort(0, firstConnection);
    decide->getConnectionManager()->insertAtPort(2, elseConnection);
    auto* firstPort = graphical->getGraphicalOutputPorts().at(0);
    auto* elsePort = graphical->getGraphicalOutputPorts().at(2);
    scene.addGraphicalConnection(firstPort, targetGraphical->getGraphicalInputPorts().at(0), 0, 0);
    scene.addGraphicalConnection(elsePort, elseGraphical->getGraphicalInputPorts().at(0), 0, 0);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    browser.setActiveObject(graphical, decide, {}, {QString::fromStdString(decide->getName())});
    drainGuiEvents();
    selectListItemAction(browser, "Conditions", "Condition 0", "Mover para baixo");
    EXPECT_EQ(decide->getConditions()->getAtRank(1), "first");
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(1), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(1), firstPort);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(2), elsePort);
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(2), elseConnection);
    ASSERT_TRUE(undoStack.canUndo());
    undoStack.undo();
    drainGuiEvents();
    EXPECT_EQ(decide->getConditions()->getAtRank(0), "first");
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(0), firstPort);
    ASSERT_NE(browser.currentItem(), nullptr);
    EXPECT_EQ(browser.currentItem()->property()->propertyName(), "Condition 0");
    undoStack.redo();
    drainGuiEvents();
    EXPECT_EQ(decide->getConditions()->getAtRank(1), "first");
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(1), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(1), firstPort);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(2), elsePort);
    ASSERT_NE(browser.currentItem(), nullptr);
    EXPECT_EQ(browser.currentItem()->property()->propertyName(), "Condition 1");
    browser.setModelScene(&scene);
    browser.setActiveObject(nullptr, decide, {}, {QString::fromStdString(decide->getName())});
    drainGuiEvents();
    selectListItemAction(browser, "Conditions", "Condition 1", "Mover para cima");
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(0), firstPort);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(2), elsePort);
    undoStack.undo();
    drainGuiEvents();
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(1), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(1), firstPort);
    undoStack.redo();
    drainGuiEvents();
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(0), firstPort);
}

TEST(PropertyEditorLists, AssignmentMoveSupportsUndoAndRedo) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* assign = new Assign(model, "Assign_UndoList");
    auto* first = new Assignment("Entity.first", "1");
    auto* second = new Assignment("Entity.second", "2");
    assign->addAssignment(first);
    assign->addAssignment(second);
    Plugin* plugin = simulator.getPluginManager()->find(Util::TypeOf<Assign>());
    ASSERT_NE(plugin, nullptr);
    ModelGraphicsScene scene(0, 0, 1000, 800);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    auto* graphical = new GraphicalModelComponent(plugin, assign, QPointF(100, 100), Qt::blue);
    scene.addItem(graphical);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    browser.setActiveObject(graphical, assign, {}, {QString::fromStdString(assign->getName())});
    drainGuiEvents();
    selectListItemAction(browser, "Assignments", "Item 0 - Entity.first", "Mover para baixo");
    EXPECT_EQ(assign->getAssignments()->getAtRank(1), first);
    ASSERT_TRUE(undoStack.canUndo());
    undoStack.undo();
    drainGuiEvents();
    EXPECT_EQ(assign->getAssignments()->getAtRank(0), first);
    EXPECT_EQ(assign->getAssignments()->getAtRank(1), second);
    undoStack.redo();
    drainGuiEvents();
    EXPECT_EQ(assign->getAssignments()->getAtRank(0), second);
    EXPECT_EQ(assign->getAssignments()->getAtRank(1), first);
}

TEST(PropertyEditorLists, InspectorListMoveUsesSceneUndoWithoutGraphicalItem) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    LSODE lsode(model, "LSODE_InspectorUndo");
    lsode.addDiffEquation("first");
    lsode.addDiffEquation("second");
    ModelGraphicsScene scene(0, 0, 1000, 800);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    browser.setModelScene(&scene);
    bindEditableKernelObject(browser, &lsode);
    selectListItemAction(browser, "DiffEquations", "DiffEquation 0", "Mover para baixo");
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(0), "second");
    ASSERT_TRUE(undoStack.canUndo());
    undoStack.undo();
    drainGuiEvents();
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(0), "first");
    undoStack.redo();
    drainGuiEvents();
    EXPECT_EQ(lsode.getDiffEquations()->getAtRank(0), "second");
}

TEST(PropertyEditorLists, LSODECreatesAndEditsEquationsInPropertyTree) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    LSODE component(model, "LSODE_PropertyEditor");
    component.addDiffEquation("existing + 1");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);

    QtBrowserItem* equations = findBrowserItemByName(browser.topLevelItems(), "DiffEquations");
    ASSERT_NE(equations, nullptr);
    browser.setCurrentItem(equations);
    browser.editItem(equations);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int createIndex = selector->findText("Criar novo DiffEquation");
    ASSERT_GE(createIndex, 0);
    selector->setCurrentIndex(createIndex);
    drainGuiEvents();
    ASSERT_EQ(component.getDiffEquations()->size(), 2u);

    equations = findBrowserItemByName(browser.topLevelItems(), "DiffEquations");
    ASSERT_NE(equations, nullptr);
    browser.setExpanded(equations, true);
    QtBrowserItem* equation = findBrowserItemByName(equations->children(), "DiffEquation 1");
    ASSERT_NE(equation, nullptr);
    browser.setExpanded(equation, true);
    QtBrowserItem* equationValue = findBrowserItemByName(equation->children(), "Value");
    ASSERT_NE(equationValue, nullptr);
    browser.setCurrentItem(equationValue);
    browser.editItem(equationValue);
    drainGuiEvents();
    QLineEdit* editor = browser.findChild<QLineEdit*>();
    ASSERT_NE(editor, nullptr);
    commitTextWithEnter(editor, "-2 * concentration");
    EXPECT_EQ(component.getDiffEquations()->getAtRank(0), "existing + 1");
    EXPECT_EQ(component.getDiffEquations()->getAtRank(1), "-2 * concentration");
}

TEST(PropertyEditorLists, DecideCreationUpdatesGraphicalPortsAndUndo) {
    for (bool inspector : {false, true}) {
        Simulator simulator;
        simulator.getPluginManager()->autoInsertPlugins();
        Model* model = simulator.getModelManager()->newModel();
        ASSERT_NE(model, nullptr);
        auto* decide = new Decide(model, "Decide_CreatePorts");
        auto* target = new Dispose(model, "Decide_ElseTarget");
        Plugin* decidePlugin = simulator.getPluginManager()->find(Util::TypeOf<Decide>());
        Plugin* disposePlugin = simulator.getPluginManager()->find(Util::TypeOf<Dispose>());
        ASSERT_NE(decidePlugin, nullptr);
        ASSERT_NE(disposePlugin, nullptr);
        ModelGraphicsScene scene(0, 0, 1000, 800);
        QUndoStack undoStack;
        scene.setUndoStack(&undoStack);
        auto* graphical = new GraphicalModelComponent(decidePlugin, decide, QPointF(100, 100), Qt::blue);
        auto* graphicalTarget = new GraphicalModelComponent(disposePlugin, target, QPointF(400, 100), Qt::blue);
        scene.addItem(graphical);
        scene.addItem(graphicalTarget);
        scene.getAllComponents()->append(graphical);
        scene.getAllComponents()->append(graphicalTarget);
        auto* connection = new Connection{target, {0}};
        decide->getConnectionManager()->insertAtPort(0, connection);
        auto* elsePort = graphical->getGraphicalOutputPorts().at(0);
        scene.addGraphicalConnection(elsePort, graphicalTarget->getGraphicalInputPorts().at(0), 0, 0);
        ObjectPropertyBrowser browser;
        browser.resize(640, 480);
        browser.show();
        browser.setModelScene(&scene);
        browser.setActiveObject(inspector ? nullptr : graphical, decide, {}, {QString::fromStdString(decide->getName())});
        drainGuiEvents();
        QtBrowserItem* conditions = findBrowserItemByName(browser.topLevelItems(), "Conditions");
        ASSERT_NE(conditions, nullptr);
        browser.setCurrentItem(conditions);
        browser.editItem(conditions);
        drainGuiEvents();
        QComboBox* selector = browser.findChild<QComboBox*>();
        ASSERT_NE(selector, nullptr);
        const int createIndex = selector->findText("Criar novo Condition");
        ASSERT_GE(createIndex, 0);
        selector->setCurrentIndex(createIndex);
        drainGuiEvents();
        ASSERT_EQ(decide->getConditions()->size(), 1u);
        ASSERT_EQ(graphical->getGraphicalOutputPorts().size(), 2);
        EXPECT_EQ(graphical->getGraphicalOutputPorts().at(1), elsePort);
        EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(1), connection);
        ASSERT_TRUE(undoStack.canUndo());
        undoStack.undo();
        drainGuiEvents();
        ASSERT_EQ(graphical->getGraphicalOutputPorts().size(), 1);
        EXPECT_EQ(graphical->getGraphicalOutputPorts().at(0), elsePort);
        EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0), connection);
        undoStack.redo();
        drainGuiEvents();
        ASSERT_EQ(graphical->getGraphicalOutputPorts().size(), 2);
        EXPECT_EQ(graphical->getGraphicalOutputPorts().at(1), elsePort);
        EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(1), connection);
        conditions = findBrowserItemByName(browser.topLevelItems(), "Conditions");
        ASSERT_NE(conditions, nullptr);
        browser.setCurrentItem(conditions);
        browser.editItem(conditions);
        drainGuiEvents();
        selector = browser.findChild<QComboBox*>();
        ASSERT_NE(selector, nullptr);
        const int secondCreateIndex = selector->findText("Criar novo Condition");
        ASSERT_GE(secondCreateIndex, 0);
        selector->setCurrentIndex(secondCreateIndex);
        drainGuiEvents();
        ASSERT_EQ(decide->getConditions()->size(), 2u);
        ASSERT_EQ(graphical->getGraphicalOutputPorts().size(), 3);
        EXPECT_EQ(graphical->getGraphicalOutputPorts().at(2), elsePort);
        EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(2), connection);
        undoStack.undo();
        drainGuiEvents();
        ASSERT_EQ(decide->getConditions()->size(), 1u);
        ASSERT_EQ(graphical->getGraphicalOutputPorts().size(), 2);
        EXPECT_EQ(graphical->getGraphicalOutputPorts().at(1), elsePort);
        undoStack.redo();
        drainGuiEvents();
        ASSERT_EQ(graphical->getGraphicalOutputPorts().size(), 3);
        EXPECT_EQ(graphical->getGraphicalOutputPorts().at(2), elsePort);
        EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(2), connection);
    }
}

TEST(PropertyEditorLists, DecideCreatesAndEditsConditionInPropertyTree) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Decide decide(model, "Decide_PropertyEditor");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &decide);

    QtBrowserItem* conditions = findBrowserItemByName(browser.topLevelItems(), "Conditions");
    ASSERT_NE(conditions, nullptr);
    browser.setCurrentItem(conditions);
    browser.editItem(conditions);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int createIndex = selector->findText("Criar novo Condition");
    ASSERT_GE(createIndex, 0);
    selector->setCurrentIndex(createIndex);
    drainGuiEvents();
    ASSERT_EQ(decide.getConditions()->size(), 1u);

    conditions = findBrowserItemByName(browser.topLevelItems(), "Conditions");
    ASSERT_NE(conditions, nullptr);
    browser.setExpanded(conditions, true);
    QtBrowserItem* condition = findBrowserItemByName(conditions->children(), "Condition 0");
    ASSERT_NE(condition, nullptr);
    browser.setExpanded(condition, true);
    QtBrowserItem* conditionValue = findBrowserItemByName(condition->children(), "Value");
    ASSERT_NE(conditionValue, nullptr);
    browser.setCurrentItem(conditionValue);
    browser.editItem(conditionValue);
    drainGuiEvents();
    QLineEdit* editor = browser.findChild<QLineEdit*>();
    ASSERT_NE(editor, nullptr);
    commitTextWithEnter(editor, "Entity.Priority > 2");
    EXPECT_EQ(decide.getConditions()->getAtRank(0), "Entity.Priority > 2");
}

QDoubleSpinBox* beginDoubleEdit(ObjectPropertyBrowser& browser, const QString& propertyName) {
    QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), propertyName);
    EXPECT_NE(item, nullptr);
    if (item == nullptr) {
        return nullptr;
    }
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    return browser.findChild<QDoubleSpinBox*>();
}

QSpinBox* beginIntegerEdit(ObjectPropertyBrowser& browser, const QString& propertyName) {
    QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), propertyName);
    EXPECT_NE(item, nullptr);
    if (item == nullptr) {
        return nullptr;
    }
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    return browser.findChild<QSpinBox*>();
}

TEST(PropertyEditorModelDataReference, AIAssistantSelectsAnExistingAISupportInstance) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* sharedSupport = new AISupport(model, "SharedSupport_PropertyEditor");
    AIAssistant assistant(model, "Assistant_PropertyEditor");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &assistant);

    QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("AISupport"));
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int sharedIndex = selector->findText(QString::fromStdString(sharedSupport->getName()));
    ASSERT_GE(sharedIndex, 0);
    selector->setCurrentIndex(sharedIndex);
    drainGuiEvents();

    EXPECT_EQ(assistant.getAISupport(), sharedSupport);
    EXPECT_EQ(assistant.getAISupportName(), "SharedSupport_PropertyEditor");
    assistant.setAISupportName("LegacySupportName");
    EXPECT_EQ(assistant.getAISupport(), nullptr);
    EXPECT_EQ(assistant.getAttachedData()->find("AISupport"), assistant.getAttachedData()->end());
}

TEST(PropertyEditorModelDataReference, AIAssistantRestoresSelectedSupportBeforeModelCheck) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* support = new AISupport(model, "AISupport_ReferenceRoundTrip");
    auto* assistant = new AIAssistant(model, "AIAssistant_ReferenceRoundTrip");
    assistant->setAISupport(support);

    const QString path = QStringLiteral("/tmp/genesys_ai_support_reference_roundtrip.gen");
    QFile::remove(path);
    ASSERT_TRUE(model->save(path.toStdString()));
    Model* loaded = simulator.getModelManager()->loadModel(path.toStdString());
    ASSERT_NE(loaded, nullptr);
    auto* loadedAssistant = dynamic_cast<AIAssistant*>(
        loaded->getComponentManager()->find("AIAssistant_ReferenceRoundTrip"));
    auto* loadedSupport = dynamic_cast<AISupport*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<AISupport>(), "AISupport_ReferenceRoundTrip"));
    ASSERT_NE(loadedAssistant, nullptr);
    ASSERT_NE(loadedSupport, nullptr);
    EXPECT_EQ(loadedAssistant->getAISupport(), loadedSupport);
    EXPECT_EQ(loaded->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<AISupport>()), 1u);
    QFile::remove(path);
}

TEST(PropertyEditorModelDataReference, ContinuousSystemSelectsAnExistingODESolver) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* solver = new ODESolver(model, "SelectedSolver_PropertyEditor");
    ContinuousSystemComponent component(model, "Continuous_PropertyEditor");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);

    QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("ODESolver"));
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int solverIndex = selector->findText(QString::fromStdString(solver->getName()));
    ASSERT_GE(solverIndex, 0);
    selector->setCurrentIndex(solverIndex);
    drainGuiEvents();

    EXPECT_EQ(component.getOdeSolver(), solver);
    ASSERT_NE(component.getAttachedData(), nullptr);
    ASSERT_TRUE(component.getAttachedData()->contains("ODESolver"));
    EXPECT_EQ(component.getAttachedData()->at("ODESolver"), solver);
}

TEST(PropertyEditorModelDataReference, ContinuousSystemRestoresSelectedSolverBeforeModelCheck) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* solver = new ODESolver(model, "ODESolver_ReferenceRoundTrip");
    auto* component = new ContinuousSystemComponent(model, "ContinuousSystem_ReferenceRoundTrip");
    component->setOdeSolver(solver);

    const QString path = QStringLiteral("/tmp/genesys_ode_solver_reference_roundtrip.gen");
    QFile::remove(path);
    ASSERT_TRUE(model->save(path.toStdString()));
    Model* loaded = simulator.getModelManager()->loadModel(path.toStdString());
    ASSERT_NE(loaded, nullptr);
    auto* loadedComponent = dynamic_cast<ContinuousSystemComponent*>(
        loaded->getComponentManager()->find("ContinuousSystem_ReferenceRoundTrip"));
    auto* loadedSolver = dynamic_cast<ODESolver*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<ODESolver>(), "ODESolver_ReferenceRoundTrip"));
    ASSERT_NE(loadedComponent, nullptr);
    ASSERT_NE(loadedSolver, nullptr);
    EXPECT_EQ(loadedComponent->getOdeSolver(), loadedSolver);
    EXPECT_EQ(loaded->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<ODESolver>()), 1u);
    QFile::remove(path);
}

TEST(PropertyEditorModelDataReference, ModalModelSelectsPolymorphicNetworkByTypeAndIdentity) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* network = new EFSMNetwork(model, "SharedNetworkName_PropertyEditor");
    auto* graphNetwork = new GraphNetwork(model, "SharedNetworkName_PropertyEditor");
    graphNetwork->addInputPort("GraphInput0");
    graphNetwork->addInputPort("GraphInput1");
    graphNetwork->addOutputPort("GraphOutput0");
    graphNetwork->addOutputPort("GraphOutput1");
    graphNetwork->addOutputPort("GraphOutput2");
    std::vector<DefaultNetwork*> networks{
        network,
        graphNetwork,
        new DirectedGraphNetwork(model, "SharedNetworkName_PropertyEditor"),
        new DirectedAcyclicGraphNetwork(model, "SharedNetworkName_PropertyEditor"),
        new MarkovChainNetwork(model, "SharedNetworkName_PropertyEditor"),
        new ColoredPetriNetNetwork(model, "SharedNetworkName_PropertyEditor")
    };
    networks.back()->addInputPort("fire");
    networks.back()->addOutputPort("fired");
    ModalModelDefault component(model, "ModalAdapter_PropertyEditor");
    Plugin* adapterPlugin = simulator.getPluginManager()->find(Util::TypeOf<ModalModelDefault>());
    ASSERT_NE(adapterPlugin, nullptr);
    ModelGraphicsScene scene(0, 0, 1000, 800);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    auto* graphical = new GraphicalModelComponent(adapterPlugin, &component, QPointF(100, 100), Qt::blue);
    scene.addItem(graphical);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    browser.setActiveObject(graphical, &component, {}, {QString::fromStdString(component.getName())});
    drainGuiEvents();

    QtBrowserItem* item = nullptr;
    for (DefaultNetwork* candidate : networks) {
        const QString choice = QStringLiteral("%1::%2 [#%3]")
            .arg(QString::fromStdString(candidate->getClassname()), QString::fromStdString(candidate->getName()))
            .arg(candidate->getId());
        QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Network"));
        ASSERT_NE(item, nullptr);
        browser.setCurrentItem(item);
        browser.editItem(item);
        drainGuiEvents();
        QComboBox* selector = browser.findChild<QComboBox*>();
        ASSERT_NE(selector, nullptr);
        const int index = selector->findText(choice);
        ASSERT_GE(index, 0) << choice.toStdString();
        selector->setCurrentIndex(index);
        drainGuiEvents();
        EXPECT_EQ(component.getNetwork(), candidate) << choice.toStdString();
        EXPECT_EQ(graphical->getGraphicalInputPorts().size(), component.getDynamicInputPortCount());
        EXPECT_EQ(graphical->getGraphicalOutputPorts().size(), component.getDynamicOutputPortCount());
    }
    ASSERT_EQ(component.getNetwork(), networks.back());
    ASSERT_TRUE(undoStack.canUndo());
    undoStack.undo();
    drainGuiEvents();
    EXPECT_EQ(component.getNetwork(), networks[networks.size() - 2]);
    EXPECT_EQ(graphical->getGraphicalInputPorts().size(), component.getDynamicInputPortCount());
    EXPECT_EQ(graphical->getGraphicalOutputPorts().size(), component.getDynamicOutputPortCount());
    undoStack.redo();
    drainGuiEvents();
    EXPECT_EQ(component.getNetwork(), networks.back());
    EXPECT_EQ(graphical->getGraphicalInputPorts().size(), component.getDynamicInputPortCount());
    EXPECT_EQ(graphical->getGraphicalOutputPorts().size(), component.getDynamicOutputPortCount());

    const QString graphType = QString::fromStdString(Util::TypeOf<DirectedGraphNetwork>());
    const QString createLabel = QStringLiteral("Criar novo %1").arg(graphType);
    item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Network"));
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    auto* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    EXPECT_EQ(selector->findText(QStringLiteral("Criar novo DefaultNetwork")), -1);
    const int createIndex = selector->findText(createLabel);
    ASSERT_GE(createIndex, 0);
    selector->setCurrentIndex(createIndex);
    drainGuiEvents();
    auto* createdNetwork = dynamic_cast<DirectedGraphNetwork*>(component.getNetwork());
    ASSERT_NE(createdNetwork, nullptr);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(graphType.toStdString(), createdNetwork->getId()), createdNetwork);
    EXPECT_EQ(graphical->getGraphicalInputPorts().size(), component.getDynamicInputPortCount());
    EXPECT_EQ(graphical->getGraphicalOutputPorts().size(), component.getDynamicOutputPortCount());

    item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Network"));
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int removeIndex = selector->findText(QStringLiteral("Remover referência"));
    ASSERT_GE(removeIndex, 0);
    selector->setCurrentIndex(removeIndex);
    drainGuiEvents();
    EXPECT_EQ(component.getNetwork(), nullptr);
    EXPECT_EQ(graphical->getGraphicalInputPorts().size(), component.getDynamicInputPortCount());
    EXPECT_EQ(graphical->getGraphicalOutputPorts().size(), component.getDynamicOutputPortCount());
    EXPECT_EQ(model->getDataManager()->getDataDefinition(graphType.toStdString(), createdNetwork->getId()), createdNetwork);

    item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Network"));
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const QString createdChoice = QStringLiteral("%1::%2 [#%3]")
        .arg(graphType, QString::fromStdString(createdNetwork->getName()))
        .arg(createdNetwork->getId());
    const int reattachIndex = selector->findText(createdChoice);
    ASSERT_GE(reattachIndex, 0);
    selector->setCurrentIndex(reattachIndex);
    drainGuiEvents();
    EXPECT_EQ(component.getNetwork(), createdNetwork);
    EXPECT_EQ(graphical->getGraphicalInputPorts().size(), component.getDynamicInputPortCount());
    EXPECT_EQ(graphical->getGraphicalOutputPorts().size(), component.getDynamicOutputPortCount());

    const DefaultNetwork* bindingNetwork = networks.back();
    const QString bindingChoice = QStringLiteral("%1::%2 [#%3]")
        .arg(QString::fromStdString(bindingNetwork->getClassname()), QString::fromStdString(bindingNetwork->getName()))
        .arg(bindingNetwork->getId());
    item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Network"));
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int bindingNetworkIndex = selector->findText(bindingChoice);
    ASSERT_GE(bindingNetworkIndex, 0);
    selector->setCurrentIndex(bindingNetworkIndex);
    drainGuiEvents();
    EXPECT_EQ(component.getNetwork(), bindingNetwork);
    ASSERT_EQ(component.getNetworkBindingControls().size(), 2u);

    item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Network Details"));
    ASSERT_NE(item, nullptr);
    browser.setExpanded(item, true);
    drainGuiEvents();
    QtBrowserItem* inputBinding = findBrowserItemByName(item->children(), QStringLiteral("Input Binding 0 (fire)"));
    QtBrowserItem* outputBinding = findBrowserItemByName(item->children(), QStringLiteral("Output Binding 0 (fired)"));
    ASSERT_NE(inputBinding, nullptr);
    ASSERT_NE(outputBinding, nullptr);
    const std::string previousInputBinding = component.getInputBinding(0);
    const std::string previousOutputBinding = component.getOutputBinding(0);
    browser.setCurrentItem(inputBinding);
    browser.editItem(inputBinding);
    drainGuiEvents();
    QLineEdit* bindingEditor = browser.findChild<QLineEdit*>();
    ASSERT_NE(bindingEditor, nullptr);
    commitTextWithEnter(bindingEditor, QStringLiteral("2"));
    EXPECT_EQ(component.getInputBinding(0), "2");

    item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Network Details"));
    ASSERT_NE(item, nullptr);
    outputBinding = findBrowserItemByName(item->children(), QStringLiteral("Output Binding 0 (fired)"));
    ASSERT_NE(outputBinding, nullptr);
    browser.setCurrentItem(outputBinding);
    browser.editItem(outputBinding);
    drainGuiEvents();
    bindingEditor = browser.findChild<QLineEdit*>();
    ASSERT_NE(bindingEditor, nullptr);
    commitTextWithEnter(bindingEditor, QStringLiteral("firedValue"));
    EXPECT_EQ(component.getOutputBinding(0), "firedValue");
    ASSERT_TRUE(undoStack.canUndo());
    undoStack.undo();
    EXPECT_EQ(component.getOutputBinding(0), previousOutputBinding);
    undoStack.undo();
    EXPECT_EQ(component.getInputBinding(0), previousInputBinding);
    undoStack.redo();
    EXPECT_EQ(component.getInputBinding(0), "2");
    undoStack.redo();
    EXPECT_EQ(component.getOutputBinding(0), "firedValue");
}

TEST(PropertyEditorModelDataReference, DummyComponentExposesItsOwnedDataDefinitionForEditing) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DummyComponent component(model, "Dummy_PropertyEditor");
    ModelDataDefinition::CreateInternalData(&component);
    ASSERT_NE(component.getInternalDataDefinition(), nullptr);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(
                  Util::TypeOf<DummyElement>(), component.getInternalDataDefinition()->getName()),
              component.getInternalDataDefinition());

    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);

    QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("DummyElement"));
    ASSERT_NE(item, nullptr);
    browser.setExpanded(item, true);
    drainGuiEvents();
    EXPECT_NE(findBrowserItemByName(browser.topLevelItems(), QStringLiteral("SomeString")), nullptr);
}

TEST(PropertyEditorModelDataReference, OwnedRuntimeDefinitionsReattachOnGenLoadWithoutDuplicates) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* dummy = new DummyComponent(model, "Dummy_InternalRoundTrip");
    auto* python = new PythonForG(model, "Python_InternalRoundTrip");
    python->getPythonRuntime()->setPythonExecutable("custom-python");
    auto* r = new RSimulator(model, "R_InternalRoundTrip");
    r->setRExecutable("custom-Rscript");
    r->setPreludeScript("# prelude\nvalue <- 7\n");
    r->setCommandList("result <- value + 1\n");

    const QString path = QStringLiteral("/tmp/genesys_owned_runtime_roundtrip.gen");
    QFile::remove(path);
    ASSERT_TRUE(model->save(path.toStdString()));
    Model* loaded = simulator.getModelManager()->loadModel(path.toStdString());
    ASSERT_NE(loaded, nullptr);

    auto* loadedDummy = dynamic_cast<DummyComponent*>(loaded->getComponentManager()->find("Dummy_InternalRoundTrip"));
    auto* loadedPython = dynamic_cast<PythonForG*>(loaded->getComponentManager()->find("Python_InternalRoundTrip"));
    auto* loadedR = dynamic_cast<RSimulator*>(loaded->getComponentManager()->find("R_InternalRoundTrip"));
    ASSERT_NE(loadedDummy, nullptr);
    ASSERT_NE(loadedPython, nullptr);
    ASSERT_NE(loadedR, nullptr);
    auto* dummyData = dynamic_cast<DummyElement*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<DummyElement>(), "Dummy_InternalRoundTrip.DummyElement"));
    auto* pythonRuntime = dynamic_cast<PythonRuntime*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<PythonRuntime>(), "Python_InternalRoundTrip.PythonRuntime"));
    auto* rRunner = dynamic_cast<RSimulatorRunner*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<RSimulatorRunner>(), "R_InternalRoundTrip.RSimulatorRunner"));
    ASSERT_NE(dummyData, nullptr);
    ASSERT_NE(pythonRuntime, nullptr);
    ASSERT_NE(rRunner, nullptr);
    EXPECT_EQ(loadedDummy->getInternalDataDefinition(), dummyData);
    EXPECT_EQ(loadedPython->getPythonRuntime(), pythonRuntime);
    EXPECT_EQ(pythonRuntime->getPythonExecutable(), "custom-python");
    EXPECT_EQ(loadedR->getRunner(), rRunner);
    EXPECT_EQ(rRunner->getRExecutable(), "custom-Rscript");
    EXPECT_EQ(rRunner->getPreludeScript(), "# prelude\nvalue <- 7\n");
    EXPECT_EQ(loadedR->getCommandList(), "result <- value + 1");
    EXPECT_EQ(loaded->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<DummyElement>()), 1u);
    EXPECT_EQ(loaded->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<PythonRuntime>()), 1u);
    EXPECT_EQ(loaded->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<RSimulatorRunner>()), 1u);
    QFile::remove(path);
}

TEST(PropertyEditorModelDataReference, PythonForGExposesItsOwnedRuntimeConfiguration) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    PythonForG component(model, "PythonForG_PropertyEditor");
    ASSERT_NE(component.getPythonRuntime(), nullptr);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);

    QtBrowserItem* runtimeItem = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("PythonRuntime"));
    ASSERT_NE(runtimeItem, nullptr);
    browser.setExpanded(runtimeItem, true);
    drainGuiEvents();
    QtBrowserItem* executableItem = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("PythonExecutable"));
    ASSERT_NE(executableItem, nullptr);
    browser.setCurrentItem(executableItem);
    browser.editItem(executableItem);
    drainGuiEvents();
    QLineEdit* editor = browser.findChild<QLineEdit*>();
    ASSERT_NE(editor, nullptr);
    commitTextWithEnter(editor, QStringLiteral("python3-property-editor"));
    EXPECT_EQ(component.getPythonRuntime()->getPythonExecutable(), "python3-property-editor");
}

TEST(PropertyEditorModelDataReference, RSimulatorComponentConfigurationIsTheRunnerSourceOfTruth) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    RSimulator component(model, "RSimulator_PropertyEditor");
    component.setRExecutable("Rscript-custom");
    component.setWorkingDirectory("/tmp/r-simulator-property-editor");
    component.setPreludeScript("# multiline prelude\noptions(stringsAsFactors = FALSE)");
    ASSERT_NE(component.getRunner(), nullptr);
    EXPECT_EQ(component.getRunner()->getRExecutable(), "Rscript-custom");
    EXPECT_EQ(component.getRunner()->getWorkingDirectory(), "/tmp/r-simulator-property-editor");
    EXPECT_EQ(component.getRunner()->getPreludeScript(), "# multiline prelude\noptions(stringsAsFactors = FALSE)");
}

TEST(PropertyEditorModelDataReference, SearchSelectsEntityGroupWithoutChangingSearchMode) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* group = new EntityGroup(model, "SearchGroup_PropertyEditor");
    Search component(model, "Search_PropertyEditor");
    component.setSearchInType(Search::SearchInType::ENTITYGROUP);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);

    EXPECT_EQ(findBrowserItemByName(browser.topLevelItems(), QStringLiteral("SearchInName")), nullptr);
    QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("SearchIn"));
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    EXPECT_GE(selector->findText(QString("Criar novo %1").arg(QString::fromStdString(Util::TypeOf<EntityGroup>()))), 0);
    const int groupIndex = selector->findText(QString::fromStdString(group->getName()));
    ASSERT_GE(groupIndex, 0);
    selector->setCurrentIndex(groupIndex);
    drainGuiEvents();

    EXPECT_EQ(component.getSearchIn(), group);
    EXPECT_EQ(component.getSearchInType(), Search::SearchInType::ENTITYGROUP);
}

TEST(PropertyEditorModelDataReference, RemoveSelectsEntityGroupWithoutChangingRemoveMode) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* group = new EntityGroup(model, "RemoveGroup_PropertyEditor");
    Remove component(model, "Remove_PropertyEditor");
    component.setRemoveFromType(Remove::RemoveFromType::ENTITYGROUP);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);

    QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("RemoveFrom"));
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    browser.editItem(item);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    EXPECT_GE(selector->findText(QString("Criar novo %1").arg(QString::fromStdString(Util::TypeOf<EntityGroup>()))), 0);
    const int groupIndex = selector->findText(QString::fromStdString(group->getName()));
    ASSERT_GE(groupIndex, 0);
    selector->setCurrentIndex(groupIndex);
    drainGuiEvents();

    EXPECT_EQ(component.getRemoveFrom(), group);
    EXPECT_EQ(component.getRemoveFromType(), Remove::RemoveFromType::ENTITYGROUP);
}

TEST(PropertyEditorModelDataReference, SearchAndRemoveRestoreSharedEntityGroupReference) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* group = new EntityGroup(model, "SharedGroup_RoundTrip");
    auto* search = new Search(model, "Search_RoundTrip");
    auto* remove = new Remove(model, "Remove_RoundTrip");
    search->setSearchInType(Search::SearchInType::ENTITYGROUP);
    search->setSearchIn(group);
    remove->setRemoveFromType(Remove::RemoveFromType::ENTITYGROUP);
    remove->setRemoveFrom(group);

    const QString path = QStringLiteral("/tmp/genesys_search_remove_reference_roundtrip.gen");
    QFile::remove(path);
    ASSERT_TRUE(model->save(path.toStdString()));
    Model* loaded = simulator.getModelManager()->loadModel(path.toStdString());
    ASSERT_NE(loaded, nullptr);
    auto* loadedGroup = dynamic_cast<EntityGroup*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<EntityGroup>(), "SharedGroup_RoundTrip"));
    auto* loadedSearch = dynamic_cast<Search*>(loaded->getComponentManager()->find("Search_RoundTrip"));
    auto* loadedRemove = dynamic_cast<Remove*>(loaded->getComponentManager()->find("Remove_RoundTrip"));
    ASSERT_NE(loadedGroup, nullptr);
    ASSERT_NE(loadedSearch, nullptr);
    ASSERT_NE(loadedRemove, nullptr);
    EXPECT_EQ(loadedSearch->getSearchIn(), loadedGroup);
    EXPECT_EQ(loadedRemove->getRemoveFrom(), loadedGroup);
    EXPECT_EQ(loadedSearch->getSearchInType(), Search::SearchInType::ENTITYGROUP);
    EXPECT_EQ(loadedRemove->getRemoveFromType(), Remove::RemoveFromType::ENTITYGROUP);
    QFile::remove(path);
}

TEST(PropertyEditorModelDataReference, MatchExposesCommonQueueOrderingProperties) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Match component(model, "Match_PropertyEditor");
    ModelDataDefinition::CreateInternalData(&component);
    auto* q0 = dynamic_cast<Queue*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "Match_PropertyEditor.Queue0"));
    auto* q1 = dynamic_cast<Queue*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "Match_PropertyEditor.Queue1"));
    ASSERT_NE(q0, nullptr);
    ASSERT_NE(q1, nullptr);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);
    drainGuiEvents();

    QtBrowserItem* ruleItem = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("QueueOrderRule"));
    ASSERT_NE(ruleItem, nullptr);
    browser.setCurrentItem(ruleItem);
    browser.editItem(ruleItem);
    drainGuiEvents();
    QComboBox* selector = browser.findChild<QComboBox*>();
    ASSERT_NE(selector, nullptr);
    const int ruleIndex = selector->findText(QStringLiteral("HIGHESTVALUE"));
    ASSERT_GE(ruleIndex, 0);
    selector->setCurrentIndex(ruleIndex);
    drainGuiEvents();

    QtBrowserItem* attributeItem = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("QueueOrderAttributeName"));
    ASSERT_NE(attributeItem, nullptr);
    browser.setCurrentItem(attributeItem);
    browser.editItem(attributeItem);
    drainGuiEvents();
    QLineEdit* editor = browser.findChild<QLineEdit*>();
    ASSERT_NE(editor, nullptr);
    commitTextWithEnter(editor, QStringLiteral("Priority"));
    EXPECT_EQ(component.getQueueOrderRule(), Queue::OrderRule::HIGHESTVALUE);
    EXPECT_EQ(component.getQueueOrderAttributeName(), "Priority");
    EXPECT_EQ(q0->getOrderRule(), Queue::OrderRule::HIGHESTVALUE);
    EXPECT_EQ(q1->getOrderRule(), Queue::OrderRule::HIGHESTVALUE);
    EXPECT_EQ(q0->getAttributeName(), "Priority");
    EXPECT_EQ(q1->getAttributeName(), "Priority");
}

TEST(PropertyEditorModelDataReference, CppForGExposesOwnedCompilerSettingsInline) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    CppForG component(model, "CppForG_PropertyEditor");
    CppCompiler* ownedCompiler = component.getCppCompiler();
    ASSERT_NE(ownedCompiler, nullptr);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);
    drainGuiEvents();

    EXPECT_NE(findBrowserItemByName(browser.topLevelItems(), QStringLiteral("CppCompiler")), nullptr);
    QtBrowserItem* sourceFileItem = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("SourceFilename"));
    ASSERT_NE(sourceFileItem, nullptr);
    browser.setCurrentItem(sourceFileItem);
    browser.editItem(sourceFileItem);
    drainGuiEvents();
    QLineEdit* editor = browser.findChild<QLineEdit*>();
    ASSERT_NE(editor, nullptr);
    commitTextWithEnter(editor, QStringLiteral("cppforg-edited.cpp"));
    EXPECT_EQ(ownedCompiler->getSourceFilename(), "cppforg-edited.cpp");
    EXPECT_EQ(component.getCppCompiler(), ownedCompiler);
    EXPECT_THROW(component.setCppCompiler(nullptr), std::invalid_argument);
}

TEST(PropertyEditorModelDataReference, SPICECircuitRunnerCallbackExpiresWithCircuit) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* runner = new SPICERunner(model, "SPICERunner_CallbackLifetime");
    {
        SPICECircuit circuit(model, "SPICECircuit_CallbackLifetime");
        circuit.setRunner(runner);
    }
    EXPECT_NO_THROW(runner->CompileSpiceFile());
}

TEST(PropertyEditorModelDataReference, SPICECircuitPreservesSharedRunnerOnGenRoundTrip) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* runner = new SPICERunner(model, "SPICERunner_RoundTrip");
    auto* circuit = new SPICECircuit(model, "SPICECircuit_RoundTrip");
    circuit->setRunner(runner);

    const QString path = QStringLiteral("/tmp/genesys_spice_runner_reference_roundtrip.gen");
    QFile::remove(path);
    ASSERT_TRUE(model->save(path.toStdString()));
    Model* loaded = simulator.getModelManager()->loadModel(path.toStdString());
    ASSERT_NE(loaded, nullptr);
    auto* loadedCircuit = dynamic_cast<SPICECircuit*>(loaded->getComponentManager()->find("SPICECircuit_RoundTrip"));
    auto* loadedRunner = dynamic_cast<SPICERunner*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<SPICERunner>(), "SPICERunner_RoundTrip"));
    ASSERT_NE(loadedCircuit, nullptr);
    ASSERT_NE(loadedRunner, nullptr);
    EXPECT_EQ(loadedCircuit->getRunner(), loadedRunner);
    QFile::remove(path);
}

TEST(PropertyEditorModelDataReference, DiffEquationsExposesOwnedCompilerSettingsInline) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DiffEquations component(model, "DiffEquations_PropertyEditor");
    CppCompiler* ownedCompiler = component.getCppCompiler();
    ASSERT_NE(ownedCompiler, nullptr);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, &component);
    drainGuiEvents();

    EXPECT_NE(findBrowserItemByName(browser.topLevelItems(), QStringLiteral("CppCompiler")), nullptr);
    QtBrowserItem* sourceFileItem = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("SourceFilename"));
    ASSERT_NE(sourceFileItem, nullptr);
    browser.setCurrentItem(sourceFileItem);
    browser.editItem(sourceFileItem);
    drainGuiEvents();
    QLineEdit* editor = browser.findChild<QLineEdit*>();
    ASSERT_NE(editor, nullptr);
    commitTextWithEnter(editor, QStringLiteral("diff-equations-edited.cpp"));
    EXPECT_EQ(ownedCompiler->getSourceFilename(), "diff-equations-edited.cpp");
    EXPECT_EQ(component.getCppCompiler(), ownedCompiler);
}

TEST(PropertyEditorModelDataReference, OwnedCompilerSettingsSurviveGenRoundTrip) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* cppForG = new CppForG(model, "CppForG_CompilerRoundTrip");
    auto* diffEquations = new DiffEquations(model, "DiffEquations_CompilerRoundTrip");
    cppForG->getCppCompiler()->setCompilerCommand("custom-cxx");
    diffEquations->getCppCompiler()->setOutputFilename("custom-output.so");
    cppForG->setOnDispatchEventCode("// editable incomplete source\n");

    const QString path = QStringLiteral("/tmp/genesys_internal_compiler_roundtrip.gen");
    QFile::remove(path);
    ASSERT_TRUE(model->save(path.toStdString()));
    Model* loaded = simulator.getModelManager()->loadModel(path.toStdString());
    ASSERT_NE(loaded, nullptr);
    auto* loadedCppForG = dynamic_cast<CppForG*>(loaded->getComponentManager()->find("CppForG_CompilerRoundTrip"));
    auto* loadedDiffEquations = dynamic_cast<DiffEquations*>(loaded->getComponentManager()->find("DiffEquations_CompilerRoundTrip"));
    ASSERT_NE(loadedCppForG, nullptr);
    ASSERT_NE(loadedDiffEquations, nullptr);
    ASSERT_NE(loadedCppForG->getCppCompiler(), nullptr);
    ASSERT_NE(loadedDiffEquations->getCppCompiler(), nullptr);
    EXPECT_EQ(loaded->getDataManager()->getDataDefinition(Util::TypeOf<CppCompiler>(),
                                                          "CppForG_CompilerRoundTrip.CppCompiler"),
              loadedCppForG->getCppCompiler());
    EXPECT_EQ(loaded->getDataManager()->getDataDefinition(Util::TypeOf<CppCompiler>(),
                                                          "DiffEquations_CompilerRoundTrip.CppCompiler"),
              loadedDiffEquations->getCppCompiler());
    auto* loadedCompilers = loaded->getDataManager()->getDataDefinitionList(Util::TypeOf<CppCompiler>());
    ASSERT_NE(loadedCompilers, nullptr);
    EXPECT_EQ(loadedCompilers->size(), 2u);
    EXPECT_EQ(loadedCppForG->getCppCompiler()->getCompilerCommand(), "custom-cxx");
    EXPECT_EQ(loadedDiffEquations->getCppCompiler()->getOutputFilename(), "custom-output.so");
    EXPECT_EQ(loadedCppForG->getOnDispatchEventCode(), "// editable incomplete source\n");
    QFile::remove(path);
}

TEST(PropertyEditorDoubleCommit, GenericSimulationControlDoubleCommitsLocalizedValueOnce) {
    ScopedDefaultLocale locale(QLocale(QLocale::Portuguese, QLocale::Brazil));

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* object = new GenericDoubleDataDefinition(model);
    ASSERT_NE(object, nullptr);

    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();

    int modelChangedCount = 0;
    bindEditableKernelObject(browser, object, &modelChangedCount);

    QDoubleSpinBox* spinBox = beginDoubleEdit(browser, QStringLiteral("Value"));
    ASSERT_NE(spinBox, nullptr);
    const int countBefore = modelChangedCount;
    commitTextWithEnter(spinBox->findChild<QLineEdit*>(), QStringLiteral("1,25"));

    EXPECT_NEAR(object->value(), 1.25, 1e-12);
    EXPECT_EQ(modelChangedCount, countBefore + 1);
    EXPECT_EQ(object->valueSetCount(), 1);
    drainGuiEvents();
    EXPECT_NEAR(object->value(), 1.25, 1e-12);
}

TEST(PropertyEditorModelDataReference, WaitAndSignalSelectTheSameSignalData) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* sharedSignalData = new SignalData(model, "SharedSignalData_PropertyEditor");
    ASSERT_NE(sharedSignalData, nullptr);
    Wait wait(model, "Wait_PropertyEditor");
    Signal signal(model, "Signal_PropertyEditor");

    // Construction and association materialization must not manufacture either reference.
    ASSERT_EQ(wait.getQueue(), nullptr);
    ASSERT_EQ(wait.getSignalData(), nullptr);
    ASSERT_EQ(signal.getSignalData(), nullptr);
    ModelDataDefinition::CreateInternalData(&wait);
    ModelDataDefinition::CreateInternalData(&signal);
    EXPECT_EQ(wait.getQueue(), nullptr);
    EXPECT_EQ(wait.getSignalData(), nullptr);
    EXPECT_EQ(signal.getSignalData(), nullptr);
    auto* queue = new Queue(model, "Queue_PropertyEditor");
    model->getDataManager()->insert(queue);
    wait.setQueue(queue);

    auto selectSharedSignalData = [sharedSignalData](ModelComponent* component) {
        ASSERT_NE(component, nullptr);
        ObjectPropertyBrowser browser;
        browser.resize(640, 480);
        browser.show();
        bindEditableKernelObject(browser, component);

        QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("SignalData"));
        ASSERT_NE(item, nullptr);
        browser.setCurrentItem(item);
        browser.editItem(item);
        drainGuiEvents();

        QComboBox* selector = browser.findChild<QComboBox*>();
        ASSERT_NE(selector, nullptr);
        const int sharedIndex = selector->findText(QString::fromStdString(sharedSignalData->getName()));
        ASSERT_GE(sharedIndex, 0);
        EXPECT_EQ(selector->findText(QStringLiteral("Selecionar SignalData e Editar")), -1);
        EXPECT_EQ(selector->findText(QStringLiteral("Remover SignalData")), -1);
        EXPECT_GE(selector->findText(QStringLiteral("Criar novo SignalData")), 0);
        selector->setCurrentIndex(sharedIndex);
        drainGuiEvents();

        SimulationControl* signalDataControl = nullptr;
        for (SimulationControl* control : *component->getSimulationControls()->list()) {
            if (control != nullptr && control->getName() == "SignalData") {
                signalDataControl = control;
                break;
            }
        }
        ASSERT_NE(signalDataControl, nullptr);
        EXPECT_EQ(signalDataControl->getReferencedModelDataDefinition(), sharedSignalData);
    };

    selectSharedSignalData(&wait);
    selectSharedSignalData(&signal);
}

TEST(PropertyEditorModelDataReference, WaitQueueSelectorRetainsAllChoicesAndSwitchesByIdentity) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* queueA = new Queue(model, "WaitQueueA");
    auto* queueB = new Queue(model, "WaitQueueB");
    model->getDataManager()->insert(queueA);
    model->getDataManager()->insert(queueB);
    {
        Wait wait(model, "WaitQueueSelector");
        wait.setQueue(queueA);
        ModelDataDefinition::CreateInternalData(&wait);
        ObjectPropertyBrowser browser;
        browser.resize(640, 480);
        browser.show();
        bindEditableKernelObject(browser, &wait);
        QtBrowserItem* item = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Queue"));
        ASSERT_NE(item, nullptr);
        browser.setCurrentItem(item);
        browser.editItem(item);
        drainGuiEvents();
        QComboBox* selector = browser.findChild<QComboBox*>();
        ASSERT_NE(selector, nullptr);
        const int aIndex = selector->findText(QStringLiteral("WaitQueueA"));
        const int bIndex = selector->findText(QStringLiteral("WaitQueueB"));
        ASSERT_GE(aIndex, 0);
        ASSERT_GE(bIndex, 0);
        EXPECT_GE(selector->findText(QStringLiteral("Criar novo Queue")), 0);
        EXPECT_EQ(selector->findText(QStringLiteral("Selecionar Queue e Editar")), -1);
        EXPECT_EQ(selector->findText(QStringLiteral("Remover Queue")), -1);
        selector->setCurrentIndex(bIndex);
        drainGuiEvents();
        EXPECT_EQ(wait.getQueue(), queueB);
        EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "WaitQueueA"), queueA);
        EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "WaitQueueB"), queueB);
    }
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "WaitQueueA"), queueA);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "WaitQueueB"), queueB);
    Wait createWait(model, "WaitQueueCreate");
    ObjectPropertyBrowser createBrowser;
    createBrowser.resize(640, 480);
    createBrowser.show();
    bindEditableKernelObject(createBrowser, &createWait);
    QtBrowserItem* createItem = findBrowserItemByName(createBrowser.topLevelItems(), QStringLiteral("Queue"));
    ASSERT_NE(createItem, nullptr);
    createBrowser.setCurrentItem(createItem);
    createBrowser.editItem(createItem);
    drainGuiEvents();
    QComboBox* createSelector = createBrowser.findChild<QComboBox*>();
    ASSERT_NE(createSelector, nullptr);
    const int createIndex = createSelector->findText(QStringLiteral("Criar novo Queue"));
    ASSERT_GE(createIndex, 0);
    const std::size_t queueCountBefore = model->getDataManager()->getDataDefinitionList(Util::TypeOf<Queue>())->size();
    createSelector->setCurrentIndex(createIndex);
    drainGuiEvents();
    EXPECT_NE(createWait.getQueue(), nullptr);
    EXPECT_EQ(model->getDataManager()->getDataDefinitionList(Util::TypeOf<Queue>())->size(), queueCountBefore + 1);
}

TEST(PropertyEditorModelDataReference, SignalExplicitlyCreatesAndWaitReusesOneSignalData) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Signal signal(model, "SignalCreateSignalData");
    Wait wait(model, "WaitReuseSignalData");
    ModelDataDefinition::CreateInternalData(&signal);
    ModelDataDefinition::CreateInternalData(&wait);
    ASSERT_EQ(signal.getSignalData(), nullptr);
    ASSERT_EQ(wait.getSignalData(), nullptr);
    auto* dataList = model->getDataManager()->getDataDefinitionList(Util::TypeOf<SignalData>());
    ASSERT_NE(dataList, nullptr);
    ASSERT_EQ(dataList->size(), 0u);

    ObjectPropertyBrowser signalBrowser;
    signalBrowser.resize(640, 480);
    signalBrowser.show();
    bindEditableKernelObject(signalBrowser, &signal);
    QtBrowserItem* signalItem = findBrowserItemByName(signalBrowser.topLevelItems(), QStringLiteral("SignalData"));
    ASSERT_NE(signalItem, nullptr);
    signalBrowser.setCurrentItem(signalItem);
    signalBrowser.editItem(signalItem);
    drainGuiEvents();
    QComboBox* signalSelector = signalBrowser.findChild<QComboBox*>();
    ASSERT_NE(signalSelector, nullptr);
    const int createIndex = signalSelector->findText(QStringLiteral("Criar novo SignalData"));
    ASSERT_GE(createIndex, 0);
    signalSelector->setCurrentIndex(createIndex);
    drainGuiEvents();
    ASSERT_NE(signal.getSignalData(), nullptr);
    ASSERT_EQ(dataList->size(), 1u);

    ObjectPropertyBrowser waitBrowser;
    waitBrowser.resize(640, 480);
    waitBrowser.show();
    bindEditableKernelObject(waitBrowser, &wait);
    QtBrowserItem* waitItem = findBrowserItemByName(waitBrowser.topLevelItems(), QStringLiteral("SignalData"));
    ASSERT_NE(waitItem, nullptr);
    waitBrowser.setCurrentItem(waitItem);
    waitBrowser.editItem(waitItem);
    drainGuiEvents();
    QComboBox* waitSelector = waitBrowser.findChild<QComboBox*>();
    ASSERT_NE(waitSelector, nullptr);
    const int sharedIndex = waitSelector->findText(QString::fromStdString(signal.getSignalData()->getName()));
    ASSERT_GE(sharedIndex, 0);
    waitSelector->setCurrentIndex(sharedIndex);
    drainGuiEvents();
    EXPECT_EQ(wait.getSignalData(), signal.getSignalData());
    EXPECT_EQ(dataList->size(), 1u);
}

TEST(PropertyEditorDoubleCommit, BacteriaSignalGridSetterAndSimulationControlPreserveDouble) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* grid =
        manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_KernelContract");
    ASSERT_NE(grid, nullptr);

    grid->setDiffusionRate(0.37);
    EXPECT_DOUBLE_EQ(grid->getDiffusionRate(), 0.37);

    SimulationControl* diffusionControl = nullptr;
    for (SimulationControl* control : *grid->getSimulationControls()->list()) {
        if (control != nullptr && control->getName() == "DiffusionRate") {
            diffusionControl = control;
            break;
        }
    }
    ASSERT_NE(diffusionControl, nullptr);
    diffusionControl->setValue("0.18");
    EXPECT_DOUBLE_EQ(grid->getDiffusionRate(), 0.18);
}

TEST(PropertyEditorDoubleCommit, PortugueseLocaleCommitsAllBiochemicalDoubleControlsOnce) {
    ScopedDefaultLocale locale(QLocale(QLocale::Portuguese, QLocale::Brazil));

    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BioNetwork* network = manager->newInstance<BioNetwork>(model, "BioNetwork_PropertyEditor");
    ASSERT_NE(network, nullptr);
    BacteriaSignalGrid* grid =
        manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_PropertyEditor");
    ASSERT_NE(grid, nullptr);

    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();

    int modelChangedCount = 0;
    bindEditableKernelObject(browser, network, &modelChangedCount);

    auto commitNetworkDouble = [&](const QString& propertyName,
                                   const QString& localizedText,
                                   const std::function<double()>& getter) -> double {
        const int countBefore = modelChangedCount;
        QDoubleSpinBox* spinBox = beginDoubleEdit(browser, propertyName);
        if (spinBox == nullptr) {
            ADD_FAILURE() << "Expected QDoubleSpinBox for " << propertyName.toStdString();
            return getter();
        }
        commitTextWithEnter(spinBox->findChild<QLineEdit*>(), localizedText);
        EXPECT_EQ(modelChangedCount, countBefore + 1);
        drainGuiEvents();
        return getter();
    };

    EXPECT_NEAR(commitNetworkDouble("StartTime", "0,18", [&]() { return network->getStartTime(); }),
                0.18, 1e-12);
    EXPECT_NEAR(commitNetworkDouble("StopTime", "1,25", [&]() { return network->getStopTime(); }),
                1.25, 1e-12);
    EXPECT_NEAR(commitNetworkDouble("StepSize", "0,1", [&]() { return network->getStepSize(); }),
                0.1, 1e-12);

    bindEditableKernelObject(browser, grid, &modelChangedCount);
    auto commitGridDouble = [&](const QString& propertyName,
                                const QString& localizedText,
                                const std::function<double()>& getter) -> double {
        const int countBefore = modelChangedCount;
        QDoubleSpinBox* spinBox = beginDoubleEdit(browser, propertyName);
        if (spinBox == nullptr) {
            ADD_FAILURE() << "Expected QDoubleSpinBox for " << propertyName.toStdString();
            return getter();
        }
        commitTextWithEnter(spinBox->findChild<QLineEdit*>(), localizedText);
        EXPECT_EQ(modelChangedCount, countBefore + 1);
        drainGuiEvents();
        return getter();
    };

    EXPECT_NEAR(commitGridDouble("InitialSignal", "-0,5", [&]() { return grid->getInitialSignal(); }),
                -0.5, 1e-12);
    EXPECT_NEAR(commitGridDouble("DiffusionRate", "1,0", [&]() { return grid->getDiffusionRate(); }),
                1.0, 1e-12);
    EXPECT_NEAR(commitGridDouble("DecayRate", "0,25", [&]() { return grid->getDecayRate(); }),
                0.25, 1e-12);
}

TEST(PropertyEditorDoubleCommit, BacteriaSignalGridDiffusionRateCommitsOnFocusOut) {
    ScopedDefaultLocale locale(QLocale::c());

    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* grid =
        manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_FocusOut");
    ASSERT_NE(grid, nullptr);
    grid->setDiffusionRate(0.0);

    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    int modelChangedCount = 0;
    bindEditableKernelObject(browser, grid, &modelChangedCount);

    QDoubleSpinBox* diffusionEditor = beginDoubleEdit(browser, QStringLiteral("DiffusionRate"));
    ASSERT_NE(diffusionEditor, nullptr);
    EXPECT_EQ(diffusionEditor->property("value").metaType().id(), QMetaType::Double);
    int valueChangedCount = 0;
    int editingFinishedCount = 0;
    QObject::connect(diffusionEditor, &QDoubleSpinBox::valueChanged,
                     [&valueChangedCount](double) { ++valueChangedCount; });
    QObject::connect(diffusionEditor, &QAbstractSpinBox::editingFinished,
                     [&editingFinishedCount]() { ++editingFinishedCount; });
    const int modelChangedBefore = modelChangedCount;
    QLineEdit* diffusionLineEdit = diffusionEditor->findChild<QLineEdit*>();
    ASSERT_NE(diffusionLineEdit, nullptr);
    diffusionLineEdit->setFocus();
    diffusionLineEdit->selectAll();
    diffusionLineEdit->setText(QStringLiteral("0.37"));

    QFocusEvent focusOut(QEvent::FocusOut);
    QApplication::sendEvent(diffusionLineEdit, &focusOut);
    browser.clear();
    drainGuiEvents();

    EXPECT_GE(valueChangedCount, 1);
    EXPECT_GE(editingFinishedCount, 1);
    EXPECT_EQ(modelChangedCount, modelChangedBefore + 1);
    EXPECT_DOUBLE_EQ(grid->getDiffusionRate(), 0.37);
}

TEST(PropertyEditorDoubleCommit, BacteriaSignalGridSpinButtonsAndEscapeUseKernelValue) {
    ScopedDefaultLocale locale(QLocale::c());

    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    BacteriaSignalGrid* grid =
        manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_SpinButtons");
    ASSERT_NE(grid, nullptr);
    grid->setDiffusionRate(0.0);

    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, grid);

    QDoubleSpinBox* spinBox = beginDoubleEdit(browser, QStringLiteral("DiffusionRate"));
    ASSERT_NE(spinBox, nullptr);
    spinBox->stepUp();
    QLineEdit* lineEdit = spinBox->findChild<QLineEdit*>();
    ASSERT_NE(lineEdit, nullptr);
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(lineEdit, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(lineEdit, &release);
    drainGuiEvents();
    EXPECT_DOUBLE_EQ(grid->getDiffusionRate(), 1.0);

    spinBox = beginDoubleEdit(browser, QStringLiteral("DiffusionRate"));
    ASSERT_NE(spinBox, nullptr);
    spinBox->stepDown();
    lineEdit = spinBox->findChild<QLineEdit*>();
    ASSERT_NE(lineEdit, nullptr);
    QApplication::sendEvent(lineEdit, &press);
    QApplication::sendEvent(lineEdit, &release);
    drainGuiEvents();
    EXPECT_DOUBLE_EQ(grid->getDiffusionRate(), 0.0);

    spinBox = beginDoubleEdit(browser, QStringLiteral("DiffusionRate"));
    ASSERT_NE(spinBox, nullptr);
    spinBox->setValue(0.18);
    lineEdit = spinBox->findChild<QLineEdit*>();
    ASSERT_NE(lineEdit, nullptr);
    QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(lineEdit, &escape);
    drainGuiEvents();
    EXPECT_DOUBLE_EQ(grid->getDiffusionRate(), 0.0);
}

TEST(PropertyEditorDoubleCommit, GraphicalDataDefinitionDiffusionRateCommitsOnFocusOut) {
    ScopedDefaultLocale locale(QLocale::c());

    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* grid =
        manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_GraphicalFocusOut");
    ASSERT_NE(grid, nullptr);
    grid->setDiffusionRate(0.0);
    Plugin* plugin = manager->find(Util::TypeOf<BacteriaSignalGrid>());
    ASSERT_NE(plugin, nullptr);

    ModelGraphicsView graphicsView;
    graphicsView.setSimulator(&simulator);
    auto* graphicalGrid = new GraphicalModelDataDefinition(
        plugin, grid, QPointF(100.0, 100.0), QColor(Qt::blue));
    graphicalGrid->setEditableInPropertyEditor(true);

    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    QSet<QString> editableObjects;
    editableObjects.insert(QString::fromStdString(grid->getName()));
    browser.setActiveObject(graphicalGrid, grid, {}, editableObjects, nullptr, nullptr, nullptr, nullptr);
    drainGuiEvents();

    QDoubleSpinBox* diffusionEditor = beginDoubleEdit(browser, QStringLiteral("DiffusionRate"));
    ASSERT_NE(diffusionEditor, nullptr);
    QLineEdit* diffusionLineEdit = diffusionEditor->findChild<QLineEdit*>();
    ASSERT_NE(diffusionLineEdit, nullptr);
    diffusionLineEdit->setFocus();
    diffusionLineEdit->selectAll();
    diffusionLineEdit->setText(QStringLiteral("0.37"));

    QtBrowserItem* decayRate = findBrowserItemByName(
        browser.topLevelItems(), QStringLiteral("DecayRate"));
    ASSERT_NE(decayRate, nullptr);
    browser.setCurrentItem(decayRate);
    browser.editItem(decayRate);
    drainGuiEvents();

    EXPECT_DOUBLE_EQ(grid->getDiffusionRate(), 0.37);
}

TEST(PropertyEditorDoubleCommit, CommitIsNotDroppedWhenBindingIsCleared) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* grid =
        manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_QueuedCommit");
    ASSERT_NE(grid, nullptr);
    grid->setDiffusionRate(0.0);

    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    bindEditableKernelObject(browser, grid);

    QtBrowserItem* item = findBrowserItemByName(
        browser.topLevelItems(), QStringLiteral("DiffusionRate"));
    ASSERT_NE(item, nullptr);
    ASSERT_TRUE(QMetaObject::invokeMethod(
        &browser,
        "onVariantEditorCommitted",
        Qt::DirectConnection,
        Q_ARG(QtProperty*, item->property()),
        Q_ARG(QVariant, QVariant(0.37))));

    browser.clearCurrentlyConnectedObject();
    drainGuiEvents();

    EXPECT_DOUBLE_EQ(grid->getDiffusionRate(), 0.37);
}

TEST(PropertyEditorDoubleCommit, IntegerStringAndBoolEditorsRemainFunctional) {
    ScopedDefaultLocale locale(QLocale(QLocale::Portuguese, QLocale::Brazil));

    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* grid =
        manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_NonDoubleRegression");
    ASSERT_NE(grid, nullptr);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();

    int modelChangedCount = 0;
    bindEditableKernelObject(browser, grid, &modelChangedCount);

    const int countBeforeWidth = modelChangedCount;
    QSpinBox* widthEditor = beginIntegerEdit(browser, QStringLiteral("Width"));
    ASSERT_NE(widthEditor, nullptr);
    commitTextWithEnter(widthEditor->findChild<QLineEdit*>(), QStringLiteral("7"));
    EXPECT_EQ(grid->getWidth(), 7u);
    EXPECT_EQ(modelChangedCount, countBeforeWidth + 1);

    QtBrowserItem* initialValues =
        findBrowserItemByName(browser.topLevelItems(), QStringLiteral("InitialValues"));
    ASSERT_NE(initialValues, nullptr);
    browser.setCurrentItem(initialValues);
    browser.editItem(initialValues);
    drainGuiEvents();
    QLineEdit* stringEditor = browser.findChild<QLineEdit*>();
    ASSERT_NE(stringEditor, nullptr);
    const int countBeforeString = modelChangedCount;
    commitTextWithEnter(stringEditor, QStringLiteral("1, 2, 3"));
    EXPECT_EQ(grid->getInitialValues(), "1, 2, 3");
    EXPECT_EQ(modelChangedCount, countBeforeString + 1);

    auto* genericObject = new GenericDoubleDataDefinition(model);
    ASSERT_NE(genericObject, nullptr);
    bindEditableKernelObject(browser, genericObject, &modelChangedCount);

    QtBrowserItem* enabled =
        findBrowserItemByName(browser.topLevelItems(), QStringLiteral("Enabled"));
    ASSERT_NE(enabled, nullptr);
    browser.setCurrentItem(enabled);
    browser.editItem(enabled);
    drainGuiEvents();
    QCheckBox* checkBox = browser.findChild<QCheckBox*>();
    ASSERT_NE(checkBox, nullptr);
    checkBox->click();
    drainGuiEvents();
    EXPECT_TRUE(genericObject->enabled());
}

TEST(ModelLanguageSynchronizerRegression, RefreshIsSignalSafeCommentFilteredAndKeepsGroPayload) {
    QFile::remove(QStringLiteral("./temp.tmp"));

    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    ASSERT_NE(model->getPersistence(), nullptr);
    model->getPersistence()->setHasChanged(true);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_TextSync");
    ASSERT_NE(program, nullptr);
    const std::string sourceCode =
        "include gro;\n"
        "set ( \"dt\", 0.18 );\n"
        "# payload line that belongs to GRO source\n"
        "// another GRO source comment\n"
        "program p() := { message ( 1, \"x=y\" ) };\n";
    program->setSourceCode(sourceCode);

    QPlainTextEdit editor;
    bool textModelHasChanged = true;
    int textChangedCount = 0;
    QObject::connect(&editor, &QPlainTextEdit::textChanged, [&]() {
        ++textChangedCount;
    });

    QWidget owner;
    ModelLanguageSynchronizer synchronizer(
        &simulator,
        &editor,
        &textModelHasChanged,
        &owner,
        {});

    synchronizer.actualizeModelSimLanguage();

    const QString displayed = editor.toPlainText();
    EXPECT_FALSE(displayed.isEmpty());
    EXPECT_FALSE(textModelHasChanged);
    EXPECT_EQ(textChangedCount, 0);
    EXPECT_TRUE(model->getPersistence()->hasChanged())
        << "Generating the TextCodeEditor representation must not mark persistence clean";
    EXPECT_FALSE(QFile::exists(QStringLiteral("./temp.tmp")));

    const QStringList lines = displayed.split(QLatin1Char('\n'));
    for (const QString& line : lines) {
        EXPECT_FALSE(line.trimmed().startsWith(QLatin1Char('#')))
            << line.toStdString();
    }

    EXPECT_TRUE(displayed.contains(QStringLiteral("GroProgram")));
    EXPECT_TRUE(displayed.contains(QStringLiteral("sourceCode=e\"")));
    EXPECT_TRUE(displayed.contains(QStringLiteral("\\\"dt\\\"")));
    EXPECT_TRUE(displayed.contains(QStringLiteral("\\n# payload line that belongs to GRO source")));
    EXPECT_TRUE(displayed.contains(QStringLiteral("// another GRO source comment")));
}


TEST(ModelLanguageSynchronizerRegression, SemanticSceneEventsRefreshMainWindowTextView) {
    MainWindow window;
    window.resize(900, 700);
    window.show();
    drainGuiEvents();

    ASSERT_TRUE(QMetaObject::invokeMethod(
        &window, "on_actionModelNew_triggered", Qt::DirectConnection));
    drainGuiEvents();

    ModelGraphicsScene* scene = window.myScene();
    ASSERT_NE(scene, nullptr);
    Simulator* simulator = scene->getSimulator();
    ASSERT_NE(simulator, nullptr);
    PluginManager* manager = simulator->getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);

    QTabWidget* modelTabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModel"));
    ASSERT_NE(modelTabs, nullptr);
    modelTabs->setCurrentIndex(0);
    drainGuiEvents();

    QPlainTextEdit* editor =
        window.findChild<QPlainTextEdit*>(QStringLiteral("TextCodeEditor"));
    ASSERT_NE(editor, nullptr);

    GroProgram* program =
        manager->newInstance<GroProgram>(model, "GroProgram_SceneSync");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("include gro;\nset ( \"dt\", 0.18 );\n");

    scene->notifyGraphicalModelChange(
        GraphicalModelEvent::EventType::CREATE,
        GraphicalModelEvent::EventObjectType::DATADEFINITION,
        nullptr);
    drainGuiEvents();
    EXPECT_TRUE(editor->toPlainText().contains(QStringLiteral("GroProgram_SceneSync")));

    model->getDataManager()->remove(program);
    scene->notifyGraphicalModelChange(
        GraphicalModelEvent::EventType::REMOVE,
        GraphicalModelEvent::EventObjectType::DATADEFINITION,
        nullptr);
    drainGuiEvents();
    EXPECT_FALSE(editor->toPlainText().contains(QStringLiteral("GroProgram_SceneSync")));
    delete program;

    Create* source = manager->newInstance<Create>(model, "Create_SceneSync");
    Dispose* destination = manager->newInstance<Dispose>(model, "Dispose_SceneSync");
    ASSERT_NE(source, nullptr);
    ASSERT_NE(destination, nullptr);

    scene->notifyGraphicalModelChange(
        GraphicalModelEvent::EventType::CREATE,
        GraphicalModelEvent::EventObjectType::COMPONENT,
        nullptr);
    drainGuiEvents();
    EXPECT_TRUE(editor->toPlainText().contains(QStringLiteral("Create_SceneSync")));
    EXPECT_TRUE(editor->toPlainText().contains(QStringLiteral("Dispose_SceneSync")));

    source->getConnectionManager()->insert(destination);
    scene->notifyGraphicalModelChange(
        GraphicalModelEvent::EventType::CREATE,
        GraphicalModelEvent::EventObjectType::CONNECTION,
        nullptr);
    drainGuiEvents();
    const QString destinationId =
        QStringLiteral("nextId=") + QString::number(destination->getId());
    EXPECT_TRUE(editor->toPlainText().contains(destinationId));

    source->getConnectionManager()->removeAtPort(0);
    scene->notifyGraphicalModelChange(
        GraphicalModelEvent::EventType::REMOVE,
        GraphicalModelEvent::EventObjectType::CONNECTION,
        nullptr);
    drainGuiEvents();
    EXPECT_FALSE(editor->toPlainText().contains(destinationId));

    model->getComponentManager()->remove(source);
    scene->notifyGraphicalModelChange(
        GraphicalModelEvent::EventType::REMOVE,
        GraphicalModelEvent::EventObjectType::COMPONENT,
        nullptr);
    drainGuiEvents();
    EXPECT_FALSE(editor->toPlainText().contains(QStringLiteral("Create_SceneSync")));
    delete source;
}


TEST(ModelLanguageSynchronizerRegression, VisualOnlySceneEventPreservesManualText) {
    MainWindow window;
    window.resize(900, 700);
    window.show();
    drainGuiEvents();

    ASSERT_TRUE(QMetaObject::invokeMethod(
        &window, "on_actionModelNew_triggered", Qt::DirectConnection));
    drainGuiEvents();

    ModelGraphicsScene* scene = window.myScene();
    ASSERT_NE(scene, nullptr);

    QTabWidget* modelTabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModel"));
    ASSERT_NE(modelTabs, nullptr);
    modelTabs->setCurrentIndex(0);
    drainGuiEvents();

    QPlainTextEdit* editor =
        window.findChild<QPlainTextEdit*>(QStringLiteral("TextCodeEditor"));
    ASSERT_NE(editor, nullptr);

    const QString manualText = QStringLiteral("manual text must survive a drawing-only scene event");
    editor->setPlainText(manualText);
    drainGuiEvents();
    EXPECT_EQ(editor->toPlainText(), manualText);

    scene->notifyGraphicalModelChange(
        GraphicalModelEvent::EventType::EDIT,
        GraphicalModelEvent::EventObjectType::DRAWING,
        nullptr);
    drainGuiEvents();

    EXPECT_EQ(editor->toPlainText(), manualText);
}

TEST(ModelLanguageSynchronizerRegression, ModelTabsKeepManualTextIsolated) {
    MainWindow window;
    window.resize(900, 700);
    window.show();
    drainGuiEvents();

    ASSERT_TRUE(QMetaObject::invokeMethod(
        &window, "on_actionModelNew_triggered", Qt::DirectConnection));
    drainGuiEvents();

    QPlainTextEdit* editor =
        window.findChild<QPlainTextEdit*>(QStringLiteral("TextCodeEditor"));
    ASSERT_NE(editor, nullptr);
    QTabWidget* graphicsTabs =
        window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModelGraphics"));
    ASSERT_NE(graphicsTabs, nullptr);

    const int firstIndex = graphicsTabs->currentIndex();
    ASSERT_GE(firstIndex, 0);

    const QString firstText = QStringLiteral("manual text for model A");
    editor->setPlainText(firstText);
    drainGuiEvents();

    ASSERT_TRUE(QMetaObject::invokeMethod(
        &window, "on_actionModelNew_triggered", Qt::DirectConnection));
    drainGuiEvents();
    ASSERT_GE(graphicsTabs->count(), 2);

    const int secondIndex = graphicsTabs->currentIndex();
    ASSERT_GE(secondIndex, 0);
    ASSERT_NE(secondIndex, firstIndex);

    const QString secondText = QStringLiteral("manual text for model B");
    editor->setPlainText(secondText);
    drainGuiEvents();

    graphicsTabs->setCurrentIndex(firstIndex);
    drainGuiEvents();
    EXPECT_EQ(editor->toPlainText(), firstText);

    graphicsTabs->setCurrentIndex(secondIndex);
    drainGuiEvents();
    EXPECT_EQ(editor->toPlainText(), secondText);
}

TEST(ModelLanguageSynchronizerRegression, ComponentAndConnectionUndoRedoRefreshTextView) {
    MainWindow window;
    window.resize(900, 700);
    window.show();
    drainGuiEvents();

    ASSERT_TRUE(QMetaObject::invokeMethod(
        &window, "on_actionModelNew_triggered", Qt::DirectConnection));
    drainGuiEvents();

    ModelGraphicsScene* scene = window.myScene();
    ASSERT_NE(scene, nullptr);
    QUndoStack* undoStack = scene->getUndoStack();
    ASSERT_NE(undoStack, nullptr);

    Simulator* simulator = scene->getSimulator();
    ASSERT_NE(simulator, nullptr);
    PluginManager* manager = simulator->getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);

    QTabWidget* modelTabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModel"));
    ASSERT_NE(modelTabs, nullptr);
    modelTabs->setCurrentIndex(0);
    drainGuiEvents();

    QPlainTextEdit* editor =
        window.findChild<QPlainTextEdit*>(QStringLiteral("TextCodeEditor"));
    ASSERT_NE(editor, nullptr);

    Create* source = manager->newInstance<Create>(model, "Create_UndoSync");
    Dispose* destination = manager->newInstance<Dispose>(model, "Dispose_UndoSync");
    ASSERT_NE(source, nullptr);
    ASSERT_NE(destination, nullptr);

    Plugin* createPlugin = manager->find(source->getClassname());
    Plugin* disposePlugin = manager->find(destination->getClassname());
    ASSERT_NE(createPlugin, nullptr);
    ASSERT_NE(disposePlugin, nullptr);

    GraphicalModelComponent* graphicalSource =
        scene->addGraphicalModelComponent(createPlugin, source, QPointF(120.0, 120.0), Qt::blue, true);
    GraphicalModelComponent* graphicalDestination =
        scene->addGraphicalModelComponent(disposePlugin, destination, QPointF(360.0, 120.0), Qt::blue, true);
    ASSERT_NE(graphicalSource, nullptr);
    ASSERT_NE(graphicalDestination, nullptr);
    drainGuiEvents();

    EXPECT_TRUE(editor->toPlainText().contains(QStringLiteral("Create_UndoSync")));
    EXPECT_TRUE(editor->toPlainText().contains(QStringLiteral("Dispose_UndoSync")));

    undoStack->undo();
    drainGuiEvents();
    EXPECT_FALSE(editor->toPlainText().contains(QStringLiteral("Dispose_UndoSync")));

    undoStack->redo();
    drainGuiEvents();
    EXPECT_TRUE(editor->toPlainText().contains(QStringLiteral("Dispose_UndoSync")));

    const QList<GraphicalComponentPort*> sourcePorts = graphicalSource->getGraphicalOutputPorts();
    const QList<GraphicalComponentPort*> destinationPorts = graphicalDestination->getGraphicalInputPorts();
    ASSERT_FALSE(sourcePorts.isEmpty());
    ASSERT_FALSE(destinationPorts.isEmpty());

    auto* connection = new GraphicalConnection(sourcePorts.first(), destinationPorts.first());
    undoStack->push(new AddUndoCommand(connection, scene));
    drainGuiEvents();

    const QString destinationId =
        QStringLiteral("nextId=") + QString::number(destination->getId());
    EXPECT_EQ(source->getConnectionManager()->size(), 1u);
    EXPECT_TRUE(editor->toPlainText().contains(destinationId));

    undoStack->undo();
    drainGuiEvents();
    EXPECT_EQ(source->getConnectionManager()->size(), 0u);
    EXPECT_FALSE(editor->toPlainText().contains(destinationId));

    undoStack->redo();
    drainGuiEvents();
    EXPECT_EQ(source->getConnectionManager()->size(), 1u);
    EXPECT_TRUE(editor->toPlainText().contains(destinationId));

    Create* deleted = manager->newInstance<Create>(model, "Create_DeleteUndoSync");
    ASSERT_NE(deleted, nullptr);
    Plugin* deletedPlugin = manager->find(deleted->getClassname());
    ASSERT_NE(deletedPlugin, nullptr);
    GraphicalModelComponent* graphicalDeleted =
        scene->addGraphicalModelComponent(deletedPlugin, deleted, QPointF(120.0, 320.0), Qt::blue, true);
    ASSERT_NE(graphicalDeleted, nullptr);
    drainGuiEvents();
    EXPECT_TRUE(editor->toPlainText().contains(QStringLiteral("Create_DeleteUndoSync")));

    QList<QGraphicsItem*> deleteItems;
    deleteItems.append(graphicalDeleted);
    undoStack->push(new DeleteUndoCommand(deleteItems, scene));
    drainGuiEvents();
    EXPECT_FALSE(editor->toPlainText().contains(QStringLiteral("Create_DeleteUndoSync")));

    undoStack->undo();
    drainGuiEvents();
    EXPECT_TRUE(editor->toPlainText().contains(QStringLiteral("Create_DeleteUndoSync")));

    undoStack->redo();
    drainGuiEvents();
    EXPECT_FALSE(editor->toPlainText().contains(QStringLiteral("Create_DeleteUndoSync")));
}
