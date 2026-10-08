#include <gtest/gtest.h>

#include "propertyeditor/ObjectPropertyBrowser.h"
#include "services/ModelLanguageSynchronizer.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BacteriaSignalGrid.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/GroProgram.h"

#include <functional>

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QFile>
#include <QKeyEvent>
#include <QLineEdit>
#include <QLocale>
#include <QPlainTextEdit>
#include <QSet>
#include <QSpinBox>
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

    BioNetwork* network =
        manager->newInstance<BioNetwork>(model, "BioNetwork_BoolRegression");
    ASSERT_NE(network, nullptr);
    network->setAutoSchedule(false);
    bindEditableKernelObject(browser, network, &modelChangedCount);

    QtBrowserItem* autoSchedule =
        findBrowserItemByName(browser.topLevelItems(), QStringLiteral("AutoSchedule"));
    ASSERT_NE(autoSchedule, nullptr);
    browser.setCurrentItem(autoSchedule);
    browser.editItem(autoSchedule);
    drainGuiEvents();
    QCheckBox* checkBox = browser.findChild<QCheckBox*>();
    ASSERT_NE(checkBox, nullptr);
    checkBox->click();
    drainGuiEvents();
    EXPECT_TRUE(network->getAutoSchedule());
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
