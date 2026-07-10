#include "guithememanager.h"

#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "systempreferences.h"
#include "UtilGUI.h"

#include <QApplication>
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>

namespace {
QString initialStyleName(QApplication* application) {
    static const QString styleName = application != nullptr && application->style() != nullptr
                                         ? application->style()->objectName()
                                         : QString();
    return styleName;
}

QFont initialApplicationFont(QApplication* application) {
    static const QFont font = application != nullptr ? application->font() : QFont();
    return font;
}

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

QString modernLightStyleSheet() {
    return QStringLiteral(
        "QToolTip { color: #111827; background-color: #f8fafc; border: 1px solid #94a3b8; padding: 4px; }"
        "QMainWindow, QDialog { background-color: #f3f6fa; }"
        "QMenuBar { background-color: #ffffff; color: #1f2937; border-bottom: 1px solid #d8dee8; spacing: 4px; }"
        "QMenuBar::item { padding: 6px 10px; border-radius: 6px; }"
        "QMenuBar::item:selected { background-color: #e8f1ff; color: #0f172a; }"
        "QMenu { background-color: #ffffff; color: #1f2937; border: 1px solid #d8dee8; padding: 4px; }"
        "QMenu::item { padding: 6px 24px 6px 18px; border-radius: 6px; }"
        "QMenu::item:selected { background-color: #2f80ed; color: #ffffff; }"
        "QToolBar { background-color: #edf2f7; border: 0; border-bottom: 1px solid #d8dee8; spacing: 5px; padding: 4px; }"
        "QToolButton { border: 1px solid transparent; border-radius: 6px; padding: 5px; }"
        "QToolButton:hover { background-color: #ffffff; border-color: #b6c5d8; }"
        "QStatusBar { background-color: #edf2f7; color: #334155; border-top: 1px solid #d8dee8; }"
        "QGroupBox { background-color: #ffffff; border: 1px solid #d8dee8; border-radius: 8px; margin-top: 12px; padding: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #334155; }"
        "QTabWidget::pane { background-color: #ffffff; border: 1px solid #d8dee8; border-radius: 8px; top: -1px; }"
        "QTabBar::tab { background: #e9eef5; color: #334155; border: 1px solid #d8dee8; padding: 7px 12px; margin-right: 3px; border-top-left-radius: 7px; border-top-right-radius: 7px; }"
        "QTabBar::tab:selected { background: #ffffff; color: #0f172a; border-bottom-color: #ffffff; }"
        "QHeaderView::section { background-color: #edf2f7; color: #334155; border: 0; border-right: 1px solid #d8dee8; border-bottom: 1px solid #d8dee8; padding: 6px; }"
        "QTableView, QTreeView, QListView, QTextEdit, QPlainTextEdit, QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {"
        " background-color: #ffffff; color: #1f2937; selection-background-color: #2f80ed; selection-color: #ffffff;"
        " border: 1px solid #cbd5e1; border-radius: 6px; padding: 3px; }"
        "QTableView, QTreeView, QListView { alternate-background-color: #f8fafc; gridline-color: #e2e8f0; }"
        "QPushButton { background-color: #ffffff; color: #1f2937; border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #e8f1ff; border-color: #2f80ed; }"
        "QPushButton:pressed { background-color: #d7e8ff; }"
        "QPushButton:default { background-color: #2f80ed; color: #ffffff; border-color: #1d6fd1; }"
        "QScrollBar:vertical { background: #edf2f7; width: 12px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #b6c5d8; border-radius: 6px; min-height: 24px; }"
        "QScrollBar:horizontal { background: #edf2f7; height: 12px; margin: 0; }"
        "QScrollBar::handle:horizontal { background: #b6c5d8; border-radius: 6px; min-width: 24px; }");
}

QString modernDarkStyleSheet() {
    return QStringLiteral(
        "QToolTip { color: #111827; background-color: #f8fafc; border: 1px solid #94a3b8; padding: 4px; }"
        "QMainWindow, QDialog { background-color: #20242b; }"
        "QMenuBar { background-color: #242a33; color: #e5e7eb; border-bottom: 1px solid #3a4351; spacing: 4px; }"
        "QMenuBar::item { padding: 6px 10px; border-radius: 6px; }"
        "QMenuBar::item:selected { background-color: #334155; }"
        "QMenu { background-color: #242a33; color: #e5e7eb; border: 1px solid #3a4351; padding: 4px; }"
        "QMenu::item { padding: 6px 24px 6px 18px; border-radius: 6px; }"
        "QMenu::item:selected { background-color: #3b82f6; color: #ffffff; }"
        "QToolBar { background-color: #252b34; border: 0; border-bottom: 1px solid #3a4351; spacing: 5px; padding: 4px; }"
        "QToolButton { color: #e5e7eb; border: 1px solid transparent; border-radius: 6px; padding: 5px; }"
        "QToolButton:hover { background-color: #303846; border-color: #475569; }"
        "QStatusBar { background-color: #252b34; color: #cbd5e1; border-top: 1px solid #3a4351; }"
        "QGroupBox { background-color: #242a33; border: 1px solid #3a4351; border-radius: 8px; margin-top: 12px; padding: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #cbd5e1; }"
        "QTabWidget::pane { background-color: #242a33; border: 1px solid #3a4351; border-radius: 8px; top: -1px; }"
        "QTabBar::tab { background: #2b3340; color: #cbd5e1; border: 1px solid #3a4351; padding: 7px 12px; margin-right: 3px; border-top-left-radius: 7px; border-top-right-radius: 7px; }"
        "QTabBar::tab:selected { background: #242a33; color: #ffffff; border-bottom-color: #242a33; }"
        "QHeaderView::section { background-color: #2b3340; color: #dbeafe; border: 0; border-right: 1px solid #3a4351; border-bottom: 1px solid #3a4351; padding: 6px; }"
        "QTableView, QTreeView, QListView, QTextEdit, QPlainTextEdit, QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {"
        " background-color: #1f242c; color: #e5e7eb; selection-background-color: #3b82f6; selection-color: #ffffff;"
        " border: 1px solid #475569; border-radius: 6px; padding: 3px; }"
        "QTableView, QTreeView, QListView { alternate-background-color: #252b34; gridline-color: #3a4351; }"
        "QPushButton { background-color: #303846; color: #f8fafc; border: 1px solid #475569; border-radius: 6px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #334155; border-color: #60a5fa; }"
        "QPushButton:pressed { background-color: #1d4ed8; }"
        "QPushButton:default { background-color: #3b82f6; color: #ffffff; border-color: #60a5fa; }"
        "QScrollBar:vertical { background: #252b34; width: 12px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #526173; border-radius: 6px; min-height: 24px; }"
        "QScrollBar:horizontal { background: #252b34; height: 12px; margin: 0; }"
        "QScrollBar::handle:horizontal { background: #526173; border-radius: 6px; min-width: 24px; }");
}

QString styleSheetFor(bool dark, bool modern) {
    if (modern) {
        return dark ? modernDarkStyleSheet() : modernLightStyleSheet();
    }
    return dark ? darkStyleSheet() : lightStyleSheet();
}

void repolishAllWidgets(QApplication* application) {
    if (application == nullptr || application->style() == nullptr) {
        return;
    }
    for (QWidget* widget : application->allWidgets()) {
        if (widget == nullptr) {
            continue;
        }
        application->style()->unpolish(widget);
        application->style()->polish(widget);
        widget->updateGeometry();
        widget->update();
    }
}
}

void GuiThemeManager::applyApplicationTheme(QApplication* application) {
    if (application == nullptr) {
        return;
    }

    const QString defaultStyleName = initialStyleName(application);
    const bool modern = SystemPreferences::interfaceStyle() == SystemPreferences::InterfaceStyle::Modern;
    const bool dark = SystemPreferences::visualTheme() == SystemPreferences::VisualTheme::Dark;
    if (modern || dark) {
        QStyle* fusionStyle = QStyleFactory::create(QStringLiteral("Fusion"));
        if (fusionStyle != nullptr) {
            application->setStyle(fusionStyle);
        }
    } else {
        QStyle* classicStyle = QStyleFactory::create(defaultStyleName);
        if (classicStyle != nullptr) {
            application->setStyle(classicStyle);
        }
    }

    application->setPalette(dark ? darkPalette() : lightPalette());
    application->setStyleSheet(styleSheetFor(dark, modern));

    const int configuredFontPointSize = SystemPreferences::applicationFontPointSize();
    QFont font = initialApplicationFont(application);
    if (configuredFontPointSize > 0) {
        font.setPointSize(configuredFontPointSize);
    } else if (modern && font.pointSize() > 0) {
        font.setPointSize(font.pointSize() + 1);
    }
    font.setStyleStrategy(QFont::PreferAntialias);
    application->setFont(font);

    repolishAllWidgets(application);
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
    for (QGraphicsItem* item : scene->items()) {
        if (item != nullptr) {
            item->update();
        }
    }
    scene->update();
    graphicsView->viewport()->update();
}
