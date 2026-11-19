// Exercises the real graphical serializer and both kernel/GUI temp-file levels.
#include <gtest/gtest.h>

#include <QAction>
#include <QFile>
#include <QPlainTextEdit>
#include <QSlider>
#include <QTemporaryDir>
#include <QTextEdit>

#include "graphicals/ModelGraphicsView.h"
#include "graphicals/ModelGraphicsScene.h"
#include "services/GraphicalModelSerializer.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/components/Decisions/Decide.h"

namespace {
struct GuiSaveHarness {
    ModelGraphicsView view;
    QPlainTextEdit editor;
    QTextEdit console;
    QSlider zoom;
    QAction grid, rule, snap, guides, internals, editable, attached, recursive;
    QString filename;
    std::unique_ptr<GraphicalModelSerializer> serializer;

    explicit GuiSaveHarness(Simulator* simulator) {
        view.setSimulator(simulator);
        view.getScene()->setSimulator(simulator);
        serializer = std::make_unique<GraphicalModelSerializer>(
            simulator, &view, &editor, &view, &zoom, &grid, &rule, &snap,
            &guides, &internals, &editable, &attached, &recursive, &console,
            &filename, []() {}, []() {}, []() {}, []() {}, []() {});
    }
};

bool hasGuiDiagnostic(Model* model, const std::string& code) {
    for (const auto& message : *model->getTracer()->errorMessages()->list()) {
        if (message.find(code) != std::string::npos) return true;
    }
    return false;
}

QByteArray readAll(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return file.readAll();
}
} // namespace

TEST(ModelSaveGuiDiagnostics, RepeatedIncompleteGuiSaveUsesTwoCleanTemporaryLevels) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    new Resource(model, "IndependentGuiResource");
    auto* incomplete = new Decide(model, "IncompleteGuiDecide");
    incomplete->addConditions("unresolved +");
    GuiSaveHarness harness(&simulator);
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString destination = dir.filePath("roundtrip.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(destination));
    const QByteArray first = readAll(destination);
    EXPECT_TRUE(first.contains("# Genesys Simulation Model"));
    EXPECT_TRUE(first.contains("# Genesys Graphic Model"));
    EXPECT_TRUE(first.contains("IndependentGuiResource"));
    EXPECT_FALSE(QFile::exists(destination + ".tmp"));
    EXPECT_FALSE(QFile::exists(destination + ".tmp.tmp"));

    ASSERT_TRUE(harness.serializer->saveGraphicalModel(destination));
    const QByteArray second = readAll(destination);
    EXPECT_TRUE(second.contains("# Genesys Simulation Model"));
    EXPECT_TRUE(second.contains("# Genesys Graphic Model"));
    EXPECT_FALSE(QFile::exists(destination + ".tmp"));
    EXPECT_FALSE(QFile::exists(destination + ".tmp.tmp"));
}

TEST(ModelSaveGuiDiagnostics, OverlaySerializationFailurePreservesPreviousGuiFile) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GuiSaveHarness harness(&simulator);
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString destination = dir.filePath("previous.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(destination));
    const QByteArray previous = readAll(destination);
    ASSERT_FALSE(previous.isEmpty());

    // A null drawing is an explicit error path in the production GUI serializer.
    harness.view.getScene()->getGraphicalGeometries()->append(nullptr);
    EXPECT_FALSE(harness.serializer->saveGraphicalModel(destination));
    EXPECT_TRUE(hasGuiDiagnostic(model, "GUI_SAVE_OVERLAY_SERIALIZATION_FAILED"));
    EXPECT_EQ(readAll(destination), previous);
    EXPECT_FALSE(QFile::exists(destination + ".tmp"));
    EXPECT_FALSE(QFile::exists(destination + ".tmp.tmp"));
}

#include "mainwindow.h"
#include "controllers/PropertyEditorController.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QTabWidget>
#include <QAbstractButton>

namespace {
// Drive the production modal widgets, with a bounded watchdog on every action.
struct LifecycleDialogs {
    QTimer driver, watchdog;
    QString path;
    QMessageBox::StandardButton answer = QMessageBox::Cancel;
    bool cancelFile = false;
    bool fileHandled = false;
    bool timedOut = false;
    int messages = 0;
    LifecycleDialogs() {
        QObject::connect(&driver, &QTimer::timeout, [&]() {
            QWidget* modal = QApplication::activeModalWidget();
            if (auto* file = qobject_cast<QFileDialog*>(modal)) {
                if (fileHandled) return;
                fileHandled = true;
                if (cancelFile) file->reject();
                else {
                    if (path.endsWith(".gen")) file->selectNameFilter("Genesys Model (*.gen)");
                    file->selectFile(path);
                    QMetaObject::invokeMethod(file, "accept", Qt::QueuedConnection);
                }
            } else if (auto* message = qobject_cast<QMessageBox*>(modal)) {
                ++messages;
                auto choice = message->standardButtons().testFlag(answer) ? answer : QMessageBox::Ok;
                if (auto* button = message->button(choice)) button->click();
                else message->reject();
            }
        });
        watchdog.setSingleShot(true);
        QObject::connect(&watchdog, &QTimer::timeout, [&]() {
            timedOut = true;
            if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) dialog->reject();
        });
        driver.start(5);
        watchdog.start(5000);
    }
};
Model* newGuiModel(MainWindow& window) {
    QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection);
    QCoreApplication::processEvents();
    return window.myScene()->getSimulator()->getModelManager()->current();
}
}

TEST(GuiClassroomLifecycle, FailedGenSavePreservesPreviousFileAndDirtyDocument) {
    MainWindow window;
    Model* model = newGuiModel(window);
    ASSERT_NE(model, nullptr);
    auto* first = new Resource(model, "UniqueResource");
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("lesson.gen");
    {
        LifecycleDialogs dialogs;
        dialogs.path = path;
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelSave_triggered", Qt::DirectConnection));
        EXPECT_TRUE(dialogs.fileHandled);
        EXPECT_FALSE(dialogs.timedOut);
    }
    const auto original = readAll(path);
    ASSERT_TRUE(original.contains("UniqueResource"));
    auto* duplicate = new Resource(model, "OtherResource");
    duplicate->setName(first->getName());
    window.setGraphicalModelHasChanged(true);
    {
        LifecycleDialogs dialogs;
        dialogs.path = path;
        dialogs.answer = QMessageBox::Yes; // overwrite confirmation
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelSave_triggered", Qt::DirectConnection));
        EXPECT_FALSE(dialogs.timedOut);
    }
    EXPECT_EQ(readAll(path), original);
    EXPECT_TRUE(window.graphicalModelHasChanged());
    EXPECT_TRUE(model->hasChanged());
}

TEST(GuiClassroomLifecycle, CancelSaveDuringCloseKeepsModelOpen) {
    MainWindow window;
    Model* model = newGuiModel(window);
    new Resource(model, "UnsavedResource");
    window.setGraphicalModelHasChanged(true);
    LifecycleDialogs dialogs;
    dialogs.answer = QMessageBox::Yes;
    dialogs.cancelFile = true;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelClose_triggered", Qt::DirectConnection));
    EXPECT_TRUE(dialogs.fileHandled);
    EXPECT_FALSE(dialogs.timedOut);
    EXPECT_EQ(window.myScene()->getSimulator()->getModelManager()->current(), model);
    EXPECT_TRUE(window.graphicalModelHasChanged());
}

TEST(GuiClassroomLifecycle, DestroyedPropertyControllerCancelsQueuedRefresh) {
    int refreshes = 0;
    auto controller = std::make_unique<PropertyEditorController>(
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        []() {}, [&](bool) { ++refreshes; }, [](bool) {}, []() { return false; }, []() {}, []() {});
    controller->onPropertyEditorModelChanged();
    controller.reset();
    QCoreApplication::processEvents();
    EXPECT_EQ(refreshes, 0);
}

#include <QUndoStack>
#include <QUndoCommand>

TEST(GuiClassroomLifecycle, RejectedOpenPreservesCurrentModelAndUndoHistory) {
    MainWindow window;
    Model* model = newGuiModel(window);
    new Resource(model, "KeepAfterInvalidOpen");
    auto* history = window.myScene()->getUndoStack();
    history->push(new QUndoCommand("Existing edit"));
    const int commands = history->count();
    window.setGraphicalModelHasChanged(true);
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString invalid = dir.filePath("broken.gen");
    QFile file(invalid);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("0 Simulator \"GenESyS\" versionNumber=260705\n140 UnknownClass \"Broken\"\n");
    file.close();
    LifecycleDialogs dialogs;
    dialogs.path = invalid;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelOpen_triggered", Qt::DirectConnection));
    EXPECT_FALSE(dialogs.timedOut);
    EXPECT_EQ(window.myScene()->getSimulator()->getModelManager()->current(), model);
    EXPECT_EQ(history->count(), commands);
    EXPECT_TRUE(window.graphicalModelHasChanged());
}

TEST(GuiClassroomLifecycle, FailedGraphicalSaveDuringCloseKeepsModelAndPreviousFile) {
    MainWindow window;
    Model* model = newGuiModel(window);
    new Resource(model, "KeepAfterFailedSave");
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("lesson.gui");
    {
        LifecycleDialogs dialogs;
        dialogs.path = path;
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelSave_triggered", Qt::DirectConnection));
        EXPECT_FALSE(dialogs.timedOut);
    }
    const auto original = readAll(path);
    ASSERT_FALSE(original.isEmpty());
    // The real serializer rejects an invalid drawing after kernel serialization.
    window.myScene()->getGraphicalGeometries()->append(nullptr);
    window.setGraphicalModelHasChanged(true);
    {
        LifecycleDialogs dialogs;
        dialogs.answer = QMessageBox::Yes;
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelClose_triggered", Qt::DirectConnection));
        EXPECT_FALSE(dialogs.timedOut);
    }
    EXPECT_EQ(window.myScene()->getSimulator()->getModelManager()->current(), model);
    EXPECT_TRUE(window.graphicalModelHasChanged());
    EXPECT_EQ(readAll(path), original);
    window.myScene()->getGraphicalGeometries()->removeAll(nullptr);
}

TEST(GuiClassroomLifecycle, PropertyCommitMarksDirtyBeforeDeferredRefresh) {
    int marks = 0, refreshes = 0;
    PropertyEditorController controller(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        [&]() { ++marks; }, [&](bool) { ++refreshes; }, [](bool) {}, []() { return false; }, []() {}, []() {});
    for (int i = 0; i < 10; ++i) controller.onPropertyEditorModelChanged();
    EXPECT_EQ(marks, 10) << "Closing or switching tabs must not outrun dirty-state marking";
    EXPECT_EQ(refreshes, 0);
    QCoreApplication::processEvents();
    EXPECT_EQ(refreshes, 1) << "Ten commits in one event-loop turn need only one tree refresh";
}

#include "services/GraphicalModelBuilder.h"
#include "plugins/components/Logic/Assign.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/Delay.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/data/Logic/Set.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "kernel/simulator/essentialPlugins/Counter.h"

class GuiClassroomRoundTrip : public ::testing::TestWithParam<int> {};
TEST_P(GuiClassroomRoundTrip, SaveCloseOpenAndExecuteThroughMainWindow) {
    MainWindow window;
    Model* model = newGuiModel(window);
    auto* simulator = window.myScene()->getSimulator();
    auto* source = new Create(model, "LessonCreate");
    auto* sink = new Dispose(model, "LessonDispose");
    source->setMaxCreations(10ul);
    source->setTimeBetweenCreationsExpression("1");
    source->setTimeUnit(Util::TimeUnit::second);
    if (GetParam() == 0) {
        source->connectTo(sink);
    } else if (GetParam() == 1) {
        auto* decide = new Decide(model, "LessonDecide");
        auto* a = new Assign(model, "LessonA");
        auto* b = new Assign(model, "LessonB");
        decide->addConditions("1");
        a->addAssignment(new Assignment(model, "LessonAttribute", "2"));
        b->addAssignment(new Assignment(model, "LessonAttribute", "3"));
        source->connectTo(decide);
        decide->getConnectionManager()->insertAtPort(0, new Connection{a, {0}});
        decide->getConnectionManager()->insertAtPort(1, new Connection{b, {0}});
        a->connectTo(sink);
        b->connectTo(sink);
    } else {
        auto* resource = new Resource(model, "LessonResource");
        auto* queue = new Queue(model, "LessonQueue");
        auto* set = new Set(model, "LessonSet");
        set->addElementSet(resource);
        auto* seize = new Seize(model, "LessonSeize");
        auto* delay = new Delay(model, "LessonDelay");
        auto* release = new Release(model, "LessonRelease");
        seize->setQueue(queue);
        seize->addRequest(new SeizableItem(set));
        release->addReleaseRequests(new SeizableItem(set));
        delay->setDelayExpression("2");
        delay->setDelayTimeUnit(Util::TimeUnit::second);
        source->connectTo(seize); seize->connectTo(delay); delay->connectTo(release); release->connectTo(sink);
    }
    model->getSimulation()->setNumberOfReplications(1);
    model->getSimulation()->setReplicationLength(100, Util::TimeUnit::second);
    model->getSimulation()->setShowReportsAfterReplication(false);
    model->getSimulation()->setShowReportsAfterSimulation(false);
    ASSERT_TRUE(model->check());
    auto* tabs = window.findChild<QTabWidget*>("tabWidgetModelGraphics");
    auto* view = dynamic_cast<ModelGraphicsView*>(tabs->currentWidget());
    ASSERT_NE(view, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(simulator, view, view->getScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();
    const int componentCount = view->getScene()->graphicalModelComponentItems().size();
    const int connectionCount = view->getScene()->getGraphicalConnections()->size();
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("classroom.gui");
    {
        LifecycleDialogs dialogs;
        dialogs.path = path;
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelSave_triggered", Qt::DirectConnection));
        EXPECT_FALSE(dialogs.timedOut);
        EXPECT_TRUE(dialogs.fileHandled);
    }
    ASSERT_FALSE(readAll(path).isEmpty());
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelClose_triggered", Qt::DirectConnection));
    EXPECT_FALSE(simulator->getModelManager()->hasModel(model));
    {
        LifecycleDialogs dialogs;
        dialogs.path = path;
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelOpen_triggered", Qt::DirectConnection));
        EXPECT_FALSE(dialogs.timedOut);
    }
    auto* loaded = simulator->getModelManager()->current();
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(window.myScene()->graphicalModelComponentItems().size(), componentCount);
    EXPECT_EQ(window.myScene()->getGraphicalConnections()->size(), connectionCount);
    auto* animations = window.findChild<QAction*>("actionAnimation");
    ASSERT_NE(animations, nullptr);
    animations->setChecked(false);
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionAnimation_triggered", Qt::DirectConnection));
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionSimulationStart_triggered", Qt::DirectConnection));
    auto* count = dynamic_cast<Counter*>(loaded->getDataManager()->getDataDefinition(Util::TypeOf<Counter>(), "LessonDispose.CountNumberIn"));
    ASSERT_NE(count, nullptr);
    EXPECT_EQ(count->getCountValue(), 10);
    EXPECT_FALSE(loaded->getSimulation()->isRunning());
}
INSTANTIATE_TEST_SUITE_P(Lessons, GuiClassroomRoundTrip, ::testing::Values(0, 1, 2));

TEST(GuiClassroomLifecycle, PreviousNextRestoreEachDocumentsDirtyState) {
    MainWindow window;
    auto* first = newGuiModel(window);
    window.setGraphicalModelHasChanged(true);
    auto* second = newGuiModel(window);
    window.setGraphicalModelHasChanged(false);
    auto* previous = window.findChild<QAction*>("actionModelPrevious");
    auto* next = window.findChild<QAction*>("actionModelNext");
    ASSERT_NE(previous, nullptr);
    ASSERT_NE(next, nullptr);
    previous->trigger();
    EXPECT_EQ(window.myScene()->getSimulator()->getModelManager()->current(), first);
    EXPECT_TRUE(window.graphicalModelHasChanged());
    next->trigger();
    EXPECT_EQ(window.myScene()->getSimulator()->getModelManager()->current(), second);
    EXPECT_FALSE(window.graphicalModelHasChanged());
}

TEST(GuiClassroomLifecycle, CancelWindowClosePreservesAllOpenModels) {
    MainWindow window;
    auto* first = newGuiModel(window);
    new Resource(first, "FirstUnsaved");
    window.setGraphicalModelHasChanged(true);
    auto* second = newGuiModel(window);
    new Resource(second, "SecondUnsaved");
    window.setGraphicalModelHasChanged(true);
    window.show();
    LifecycleDialogs dialogs;
    dialogs.answer = QMessageBox::Cancel;
    EXPECT_FALSE(window.close());
    EXPECT_TRUE(window.isVisible());
    auto* manager = window.myScene()->getSimulator()->getModelManager();
    EXPECT_TRUE(manager->hasModel(first));
    EXPECT_TRUE(manager->hasModel(second));
    EXPECT_FALSE(dialogs.timedOut);
}

TEST(GuiClassroomLifecycle, ExplicitGenSaveUsesLiveModelAndKeepsFormatOnCloseSave) {
    MainWindow window;
    auto* model = newGuiModel(window);
    auto* resource = new Resource(model, "LiveResource");
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const auto path = dir.filePath("live.gen");
    {
        LifecycleDialogs dialogs;
        dialogs.path = path;
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelSave_triggered", Qt::DirectConnection));
        EXPECT_FALSE(dialogs.timedOut);
    }
    resource->setCapacity(7);
    window.setGraphicalModelHasChanged(true);
    {
        LifecycleDialogs dialogs;
        dialogs.answer = QMessageBox::Yes;
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelClose_triggered", Qt::DirectConnection));
        EXPECT_FALSE(dialogs.fileHandled);
        EXPECT_FALSE(dialogs.timedOut);
    }
    EXPECT_FALSE(QFile::exists(dir.filePath("live.gui")));
    Simulator restored;
    restored.getPluginManager()->autoInsertPlugins();
    auto* loaded = restored.getModelManager()->loadModel(path.toStdString());
    ASSERT_NE(loaded, nullptr);
    auto* loadedResource = dynamic_cast<Resource*>(loaded->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), "LiveResource"));
    ASSERT_NE(loadedResource, nullptr);
    EXPECT_EQ(loadedResource->getCapacity(), 7u);
}
