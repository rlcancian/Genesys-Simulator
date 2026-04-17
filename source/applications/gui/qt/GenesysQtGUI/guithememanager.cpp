#include "guithememanager.h"

#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "systempreferences.h"
#include "UtilGUI.h"

#include <QApplication>
#include <QFont>
#include <QGraphicsLineItem>
#include <QPalette>
#include <QStyleFactory>

namespace {
QPalette lightPalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(245, 245, 245));
    palette.setColor(QPalette::WindowText, QColor(24, 24, 24));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 232));
    palette.setColor(QPalette::ToolTipText, QColor(24, 24, 24));
    palette.setColor(QPalette::Text, QColor(24, 24, 24));
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, QColor(24, 24, 24));
    palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
    palette.setColor(QPalette::Highlight, QColor(45, 125, 210));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 120, 120));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));
    return palette;
}

QPalette darkPalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(43, 43, 43));
    palette.setColor(QPalette::WindowText, QColor(238, 238, 238));
    palette.setColor(QPalette::Base, QColor(32, 32, 32));
    palette.setColor(QPalette::AlternateBase, QColor(51, 51, 51));
    palette.setColor(QPalette::ToolTipBase, QColor(238, 238, 238));
    palette.setColor(QPalette::ToolTipText, QColor(24, 24, 24));
    palette.setColor(QPalette::Text, QColor(238, 238, 238));
    palette.setColor(QPalette::Button, QColor(55, 55, 55));
    palette.setColor(QPalette::ButtonText, QColor(238, 238, 238));
    palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
    palette.setColor(QPalette::Link, QColor(88, 166, 255));
    palette.setColor(QPalette::Highlight, QColor(64, 132, 214));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(150, 150, 150));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150));
    return palette;
}

QString darkStyleSheet() {
    return QStringLiteral(
        "QToolTip { color: #181818; background-color: #eeeeee; border: 1px solid #767676; }"
        "QMenuBar, QMenu, QToolBar, QStatusBar { background-color: #2f2f2f; color: #eeeeee; }"
        "QMenu::item:selected, QMenuBar::item:selected { background-color: #4a4a4a; }"
        "QDockWidget::title { background-color: #353535; color: #eeeeee; padding: 4px; }"
        "QTabWidget::pane { border: 1px solid #595959; }"
        "QTabBar::tab { background: #383838; color: #eeeeee; padding: 6px 10px; border: 1px solid #595959; }"
        "QTabBar::tab:selected { background: #454545; }"
        "QHeaderView::section { background-color: #3d3d3d; color: #eeeeee; border: 1px solid #595959; padding: 4px; }"
        "QTableView, QTreeView, QListView, QTextEdit, QPlainTextEdit, QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {"
        " background-color: #202020; color: #eeeeee; selection-background-color: #4084d6; selection-color: #ffffff;"
        " border: 1px solid #595959; }"
        "QPushButton { background-color: #3d3d3d; color: #eeeeee; border: 1px solid #6a6a6a; padding: 4px 10px; }"
        "QPushButton:hover { background-color: #4a4a4a; }"
        "QPushButton:pressed { background-color: #565656; }"
        "QGroupBox { border: 1px solid #595959; margin-top: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
}

QString lightStyleSheet() {
    return QStringLiteral(
        "QToolTip { color: #181818; background-color: #ffffe8; border: 1px solid #9a9a9a; }"
        "QHeaderView::section { padding: 4px; }"
        "QPushButton { padding: 4px 10px; }");
}
}

void GuiThemeManager::applyApplicationTheme(QApplication* application) {
    if (application == nullptr) {
        return;
    }

    const bool modern = SystemPreferences::interfaceStyle() == SystemPreferences::InterfaceStyle::Modern;
    const bool dark = SystemPreferences::visualTheme() == SystemPreferences::VisualTheme::Dark;
    if (modern || dark) {
        application->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    }

    application->setPalette(dark ? darkPalette() : lightPalette());
    application->setStyleSheet(dark ? darkStyleSheet() : lightStyleSheet());

    const int configuredFontPointSize = SystemPreferences::applicationFontPointSize();
    if (configuredFontPointSize > 0) {
        QFont font = application->font();
        font.setPointSize(configuredFontPointSize);
        application->setFont(font);
    }
}

void GuiThemeManager::applyModelGraphicsTheme(ModelGraphicsView* graphicsView) {
    if (graphicsView == nullptr) {
        return;
    }

    graphicsView->setEnabled(graphicsView->isEnabled());
    ModelGraphicsScene* scene = graphicsView->getScene();
    if (scene == nullptr || scene->grid() == nullptr) {
        return;
    }

    if (SystemPreferences::diagramUsesThemeColors()) {
        QPen gridPen(UtilGUI::rgbaFromPacked(SystemPreferences::gridColor()));
        gridPen.setWidth(scene->grid()->pen.width());
        gridPen.setStyle(scene->grid()->pen.style());
        scene->grid()->pen = gridPen;
        for (QGraphicsLineItem* line : *scene->grid()->lines) {
            if (line != nullptr) {
                line->setPen(gridPen);
            }
        }
    }
    scene->update();
}
