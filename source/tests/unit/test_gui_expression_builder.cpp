#include <gtest/gtest.h>
#include <QSet>
#include <QString>

#include "applications/gui/genesys/tools/expressionbuilder/ParserFunctionCatalog.h"
#include "parser/Genesys++-driver.h"
#include "applications/gui/genesys/tools/expressionbuilder/ExpressionBuilder.h"
#include "applications/gui/genesys/propertyeditor/ObjectPropertyBrowser.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/components/DiscreteProcessing/Delay.h"

#include <QApplication>
#include <QCoreApplication>
#include <QEventLoop>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>

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
        if (control->getPropertyName() == "DelayExpression") {
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
