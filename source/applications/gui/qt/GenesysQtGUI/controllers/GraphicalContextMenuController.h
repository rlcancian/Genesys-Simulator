#ifndef GRAPHICALCONTEXTMENUCONTROLLER_H
#define GRAPHICALCONTEXTMENUCONTROLLER_H

#include <functional>

class QContextMenuEvent;
class QGraphicsItem;
class QMenu;
class ModelGraphicsView;
class ModelGraphicsScene;

namespace Ui {
class MainWindow;
}

/**
 * @brief Builds context menus for the graphical model view using existing main-window actions.
 *
 * The controller intentionally reuses QAction instances from the main window so shortcuts,
 * enabled state, icons, checked state, and command handlers remain centralized.
 */
class GraphicalContextMenuController {
public:
    /**
     * @brief Creates the controller without taking ownership of UI or scene objects.
     */
    GraphicalContextMenuController(ModelGraphicsView* graphicsView,
                                   Ui::MainWindow* ui,
                                   std::function<ModelGraphicsScene*()> currentScene,
                                   std::function<void()> actualizeActions);

    /**
     * @brief Shows the context menu matching the item or background under the mouse.
     */
    void handleGraphicsViewContextMenu(QContextMenuEvent* event);

private:
    enum class ContextKind {
        Background,
        SingleComponent,
        SingleDataDefinition,
        SingleConnection,
        SingleDiagramConnection,
        SingleGroup,
        MultiSelection,
        GenericItem
    };

    /**
     * @brief Maps infrastructure children to the user-facing graphical item they represent.
     */
    QGraphicsItem* semanticItemFor(QGraphicsItem* item) const;

    /**
     * @brief Keeps right-click commands aligned with the item that opened the menu.
     */
    void synchronizeSelectionForContext(QGraphicsItem* item) const;

    /**
     * @brief Classifies the current click after selection synchronization has run.
     */
    ContextKind classifyContext(QGraphicsItem* item) const;

    /**
     * @brief Adds actions for right-clicking the empty canvas area.
     */
    void populateBackgroundMenu(QMenu* menu) const;

    /**
     * @brief Adds actions for right-clicking a graphical item or current selection.
     */
    void populateItemMenu(QMenu* menu, ContextKind contextKind) const;

    /**
     * @brief Adds edit commands shared by every item context.
     */
    void addEditActions(QMenu* menu) const;

    /**
     * @brief Adds visibility toggles that affect the graphical scene.
     */
    void addShowMenu(QMenu* menu) const;

    /**
     * @brief Adds zoom commands that navigate the graphical scene.
     */
    void addZoomMenu(QMenu* menu) const;

    /**
     * @brief Adds drawing tool actions used when the user starts from the canvas background.
     */
    void addDrawMenu(QMenu* menu) const;

    /**
     * @brief Adds animation tool actions used when the user starts from the canvas background.
     */
    void addAnimateMenu(QMenu* menu) const;

    /**
     * @brief Adds alignment commands for multi-selection contexts.
     */
    void addAlignMenu(QMenu* menu) const;

    ModelGraphicsView* _graphicsView;
    Ui::MainWindow* _ui;
    std::function<ModelGraphicsScene*()> _currentScene;
    std::function<void()> _actualizeActions;
};

#endif // GRAPHICALCONTEXTMENUCONTROLLER_H
