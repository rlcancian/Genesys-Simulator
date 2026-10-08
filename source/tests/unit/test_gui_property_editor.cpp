#include <gtest/gtest.h>

#include "propertyeditor/ObjectPropertyBrowser.h"
#include "services/ModelLanguageSynchronizer.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/GroProgram.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QFile>
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QPlainTextEdit>
#include <QSet>
#include <QString>

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

} // namespace

TEST(PropertyEditorDoubleCommit, PortugueseLocaleEnterCommitsNumericVariant) {
    ScopedDefaultLocale locale(QLocale(QLocale::Portuguese, QLocale::Brazil));

    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BioNetwork* network = manager->newInstance<BioNetwork>(model, "BioNetwork_PropertyEditor");
    ASSERT_NE(network, nullptr);
    network->setStartTime(0.0);

    ObjectPropertyBrowser browser;
    browser.resize(640, 480);
    browser.show();

    QSet<QString> editableObjects;
    editableObjects.insert(QString::fromStdString(network->getName()));
    browser.setActiveObject(
        nullptr,
        network,
        {},
        editableObjects,
        nullptr,
        nullptr,
        nullptr,
        nullptr);
    drainGuiEvents();

    QtBrowserItem* startTimeItem = findBrowserItemByName(browser.topLevelItems(), QStringLiteral("StartTime"));
    ASSERT_NE(startTimeItem, nullptr);
    browser.setCurrentItem(startTimeItem);
    browser.editItem(startTimeItem);
    drainGuiEvents();

    auto* spinBox = browser.findChild<QDoubleSpinBox*>();
    ASSERT_NE(spinBox, nullptr);
    auto* lineEdit = spinBox->findChild<QLineEdit*>();
    ASSERT_NE(lineEdit, nullptr);

    lineEdit->setFocus();
    lineEdit->selectAll();
    lineEdit->setText(QStringLiteral("0,18"));

    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(lineEdit, &press);
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(lineEdit, &release);
    drainGuiEvents();

    EXPECT_NEAR(network->getStartTime(), 0.18, 1e-12);

    // The deferred Property Editor rebuild must not revert the committed kernel value.
    drainGuiEvents();
    EXPECT_NEAR(network->getStartTime(), 0.18, 1e-12);
}


TEST(ModelLanguageSynchronizerRegression, RefreshIsSignalSafeCommentFilteredAndKeepsGroPayload) {
    QFile::remove(QStringLiteral("./temp.tmp"));

    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

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
