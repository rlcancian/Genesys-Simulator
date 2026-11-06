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
