#include <gtest/gtest.h>
#include <QSet>
#include <QString>

#include "applications/gui/genesys/tools/expressionbuilder/ParserFunctionCatalog.h"
#include "parser/Genesys++-driver.h"
#include "applications/gui/genesys/tools/expressionbuilder/ExpressionBuilder.h"
#include "applications/gui/genesys/graphicals/ModelGraphicsScene.h"
#include "applications/gui/genesys/propertyeditor/ObjectPropertyBrowser.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelManager.h"
#include "kernel/simulator/model/ModelComponentManager.h"
#include "kernel/simulator/PluginManager.h"
#include "plugins/components/DiscreteProcessing/Delay.h"

#include <QApplication>
#include <QClipboard>
#include <QListWidget>
#include <QContextMenuEvent>
#include <QMenu>
#include <QTreeWidget>
#include <QCoreApplication>
#include <QEventLoop>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QTemporaryDir>

TEST(ExpressionBuilderCatalogTest, OnlyAdvertisesImplementedGrammarEntries) {
    const auto entries = parserFunctionCatalog();
    ASSERT_FALSE(entries.isEmpty());
    const QSet<QString> tokenOnlyOrStub = {
        QStringLiteral("abs"), QStringLiteral("pow"),
        QStringLiteral("disc"), QStringLiteral("lastinq"),
        QStringLiteral("resseizes"), QStringLiteral("numgr"),
        QStringLiteral("atrgr"), QStringLiteral("resutil")
    };
    QSet<QString> signatures;
    for (const auto& entry : entries) {
        EXPECT_FALSE(entry.category.isEmpty());
        EXPECT_FALSE(entry.name.isEmpty());
        EXPECT_FALSE(entry.syntax.isEmpty());
        EXPECT_FALSE(entry.description.isEmpty());
        EXPECT_FALSE(tokenOnlyOrStub.contains(entry.name)) << entry.name.toStdString();
        const QString signature = entry.name + QLatin1Char(':') + entry.syntax;
        EXPECT_FALSE(signatures.contains(signature)) << signature.toStdString();
        signatures.insert(signature);
    }
}

TEST(ExpressionBuilderCatalogTest, EveryExecutableExampleHasValidOrContextDependentSyntax) {
    genesyspp_driver driver(nullptr, nullptr, false);
    for (const auto& entry : parserFunctionCatalog()) {
        if (entry.name == QStringLiteral("operators")) {
            continue; // This is a reference row rather than an insertion template.
        }
        const auto validation = driver.validateSyntax(entry.example.toStdString());
        EXPECT_NE(validation.status, ExpressionSyntaxValidation::Status::InvalidSyntax)
            << entry.syntax.toStdString() << ": " << validation.errorMessage;
    }
}


namespace {
QtBrowserItem* findExpressionProperty(const QList<QtBrowserItem*>& items) {
    for (QtBrowserItem* item : items) {
        if (item != nullptr && item->property() != nullptr &&
            item->property()->propertyName() == QStringLiteral("DelayExpression")) {
            return item;
        }
        if (item != nullptr) {
            if (auto* child = findExpressionProperty(item->children())) return child;
        }
    }
    return nullptr;
}

void showEditableDelay(ObjectPropertyBrowser& browser, Delay& delay) {
    const QSet<QString> editable = {QString::fromStdString(delay.getName())};
    browser.setActiveObject(nullptr, &delay, {}, editable);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
}

void openSelectedExpressionEditor(ObjectPropertyBrowser& browser) {
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(&browser, &press);
}
} // namespace


TEST(ExpressionBuilderDialogTest, FunctionInsertionRespectsExistingTextAndCursor) {
    ExpressionBuilder editor;
    editor.setExpressionContext(nullptr, QStringLiteral("2 + "));
    auto* tree = editor.findChild<QTreeWidget*>(QStringLiteral("treeCategories"));
    auto* choices = editor.findChild<QListWidget*>(QStringLiteral("listExpressions"));
    auto* input = editor.findChild<QPlainTextEdit*>(QStringLiteral("expressionEditor"));
    auto* insert = editor.findChild<QPushButton*>(QStringLiteral("btnInsert"));
    ASSERT_NE(tree, nullptr);
    ASSERT_NE(choices, nullptr);
    ASSERT_NE(input, nullptr);
    ASSERT_NE(insert, nullptr);
    ASSERT_GT(tree->topLevelItemCount(), 0);
    EXPECT_EQ(tree->topLevelItem(0)->text(0), QStringLiteral("Arithmetic"));
    choices->setCurrentRow(0);
    EXPECT_GT(choices->count(), 0);
    input->moveCursor(QTextCursor::End);
    insert->click();
    EXPECT_EQ(editor.expressionText(), QStringLiteral("2 + round(3.7)"));
    EXPECT_TRUE(editor.validateCurrentExpression());
}

TEST(ExpressionBuilderDialogTest, NestedCatalogExamplePreservesArgumentStructure) {
    ExpressionBuilder editor;
    editor.setExpressionContext(nullptr, QString());
    auto* categories = editor.findChild<QTreeWidget*>(QStringLiteral("treeCategories"));
    auto* functions = editor.findChild<QListWidget*>(QStringLiteral("listExpressions"));
    auto* insert = editor.findChild<QPushButton*>(QStringLiteral("btnInsert"));
    ASSERT_NE(categories, nullptr);
    ASSERT_NE(functions, nullptr);
    ASSERT_NE(insert, nullptr);
    QTreeWidgetItem* math = nullptr;
    for (int i = 0; i < categories->topLevelItemCount(); ++i) {
        if (categories->topLevelItem(i)->text(0) == QStringLiteral("Math")) {
            math = categories->topLevelItem(i);
            break;
        }
    }
    ASSERT_NE(math, nullptr);
    categories->setCurrentItem(math);
    QListWidgetItem* ln = nullptr;
    for (int i = 0; i < functions->count(); ++i) {
        if (functions->item(i)->text() == QStringLiteral("ln(expression)")) {
            ln = functions->item(i);
            break;
        }
    }
    ASSERT_NE(ln, nullptr);
    functions->setCurrentItem(ln);
    insert->click();
    EXPECT_EQ(editor.expressionText(), QStringLiteral("ln(exp(1))"));
    EXPECT_TRUE(editor.validateCurrentExpression());
}

TEST(ExpressionBuilderDialogTest, StandaloneCopyUsesSelectedFunctionWhenExpressionIsEmpty) {
    ExpressionBuilder editor;
    editor.setExpressionContext(nullptr, QString());
    auto* choices = editor.findChild<QListWidget*>(QStringLiteral("listExpressions"));
    auto* copy = editor.findChild<QPushButton*>(QStringLiteral("btnCopyToClipboard"));
    ASSERT_NE(choices, nullptr);
    ASSERT_NE(copy, nullptr);
    choices->setCurrentRow(0);
    copy->click();
    EXPECT_EQ(QApplication::clipboard()->text(), QStringLiteral("round(3.7)"));
}

TEST(ExpressionBuilderDialogTest, EditsExpressionAndRejectsInvalidSyntaxWithoutEvaluation) {
    ExpressionBuilder editor;
    editor.setExpressionContext(nullptr, QStringLiteral("1 + 2"));
    auto* text = editor.findChild<QPlainTextEdit*>(QStringLiteral("expressionEditor"));
    auto* apply = editor.findChild<QPushButton*>(QStringLiteral("btnApply"));
    ASSERT_NE(text, nullptr);
    ASSERT_NE(apply, nullptr);
    EXPECT_EQ(editor.expressionText(), QStringLiteral("1 + 2"));

    text->setPlainText(QStringLiteral("norm(2, -1) + rnd + tnow"));
    EXPECT_TRUE(editor.validateCurrentExpression());
    text->setPlainText(QStringLiteral("norm(2)"));
    EXPECT_FALSE(editor.validateCurrentExpression());
    apply->click();
    EXPECT_NE(editor.result(), QDialog::Accepted);
    text->setPlainText(QStringLiteral("1+3"));
    apply->click();
    EXPECT_EQ(editor.result(), QDialog::Accepted);
    EXPECT_EQ(editor.expressionText(), QStringLiteral("1+3"));
}

TEST(ExpressionBuilderPropertyTest, DelayIsTaggedAsParserExpression) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Delay delay(model, "ExpressionHintDelay");
    bool found = false;
    for (SimulationControl* control : *delay.getSimulationControls()->list()) {
        if (control->getName() == "DelayExpression") {
            EXPECT_EQ(control->preferredEditorHint(), SimulationControlEditorHint::ParserExpression);
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST(ExpressionBuilderPropertyTest, CancelDoesNotMutateDelayExpression) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Delay delay(model, "CancelExpressionDelay");
    const std::string original = delay.delayExpression();
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    showEditableDelay(browser, delay);
    QtBrowserItem* item = findExpressionProperty(browser.topLevelItems());
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    bool opened = false;
    QTimer::singleShot(0, [&]() {
        auto* dialog = qobject_cast<ExpressionBuilder*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        opened = true;
        dialog->findChild<QPlainTextEdit*>(QStringLiteral("expressionEditor"))
            ->setPlainText(QStringLiteral("100 + 200"));
        dialog->reject();
    });
    QTimer::singleShot(1000, &browser, []() {
        if (auto* dialog = QApplication::activeModalWidget()) dialog->close();
    });
    openSelectedExpressionEditor(browser);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_TRUE(opened);
    EXPECT_EQ(delay.delayExpression(), original);
}


TEST(ExpressionBuilderPropertyTest, ContextMenuOpensExpressionBuilderAndCancelIsSafe) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Delay delay(model, "ContextExpressionDelay");
    const std::string original = delay.delayExpression();
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    showEditableDelay(browser, delay);
    QtBrowserItem* item = findExpressionProperty(browser.topLevelItems());
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    bool foundAction = false;
    bool foundDialog = false;
    QTimer::singleShot(0, [&]() {
        auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
        if (menu == nullptr) return;
        QAction* builderAction = nullptr;
        for (QAction* action : menu->actions()) {
            if (action->text() == QStringLiteral("Build expression...")) {
                builderAction = action;
                break;
            }
        }
        foundAction = builderAction != nullptr;
        if (builderAction == nullptr) { menu->close(); return; }
        QTimer::singleShot(0, [&]() {
            auto* dialog = qobject_cast<ExpressionBuilder*>(QApplication::activeModalWidget());
            if (dialog == nullptr) return;
            foundDialog = true;
            dialog->reject();
        });
        menu->setActiveAction(builderAction);
        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(menu, &enter);
    });
    QTimer::singleShot(1000, &browser, []() {
        if (auto* widget = QApplication::activeModalWidget()) widget->close();
        if (auto* menu = QApplication::activePopupWidget()) menu->close();
    });
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(5, 5),
                            browser.mapToGlobal(QPoint(5, 5)));
    QApplication::sendEvent(&browser, &event);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_TRUE(foundAction);
    EXPECT_TRUE(foundDialog);
    EXPECT_EQ(delay.delayExpression(), original);
}

TEST(ExpressionBuilderPropertyTest, AcceptCommitsExpressionOnceThroughPropertyBrowser) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    Delay delay(model, "CommitExpressionDelay");
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    int notifications = 0;
    browser.setModelChangedCallback([&]() { ++notifications; });
    showEditableDelay(browser, delay);
    QtBrowserItem* item = findExpressionProperty(browser.topLevelItems());
    ASSERT_NE(item, nullptr);
    browser.setCurrentItem(item);
    bool opened = false;
    QTimer::singleShot(0, [&]() {
        auto* dialog = qobject_cast<ExpressionBuilder*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        opened = true;
        dialog->findChild<QPlainTextEdit*>(QStringLiteral("expressionEditor"))
            ->setPlainText(QStringLiteral("norm(2,-1) + rnd"));
        dialog->findChild<QPushButton*>(QStringLiteral("btnApply"))->click();
    });
    QTimer::singleShot(1000, &browser, []() {
        if (auto* dialog = QApplication::activeModalWidget()) dialog->close();
    });
    openSelectedExpressionEditor(browser);
    for (int i = 0; i < 8; ++i) QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_TRUE(opened);
    EXPECT_EQ(delay.delayExpression(), "norm(2,-1) + rnd");
    EXPECT_EQ(notifications, 1);
}

TEST(ExpressionBuilderPropertyTest, CommittedExpressionSurvivesSaveAndReload) {
    Simulator sourceSimulator;
    sourceSimulator.getPluginManager()->autoInsertPlugins();
    Model* model = sourceSimulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* delay = new Delay(model, "ExpressionBuilderPersistedDelay");
    delay->setDelayTimeUnit(Util::TimeUnit::minute);
    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();
    showEditableDelay(browser, *delay);

    QtBrowserItem* property = findExpressionProperty(browser.topLevelItems());
    ASSERT_NE(property, nullptr);
    browser.setCurrentItem(property);
    bool accepted = false;
    QTimer::singleShot(0, [&]() {
        auto* dialog = qobject_cast<ExpressionBuilder*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        dialog->findChild<QPlainTextEdit*>(QStringLiteral("expressionEditor"))
            ->setPlainText(QStringLiteral("norm(2,-1) + rnd"));
        EXPECT_TRUE(dialog->validateCurrentExpression());
        dialog->findChild<QPushButton*>(QStringLiteral("btnApply"))->click();
        accepted = dialog->result() == QDialog::Accepted;
    });
    QTimer::singleShot(1000, &browser, []() {
        if (auto* dialog = QApplication::activeModalWidget()) dialog->close();
    });
    openSelectedExpressionEditor(browser);
    for (int i = 0; i < 8; ++i) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }

    const std::string expected = "norm(2,-1) + rnd";
    EXPECT_TRUE(accepted);
    ASSERT_EQ(delay->delayExpression(), expected);
    ASSERT_EQ(delay->delayTimeUnit(), Util::TimeUnit::minute);

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const std::string path = directory.filePath(QStringLiteral("expression-builder.gen")).toStdString();
    ASSERT_TRUE(model->save(path));

    Simulator loadedSimulator;
    loadedSimulator.getPluginManager()->autoInsertPlugins();
    Model* loadedModel = loadedSimulator.getModelManager()->loadModel(path);
    ASSERT_NE(loadedModel, nullptr);
    auto* restored = dynamic_cast<Delay*>(
        loadedModel->getComponentManager()->find("ExpressionBuilderPersistedDelay"));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->delayExpression(), expected);
    EXPECT_EQ(restored->delayTimeUnit(), Util::TimeUnit::minute);
}
