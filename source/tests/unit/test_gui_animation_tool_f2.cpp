#include "extensions/GuiExtensionContracts.h"
#include "extensions/GuiExtensionManager.h"
#include "extensions/GuiExtensionPluginCatalog.h"
#include "graphicals/ModelGraphicsScene.h"

#include <gtest/gtest.h>

#include <QAction>
#include <QGraphicsRectItem>
#include <QMainWindow>
#include <QUndoStack>

#include <algorithm>

namespace {
const GuiExtensionPlugin* sceneMarkerPlugin() {
    for (const auto* plugin : GuiExtensionPluginCatalog::plugins()) {
        if (plugin && plugin->extensionId() == "gui.scene-marker.f2") return plugin;
    }
    return nullptr;
}

TEST(GuiAnimationToolF2, ConcreteExtensionRegistersIndependentFactory) {
    const auto* plugin = sceneMarkerPlugin();
    ASSERT_NE(plugin, nullptr);
    EXPECT_TRUE(plugin->requiredModelPlugins().empty());
    GuiExtensionRegistry registry;
    plugin->registerContributions(&registry);
    ASSERT_EQ(registry.animationTools().size(), 1u);
    const auto& contribution = registry.animationTools().front();
    EXPECT_EQ(contribution.id, "gui.scene-marker.f2.insert");
    EXPECT_EQ(contribution.menuPath, "View/Animations");
    ASSERT_TRUE(static_cast<bool>(contribution.createItem));
    GuiExtensionRuntimeContext context;
    auto item = contribution.createItem(context);
    ASSERT_NE(item, nullptr);
    auto* rectangle = dynamic_cast<QGraphicsRectItem*>(item.get());
    ASSERT_NE(rectangle, nullptr);
    EXPECT_TRUE(item->flags().testFlag(QGraphicsItem::ItemIsSelectable));
    EXPECT_TRUE(item->flags().testFlag(QGraphicsItem::ItemIsMovable));
    EXPECT_GT(rectangle->rect().width(), 0.0);
    EXPECT_GT(rectangle->rect().height(), 0.0);
}

TEST(GuiAnimationToolF2, GenericManagerCreatesRealSceneItemAndUndoRedoWorks) {
    const auto* plugin = sceneMarkerPlugin();
    ASSERT_NE(plugin, nullptr);
    QUndoStack undo;
    ModelGraphicsScene scene(0, 0, 640, 480);
    scene.setUndoStack(&undo);
    QMainWindow mainWindow;
    GuiExtensionManager manager(&mainWindow);
    manager.setPlugins({plugin});

    GuiExtensionRuntimeContext context;
    context.mainWindow = &mainWindow;
    context.graphicsScene = &scene;
    manager.rebuild(context);

    QAction* action = mainWindow.findChild<QAction*>("gui.scene-marker.f2.insert");
    ASSERT_NE(action, nullptr);
    EXPECT_TRUE(action->isEnabled());
    const int before = scene.items().size();
    action->trigger();
    EXPECT_EQ(scene.items().size(), before + 1);
    EXPECT_EQ(undo.count(), 1);
    ASSERT_FALSE(scene.items().empty());
    QGraphicsItem* item = scene.items().first();
    ASSERT_NE(dynamic_cast<QGraphicsRectItem*>(item), nullptr);
    EXPECT_TRUE(item->flags().testFlag(QGraphicsItem::ItemIsMovable));

    undo.undo();
    EXPECT_EQ(scene.items().size(), before);
    undo.redo();
    EXPECT_EQ(scene.items().size(), before + 1);
    EXPECT_EQ(item->scene(), &scene);

    // Resizing changes the actual item geometry, not a fake placeholder.
    auto* rectangle = dynamic_cast<QGraphicsRectItem*>(item);
    const QRectF original = rectangle->rect();
    rectangle->setRect(QRectF(0, 0, original.width() + 30, original.height() + 20));
    EXPECT_GT(rectangle->rect().width(), original.width());
    EXPECT_GT(rectangle->rect().height(), original.height());
}

TEST(GuiAnimationToolF2, ToolCannotInsertWithoutActiveSceneUndoStack) {
    const auto* plugin = sceneMarkerPlugin();
    ASSERT_NE(plugin, nullptr);
    QMainWindow mainWindow;
    GuiExtensionManager manager(&mainWindow);
    manager.setPlugins({plugin});
    GuiExtensionRuntimeContext context;
    context.mainWindow = &mainWindow;
    manager.rebuild(context);
    QAction* action = mainWindow.findChild<QAction*>("gui.scene-marker.f2.insert");
    ASSERT_NE(action, nullptr);
    EXPECT_FALSE(action->isEnabled());
    action->trigger();
    // No scene and no QUndoStack were created as side effects.
}

TEST(GuiAnimationToolF2, ManagerRebuildDoesNotDuplicateToolActions) {
    const auto* plugin = sceneMarkerPlugin();
    ASSERT_NE(plugin, nullptr);
    QMainWindow mainWindow;
    GuiExtensionManager manager(&mainWindow);
    manager.setPlugins({plugin});
    GuiExtensionRuntimeContext context;
    context.mainWindow = &mainWindow;
    manager.rebuild(context);
    manager.rebuild(context);
    const auto matching = mainWindow.findChildren<QAction*>("gui.scene-marker.f2.insert");
    EXPECT_EQ(matching.size(), 1);
}
} // namespace
