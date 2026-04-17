#include "DialogUtilityController.h"

#include "../mainwindow.h"
#include "ui_mainwindow.h"
#include "../dialogs/DialogFind.h"
#include "../dialogs/dialogBreakpoint.h"
#include "../dialogs/dialogpluginmanager.h"
#include "../dialogs/dialogsystempreferences.h"
#include "../graphicals/ModelGraphicsView.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../tools/dataanalyzer/DataAnalyzerWindow.h"
#include "../tools/optimizer/OptimizerWindow.h"

#include "../../../../../kernel/simulator/Simulator.h"
#include "../../../../../kernel/simulator/Model.h"
#include "../../../../../kernel/simulator/ModelManager.h"
#include "../../../../../kernel/simulator/ModelSimulation.h"
#include "../../../../../kernel/simulator/ModelDataDefinition.h"
#include "../../../../../kernel/simulator/ModelDataManager.h"
#include "../../../../../kernel/simulator/ModelComponent.h"
#include "../../../../../kernel/simulator/SimulationControlAndResponse.h"
#include "../../../../../kernel/simulator/LicenceManager.h"
#include "../../../../../kernel/simulator/Entity.h"
#include "../../../../../tools/FitterDefaultImpl.h"
#include "../../../../../tools/HypothesisTesterDefaultImpl1.h"
#include "../../../../../tools/OptimizerDefaultImpl1.h"
#include "../../../../../tools/SolverDefaultImpl1.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QSize>
#include <QPixmap>
#include <QPointF>
#include <QModelIndex>
#include <QDir>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QSet>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QTemporaryFile>
#include <QTextCursor>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <QVector>
#include <Qt>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>

namespace {
struct DataSummary {
    int count = 0;
    double min = 0.0;
    double max = 0.0;
    double mean = 0.0;
    double variance = 0.0;
    double stddev = 0.0;
    double cv = 0.0;
};

struct ParserFunctionInfo {
    QString category;
    QString name;
    QString syntax;
    QString description;
    QString example;
};

struct ParserGrammarNodeInfo {
    QString id;
    QString label;
    QPointF position;
    QString parentId;
};

static QString formatNumber(double value) {
    if (!std::isfinite(value)) {
        return QObject::tr("n/a");
    }
    return QString::number(value, 'g', 8);
}

static QTableWidgetItem* readOnlyItem(const QString& text) {
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

static QTableWidgetItem* checkableItem(bool checked) {
    QTableWidgetItem* item = readOnlyItem(QString());
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    return item;
}

static DataSummary summarizeValues(const QList<double>& values) {
    DataSummary summary;
    summary.count = values.size();
    if (values.isEmpty()) {
        return summary;
    }

    summary.min = values.first();
    summary.max = values.first();
    double sum = 0.0;
    for (double value : values) {
        summary.min = std::min(summary.min, value);
        summary.max = std::max(summary.max, value);
        sum += value;
    }

    summary.mean = sum / static_cast<double>(values.size());
    double squaredDiffSum = 0.0;
    for (double value : values) {
        const double diff = value - summary.mean;
        squaredDiffSum += diff * diff;
    }
    summary.variance = values.size() > 1 ? squaredDiffSum / static_cast<double>(values.size() - 1) : 0.0;
    summary.stddev = std::sqrt(summary.variance);
    summary.cv = summary.mean != 0.0 ? summary.stddev / summary.mean : 0.0;
    return summary;
}

static bool parseNumericDataset(const QString& fileName, QList<double>* numericValues, QStringList* previewLines, QString* errorMessage) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage != nullptr) {
            *errorMessage = QObject::tr("Could not open selected file.");
        }
        return false;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (previewLines != nullptr && previewLines->size() < 10) {
            *previewLines << line;
        }
        const QStringList tokens = line.split(QRegularExpression("[,;\\s]+"), Qt::SkipEmptyParts);
        for (const QString& token : tokens) {
            bool ok = false;
            const double value = token.toDouble(&ok);
            if (ok && std::isfinite(value)) {
                numericValues->append(value);
            }
        }
    }
    return true;
}

static QVector<ParserFunctionInfo> parserFunctionCatalog() {
    return {
        {QObject::tr("Arithmetic"), QStringLiteral("round"), QStringLiteral("round(expression)"), QObject::tr("Rounds a numeric expression to the nearest integer."), QStringLiteral("round(3.7)")},
        {QObject::tr("Arithmetic"), QStringLiteral("trunc"), QStringLiteral("trunc(expression)"), QObject::tr("Removes the fractional part of a numeric expression."), QStringLiteral("trunc(3.7)")},
        {QObject::tr("Arithmetic"), QStringLiteral("frac"), QStringLiteral("frac(expression)"), QObject::tr("Returns only the fractional part of a numeric expression."), QStringLiteral("frac(3.7)")},
        {QObject::tr("Arithmetic"), QStringLiteral("mod"), QStringLiteral("mod(expression, expression)"), QObject::tr("Integer remainder of the first expression divided by the second."), QStringLiteral("mod(17, 5)")},
        {QObject::tr("Arithmetic"), QStringLiteral("min"), QStringLiteral("min(expression, expression)"), QObject::tr("Returns the smaller of two numeric expressions."), QStringLiteral("min(10, 4)")},
        {QObject::tr("Arithmetic"), QStringLiteral("max"), QStringLiteral("max(expression, expression)"), QObject::tr("Returns the larger of two numeric expressions."), QStringLiteral("max(10, 4)")},
        {QObject::tr("Math"), QStringLiteral("exp"), QStringLiteral("exp(expression)"), QObject::tr("Natural exponential function."), QStringLiteral("exp(1)")},
        {QObject::tr("Math"), QStringLiteral("sqrt"), QStringLiteral("sqrt(expression)"), QObject::tr("Square root of a numeric expression."), QStringLiteral("sqrt(16)")},
        {QObject::tr("Math"), QStringLiteral("log"), QStringLiteral("log(expression)"), QObject::tr("Base-10 logarithm."), QStringLiteral("log(100)")},
        {QObject::tr("Math"), QStringLiteral("ln"), QStringLiteral("ln(expression)"), QObject::tr("Natural logarithm."), QStringLiteral("ln(exp(1))")},
        {QObject::tr("Trigonometry"), QStringLiteral("sin"), QStringLiteral("sin(expression)"), QObject::tr("Sine of an angle in radians."), QStringLiteral("sin(0)")},
        {QObject::tr("Trigonometry"), QStringLiteral("cos"), QStringLiteral("cos(expression)"), QObject::tr("Cosine of an angle in radians."), QStringLiteral("cos(0)")},
        {QObject::tr("Logic"), QStringLiteral("if"), QStringLiteral("if(condition, trueExpression, falseExpression)"), QObject::tr("Evaluates one expression when condition is non-zero and another when condition is zero."), QStringLiteral("if(2 > 1, 10, 20)")},
        {QObject::tr("Logic"), QStringLiteral("if"), QStringLiteral("if(condition, trueExpression)"), QObject::tr("Returns the true expression when condition is non-zero, otherwise returns zero."), QStringLiteral("if(1, 42)")},
        {QObject::tr("Logic"), QStringLiteral("operators"), QStringLiteral("<, >, <=, >=, ==, <>, !=, and, or, nand, xor, not"), QObject::tr("Relational and logical operators return numeric booleans, 1 for true and 0 for false."), QStringLiteral("(2 <= 3) and not false")},
        {QObject::tr("Random"), QStringLiteral("rnd"), QStringLiteral("rnd"), QObject::tr("Uniform random value in [0, 1]."), QStringLiteral("rnd")},
        {QObject::tr("Random"), QStringLiteral("expo"), QStringLiteral("expo(mean)"), QObject::tr("Exponential random variate using the configured sampler."), QStringLiteral("expo(5)")},
        {QObject::tr("Random"), QStringLiteral("norm"), QStringLiteral("norm(mean, stddev)"), QObject::tr("Normal random variate."), QStringLiteral("norm(10, 2)")},
        {QObject::tr("Random"), QStringLiteral("unif"), QStringLiteral("unif(min, max)"), QObject::tr("Uniform random variate between min and max."), QStringLiteral("unif(1, 10)")},
        {QObject::tr("Random"), QStringLiteral("weib"), QStringLiteral("weib(shape, scale)"), QObject::tr("Weibull random variate."), QStringLiteral("weib(2, 10)")},
        {QObject::tr("Random"), QStringLiteral("logn"), QStringLiteral("logn(mean, stddev)"), QObject::tr("Lognormal random variate."), QStringLiteral("logn(1, 0.25)")},
        {QObject::tr("Random"), QStringLiteral("gamm"), QStringLiteral("gamm(shape, scale)"), QObject::tr("Gamma random variate."), QStringLiteral("gamm(2, 3)")},
        {QObject::tr("Random"), QStringLiteral("erla"), QStringLiteral("erla(mean, m)"), QObject::tr("Erlang random variate."), QStringLiteral("erla(8, 2)")},
        {QObject::tr("Random"), QStringLiteral("tria"), QStringLiteral("tria(min, mode, max)"), QObject::tr("Triangular random variate."), QStringLiteral("tria(1, 3, 8)")},
        {QObject::tr("Random"), QStringLiteral("beta"), QStringLiteral("beta(alpha, beta, infLimit, supLimit)"), QObject::tr("Scaled beta random variate."), QStringLiteral("beta(2, 5, 0, 1)")},
        {QObject::tr("Simulation"), QStringLiteral("tnow"), QStringLiteral("tnow"), QObject::tr("Current simulated time."), QStringLiteral("tnow")},
        {QObject::tr("Simulation"), QStringLiteral("tfin"), QStringLiteral("tfin"), QObject::tr("Configured replication length."), QStringLiteral("tfin")},
        {QObject::tr("Simulation"), QStringLiteral("maxrep"), QStringLiteral("maxrep"), QObject::tr("Configured number of replications."), QStringLiteral("maxrep")},
        {QObject::tr("Simulation"), QStringLiteral("numrep"), QStringLiteral("numrep"), QObject::tr("Current replication number."), QStringLiteral("numrep")},
        {QObject::tr("Simulation"), QStringLiteral("ident"), QStringLiteral("ident"), QObject::tr("Current entity id, when an event/entity context exists."), QStringLiteral("ident")},
        {QObject::tr("Simulation"), QStringLiteral("entitieswip"), QStringLiteral("entitieswip"), QObject::tr("Number of entity data definitions in the model."), QStringLiteral("entitieswip")},
        {QObject::tr("Collectors"), QStringLiteral("tavg"), QStringLiteral("tavg(statisticsCollector)"), QObject::tr("Time-average value of a StatisticsCollector."), QStringLiteral("tavg(MyCollector)")},
        {QObject::tr("Collectors"), QStringLiteral("count"), QStringLiteral("count(counter)"), QObject::tr("Current value of a Counter."), QStringLiteral("count(MyCounter)")},
        {QObject::tr("Queue"), QStringLiteral("nq"), QStringLiteral("nq(queue)"), QObject::tr("Number of entities waiting in a queue."), QStringLiteral("nq(MyQueue)")},
        {QObject::tr("Queue"), QStringLiteral("firstinq"), QStringLiteral("firstinq(queue)"), QObject::tr("Id of the first entity waiting in a queue, or zero when empty."), QStringLiteral("firstinq(MyQueue)")},
        {QObject::tr("Queue"), QStringLiteral("saque"), QStringLiteral("saque(queue, attribute)"), QObject::tr("Sum of an attribute over entities waiting in a queue."), QStringLiteral("saque(MyQueue, MyAttribute)")},
        {QObject::tr("Queue"), QStringLiteral("aque"), QStringLiteral("aque(queue, rank, attribute)"), QObject::tr("Attribute value of an entity at a queue rank."), QStringLiteral("aque(MyQueue, 1, MyAttribute)")},
        {QObject::tr("Resource"), QStringLiteral("mr"), QStringLiteral("mr(resource)"), QObject::tr("Resource capacity."), QStringLiteral("mr(MyResource)")},
        {QObject::tr("Resource"), QStringLiteral("nr"), QStringLiteral("nr(resource)"), QObject::tr("Number of busy units in a resource."), QStringLiteral("nr(MyResource)")},
        {QObject::tr("Resource"), QStringLiteral("state"), QStringLiteral("state(resource)"), QObject::tr("Numeric state of a resource."), QStringLiteral("state(MyResource)")},
        {QObject::tr("Resource"), QStringLiteral("irf"), QStringLiteral("irf(resource)"), QObject::tr("Returns 1 when a resource is failed, otherwise 0."), QStringLiteral("irf(MyResource)")},
        {QObject::tr("Resource"), QStringLiteral("setsum"), QStringLiteral("setsum(set)"), QObject::tr("Counts busy resources in a set."), QStringLiteral("setsum(MySet)")},
        {QObject::tr("Set"), QStringLiteral("numset"), QStringLiteral("numset(set)"), QObject::tr("Number of model data definitions in a set."), QStringLiteral("numset(MySet)")},
        {QObject::tr("Entity Group"), QStringLiteral("numgr"), QStringLiteral("numgr(group)"), QObject::tr("Entity group function token recognized by the lexer."), QStringLiteral("numgr(MyGroup)")},
        {QObject::tr("Entity Group"), QStringLiteral("atrgr"), QStringLiteral("atrgr(group, attribute)"), QObject::tr("Entity group attribute function token recognized by the lexer."), QStringLiteral("atrgr(MyGroup, MyAttribute)")}
    };
}

static QString parserDotText() {
    return QStringLiteral(
        "digraph GenesysParser {\n"
        "  rankdir=LR;\n"
        "  node [shape=box, style=\"rounded,filled\", fillcolor=\"#f7f9fb\", color=\"#78909c\"];\n"
        "  input -> expression;\n"
        "  expression -> assignment;\n"
        "  expression -> command;\n"
        "  expression -> logical_or;\n"
        "  logical_or -> logical_xor -> logical_and -> logical_not -> relational;\n"
        "  relational -> additive -> multiplicative -> power -> unary -> primary;\n"
        "  primary -> number;\n"
        "  primary -> function;\n"
        "  primary -> \"( expression )\";\n"
        "  primary -> attribute;\n"
        "  primary -> simulation_response;\n"
        "  primary -> simulation_control;\n"
        "  primary -> variable;\n"
        "  primary -> formula;\n"
        "  function -> math_function;\n"
        "  function -> trigon_function;\n"
        "  function -> probability_function;\n"
        "  function -> kernel_function;\n"
        "  function -> element_function;\n"
        "  function -> plugin_function;\n"
        "  command -> if_command;\n"
        "  command -> for_command;\n"
        "  assignment -> attribute_assignment;\n"
        "  assignment -> variable_assignment;\n"
        "}\n");
}

static QString parserGrammarSummary() {
    return QObject::tr(
        "The Genesys expression grammar is compiled from Bison/Flex sources.\n\n"
        "Recognized structure:\n"
        "- expressions include assignment, commands, logical expressions, arithmetic expressions, functions, model symbols, and literals.\n"
        "- precedence flows from logical OR down to XOR, AND/NAND, NOT, relational operators, addition/subtraction, multiplication/division, power, unary signs, and primary values.\n"
        "- primary values include numbers, parenthesized expressions, functions, attributes, variables, formulas, simulation responses, and simulation controls.\n"
        "- commands currently include if(...) and a limited for...to...do assignment form.\n"
        "- parser symbols that depend on model context are resolved through the current ModelDataManager, SimulationResponse list, and SimulationControl list.\n\n"
        "DOT export:\n"
        "The DOT text in this tab is a simplified graph generated from the grammar structure. A future build step can ask Bison to emit an automaton report/DOT and load it here.");
}

class ParserConsoleEdit : public QPlainTextEdit {
public:
    explicit ParserConsoleEdit(QWidget* parent = nullptr)
        : QPlainTextEdit(parent) {
    }

    void setReturnHandler(std::function<void()> handler) {
        _returnHandler = std::move(handler);
    }

protected:
    void keyPressEvent(QKeyEvent* event) override {
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            && !(event->modifiers() & Qt::ShiftModifier)) {
            if (_returnHandler) {
                _returnHandler();
            }
            event->accept();
            return;
        }
        QPlainTextEdit::keyPressEvent(event);
    }

private:
    std::function<void()> _returnHandler;
};

class OptimizationProgressWidget : public QWidget {
public:
    explicit OptimizationProgressWidget(QWidget* parent = nullptr)
        : QWidget(parent) {
        setMinimumHeight(150);
    }

    void appendValue(double value) {
        _values.append(value);
        update();
    }

    void reset() {
        _values.clear();
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor(250, 251, 252));
        const QRect plotRect = rect().adjusted(34, 16, -16, -28);
        painter.setPen(QPen(QColor(190, 198, 206)));
        painter.drawRect(plotRect);
        painter.setPen(QColor(96, 112, 128));
        painter.drawText(rect().adjusted(8, 0, -8, -6), Qt::AlignBottom | Qt::AlignLeft,
                         QObject::tr("Objective trend preview"));

        if (_values.size() < 2) {
            painter.setPen(QColor(120, 130, 140));
            painter.drawText(plotRect, Qt::AlignCenter, QObject::tr("Start or step the optimizer to plot progress."));
            return;
        }

        const auto minmax = std::minmax_element(_values.begin(), _values.end());
        const double minValue = *minmax.first;
        const double maxValue = *minmax.second;
        const double range = std::max(1.0, maxValue - minValue);

        QPainterPath path;
        for (int i = 0; i < _values.size(); ++i) {
            const double x = plotRect.left() + (plotRect.width() * static_cast<double>(i) / static_cast<double>(_values.size() - 1));
            const double normalized = (_values.at(i) - minValue) / range;
            const double y = plotRect.bottom() - normalized * plotRect.height();
            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }
        painter.setPen(QPen(QColor(0, 105, 92), 2));
        painter.drawPath(path);
    }

private:
    QVector<double> _values;
};

static QString aboutDialogStyleSheet() {
    return QStringLiteral(
        "QDialog { background: #f6f7f9; }"
        "QFrame#heroFrame {"
        "  border: 0;"
        "  border-radius: 8px;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                              stop:0 #1b5e20, stop:1 #00897b);"
        "}"
        "QLabel#heroTitle { color: white; font-size: 26px; font-weight: 700; }"
        "QLabel#heroSubtitle { color: #e8f5e9; font-size: 13px; }"
        "QLabel#sectionTitle { color: #263238; font-size: 15px; font-weight: 700; }"
        "QTextBrowser {"
        "  border: 1px solid #d7dde3;"
        "  border-radius: 8px;"
        "  background: white;"
        "  padding: 10px;"
        "}"
        "QPushButton { min-width: 88px; min-height: 28px; border-radius: 6px; padding: 4px 12px; }");
}

static QString bodyCss() {
    return QStringLiteral(
        "<style>"
        "body { font-family: sans-serif; color: #263238; font-size: 10pt; line-height: 1.36; }"
        "h2 { margin: 0 0 8px 0; font-size: 17pt; color: #1b5e20; }"
        "h3 { margin: 16px 0 6px 0; font-size: 12pt; color: #37474f; }"
        "p { margin: 8px 0; }"
        "ul { margin: 6px 0 8px 20px; padding: 0; }"
        "li { margin: 5px 0; }"
        "a { color: #00695c; text-decoration: none; font-weight: 600; }"
        ".pill { display: inline-block; color: #1b5e20; background: #e8f5e9; padding: 2px 7px; }"
        ".muted { color: #607d8b; }"
        "</style>");
}

static QLabel* createHeroIcon(QWidget* parent) {
    QLabel* icon = new QLabel(parent);
    QPixmap pixmap(QStringLiteral(":/resources/icons/genesysico.gif"));
    if (!pixmap.isNull()) {
        icon->setPixmap(pixmap.scaled(68, 68, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    icon->setFixedSize(76, 76);
    icon->setAlignment(Qt::AlignCenter);
    return icon;
}

static void showRichAboutDialog(QWidget* owner,
                                const QString& windowTitle,
                                const QString& heroTitle,
                                const QString& heroSubtitle,
                                const QString& htmlBody,
                                const QSize& size = QSize(680, 560)) {
    QDialog dialog(owner);
    dialog.setWindowTitle(windowTitle);
    dialog.setModal(true);
    dialog.resize(size);
    dialog.setStyleSheet(aboutDialogStyleSheet());

    QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setContentsMargins(18, 18, 18, 18);
    dialogLayout->setSpacing(14);

    QFrame* hero = new QFrame(&dialog);
    hero->setObjectName(QStringLiteral("heroFrame"));
    QHBoxLayout* heroLayout = new QHBoxLayout(hero);
    heroLayout->setContentsMargins(18, 16, 18, 16);
    heroLayout->setSpacing(16);
    heroLayout->addWidget(createHeroIcon(hero), 0, Qt::AlignTop);

    QVBoxLayout* heroTextLayout = new QVBoxLayout();
    QLabel* title = new QLabel(heroTitle, hero);
    title->setObjectName(QStringLiteral("heroTitle"));
    QLabel* subtitle = new QLabel(heroSubtitle, hero);
    subtitle->setObjectName(QStringLiteral("heroSubtitle"));
    subtitle->setWordWrap(true);
    heroTextLayout->addWidget(title);
    heroTextLayout->addWidget(subtitle);
    heroTextLayout->addStretch();
    heroLayout->addLayout(heroTextLayout, 1);
    dialogLayout->addWidget(hero);

    QTextBrowser* body = new QTextBrowser(&dialog);
    body->setOpenExternalLinks(true);
    body->setHtml(bodyCss() + htmlBody);
    dialogLayout->addWidget(body, 1);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialogLayout->addWidget(buttons);

    dialog.exec();
}
} // namespace

// Initialize the Phase 11 controller with narrow dependencies and persisted GUI state references.
DialogUtilityController::DialogUtilityController(MainWindow* ownerWidget,
                                                 Simulator* simulator,
                                                 Ui::MainWindow* ui,
                                                 ModelGraphicsView* graphicsView,
                                                 std::function<void()> showMessageNotImplemented,
                                                 std::function<void(bool)> actualizeDebugBreakpoints,
                                                 std::function<bool()> createModelImage,
                                                 std::function<void()> actualizeActions,
                                                 std::function<void()> actualizeTabPanes,
                                                 std::function<void()> reloadPluginCatalog,
                                                 std::function<ModelGraphicsScene*()> currentScene,
                                                 double& optimizerPrecision,
                                                 unsigned int& optimizerMaxSteps,
                                                 bool& parallelizationEnabled,
                                                 int& parallelizationThreads,
                                                 int& parallelizationBatchSize,
                                                 QString& lastDataAnalyzerPath)
    : _ownerWidget(ownerWidget),
      _simulator(simulator),
      _ui(ui),
      _graphicsView(graphicsView),
      _showMessageNotImplemented(std::move(showMessageNotImplemented)),
      _actualizeDebugBreakpoints(std::move(actualizeDebugBreakpoints)),
      _createModelImage(std::move(createModelImage)),
      _actualizeActions(std::move(actualizeActions)),
      _actualizeTabPanes(std::move(actualizeTabPanes)),
      _reloadPluginCatalog(std::move(reloadPluginCatalog)),
      _currentScene(std::move(currentScene)),
      _optimizerPrecision(optimizerPrecision),
      _optimizerMaxSteps(optimizerMaxSteps),
      _parallelizationEnabled(parallelizationEnabled),
      _parallelizationThreads(parallelizationThreads),
      _parallelizationBatchSize(parallelizationBatchSize),
      _lastDataAnalyzerPath(lastDataAnalyzerPath) {
}

// Preserve the existing About dialog text and parenting behavior.
void DialogUtilityController::onActionAboutAboutTriggered() {
    const QString html = QStringLiteral(
        "<h2>Discrete-event simulation for teaching, research, and experimentation</h2>"
        "<p><span class='pill'>Open source simulator</span></p>"
        "<p>Genesys is the result of teaching and research activities led by "
        "Professor Dr. Ing. Rafael Luiz Cancian. The project began in early 2002 "
        "as a way to help students learn systems simulation concepts and reproduce "
        "techniques available in commercial simulation environments such as Arena.</p>"
        "<p>During its first development cycle, Genesys replicated the SIMAN language "
        "used by Arena and grew into a graphical simulation environment with support "
        "for new simulation components, dynamic libraries, and distributed model execution.</p>"
        "<h3>A second life</h3>"
        "<p>Development paused after 2009, when simulation classes were no longer being "
        "taught regularly. In 2019, with new teaching and research activity in the area, "
        "Genesys was reborn with updated programming techniques and more ambitious goals.</p>"
        "<h3>Author</h3>"
        "<p><b>Rafael Luiz Cancian</b><br>"
        "<span class='muted'>Departamento de Informatica e Estatistica, UFSC</span><br>"
        "<a href='https://ine.ufsc.br/rafael.cancian'>ine.ufsc.br/rafael.cancian</a></p>");

    showRichAboutDialog(_ownerWidget,
                        QObject::tr("About Genesys"),
                        QObject::tr("Genesys"),
                        QObject::tr("A simulation environment for learning, modeling, and research."),
                        html);
}

// Preserve the existing license dialog flow and text composition.
void DialogUtilityController::onActionAboutLicenceTriggered() {
    LicenceManager* licman = _simulator->getLicenceManager();
    std::string text = licman->showLicence() + "\n";
    text += licman->showLimits() + "\n";
    text += licman->showActivationCode();
    QMessageBox::about(_ownerWidget, "About Licence", QString::fromStdString(text));
}

// Preserve the existing get-involved message content and parent window.
void DialogUtilityController::onActionAboutGetInvolvedTriggered() {
    const QString html = QStringLiteral(
        "<h2>Help improve Genesys</h2>"
        "<p>Genesys is a free open-source simulator and tool set. Contributions can be small, "
        "practical, and focused: examples, bug reports, documentation, tests, models, and code "
        "changes all help the project move forward.</p>"
        "<h3>Good ways to contribute</h3>"
        "<ul>"
        "<li><b>Report bugs:</b> include the model file, steps to reproduce, expected behavior, and actual behavior.</li>"
        "<li><b>Improve examples:</b> add teaching models, validation scenarios, and small reproducible simulations.</li>"
        "<li><b>Write documentation:</b> clarify components, simulation controls, reports, and common workflows.</li>"
        "<li><b>Contribute code:</b> fix issues, improve the GUI, add tests, or refine simulation components.</li>"
        "<li><b>Review and test:</b> run sample models, compare results, and report regressions.</li>"
        "<li><b>Use it in class or research:</b> share feedback from students, researchers, and real modeling use cases.</li>"
        "</ul>"
        "<h3>Project links</h3>"
        "<ul>"
        "<li><a href='https://github.com/rlcancian/Genesys-Simulator'>GitHub repository</a></li>"
        "<li><a href='https://github.com/rlcancian/Genesys-Simulator/issues'>Issues and bug reports</a></li>"
        "<li><a href='https://github.com/rlcancian/Genesys-Simulator/pulls'>Pull requests</a></li>"
        "<li><a href='https://ine.ufsc.br/rafael.cancian'>Rafael Cancian professional page</a></li>"
        "</ul>"
        "<h3>Contact</h3>"
        "<p>For academic collaboration or project questions, contact "
        "<a href='mailto:rafael.cancian@ufsc.br'>rafael.cancian@ufsc.br</a>.</p>");

    showRichAboutDialog(_ownerWidget,
                        QObject::tr("Get Involved"),
                        QObject::tr("Get Involved"),
                        QObject::tr("Contribute models, documentation, tests, code, and research feedback."),
                        html);
}

// Preserve the existing find dialog workflow using the current scene.
void DialogUtilityController::onActionEditFindTriggered() {
    DialogFind* find = new DialogFind(_ownerWidget, _graphicsView->getScene());
    find->show();
    if (find->exec() == QDialog::Accepted) {
        find->setFocus();
    }
}

// Preserve the existing replace dialog and selection-based replace workflow.
void DialogUtilityController::onActionEditReplaceTriggered() {
    if (_ui->TextCodeEditor == nullptr) {
        return;
    }

    auto* editor = _ui->TextCodeEditor;

    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Replace"));
    dialog.setModal(false);

    auto* layout = new QVBoxLayout(&dialog);
    auto* formLayout = new QFormLayout();
    auto* findLine = new QLineEdit(&dialog);
    auto* replaceLine = new QLineEdit(&dialog);
    auto* caseSensitive = new QCheckBox(QObject::tr("Case sensitive"), &dialog);
    auto* statusLabel = new QLabel(&dialog);
    statusLabel->setWordWrap(true);
    statusLabel->setText(QObject::tr("Ready."));

    static QString lastFindText;
    static QString lastReplaceText;
    findLine->setText(lastFindText);
    replaceLine->setText(lastReplaceText);

    formLayout->addRow(QObject::tr("Find:"), findLine);
    formLayout->addRow(QObject::tr("Replace with:"), replaceLine);

    auto* buttonFindNext = new QPushButton(QObject::tr("Find Next"), &dialog);
    auto* buttonReplace = new QPushButton(QObject::tr("Replace"), &dialog);
    auto* buttonReplaceAll = new QPushButton(QObject::tr("Replace All"), &dialog);
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);

    auto* actionLayout = new QHBoxLayout();
    actionLayout->addWidget(buttonFindNext);
    actionLayout->addWidget(buttonReplace);
    actionLayout->addWidget(buttonReplaceAll);

    layout->addLayout(formLayout);
    layout->addWidget(caseSensitive);
    layout->addLayout(actionLayout);
    layout->addWidget(statusLabel);
    layout->addWidget(buttonBox);

    // Keep the same looping find behavior that wraps to the document start.
    auto findNext = [&]() -> bool {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(QObject::tr("Enter text to find."));
            return false;
        }

        QTextDocument::FindFlags flags;
        if (caseSensitive->isChecked()) {
            flags |= QTextDocument::FindCaseSensitively;
        }

        bool found = editor->find(findText, flags);
        if (!found) {
            QTextCursor cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::Start);
            editor->setTextCursor(cursor);
            found = editor->find(findText, flags);
        }

        if (found) {
            statusLabel->setText(QObject::tr("Occurrence selected."));
            lastFindText = findText;
            lastReplaceText = replaceLine->text();
            return true;
        }

        statusLabel->setText(QObject::tr("No occurrences found."));
        return false;
    };

    // Keep existing button wiring and status updates for find next.
    QObject::connect(buttonFindNext, &QPushButton::clicked, &dialog, [&]() {
        findNext();
    });

    // Keep existing replace-one behavior and immediate next search.
    QObject::connect(buttonReplace, &QPushButton::clicked, &dialog, [&]() {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(QObject::tr("Enter text to find."));
            return;
        }

        QTextCursor cursor = editor->textCursor();
        const bool matchesSelection = cursor.hasSelection() &&
                ((caseSensitive->isChecked() && cursor.selectedText() == findText) ||
                 (!caseSensitive->isChecked() && cursor.selectedText().compare(findText, Qt::CaseInsensitive) == 0));

        if (!matchesSelection && !findNext()) {
            return;
        }

        cursor = editor->textCursor();
        if (cursor.hasSelection()) {
            cursor.insertText(replaceLine->text());
            editor->setTextCursor(cursor);
            statusLabel->setText(QObject::tr("Occurrence replaced."));
            lastFindText = findText;
            lastReplaceText = replaceLine->text();
            findNext();
        }
    });

    // Keep existing replace-all scan semantics and edit block batching.
    QObject::connect(buttonReplaceAll, &QPushButton::clicked, &dialog, [&]() {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(QObject::tr("Enter text to find."));
            return;
        }

        QTextDocument::FindFlags flags;
        if (caseSensitive->isChecked()) {
            flags |= QTextDocument::FindCaseSensitively;
        }

        QTextCursor scanCursor(editor->document());
        scanCursor.movePosition(QTextCursor::Start);
        int replacements = 0;

        scanCursor.beginEditBlock();
        while (true) {
            QTextCursor found = editor->document()->find(findText, scanCursor, flags);
            if (found.isNull()) {
                break;
            }
            found.insertText(replaceLine->text());
            scanCursor = found;
            replacements++;
        }
        scanCursor.endEditBlock();

        editor->setFocus();
        lastFindText = findText;
        lastReplaceText = replaceLine->text();
        statusLabel->setText(QObject::tr("%1 occurrence(s) replaced.").arg(replacements));
    });

    // Keep existing close behavior and modal execution path.
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    findLine->setFocus();
    dialog.exec();
}

// Provide a parser management surface with grammar overview, function catalog and an expression console.
void DialogUtilityController::onActionToolsParserGrammarCheckerTriggered() {
    Model* model = _simulator->getModelManager()->current();
    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Parser Manager"));
    dialog.resize(980, 700);
    auto* layout = new QVBoxLayout(&dialog);
    auto* tabs = new QTabWidget(&dialog);
    layout->addWidget(tabs, 1);

    auto* overviewTab = new QWidget(&dialog);
    auto* overviewLayout = new QHBoxLayout(overviewTab);
    auto* grammarScene = new QGraphicsScene(overviewTab);
    auto* grammarView = new QGraphicsView(grammarScene, overviewTab);
    grammarView->setRenderHint(QPainter::Antialiasing, true);
    grammarView->setDragMode(QGraphicsView::ScrollHandDrag);
    grammarView->setMinimumWidth(560);

    const QVector<ParserGrammarNodeInfo> grammarNodes = {
        {QStringLiteral("input"), QStringLiteral("input"), QPointF(0, 300), QString()},
        {QStringLiteral("expression"), QStringLiteral("expression"), QPointF(180, 300), QStringLiteral("input")},
        {QStringLiteral("assignment"), QStringLiteral("assignment"), QPointF(390, 120), QStringLiteral("expression")},
        {QStringLiteral("command"), QStringLiteral("command"), QPointF(390, 260), QStringLiteral("expression")},
        {QStringLiteral("logical_or"), QStringLiteral("logical OR"), QPointF(390, 440), QStringLiteral("expression")},
        {QStringLiteral("attribute_assignment"), QStringLiteral("attribute = expr"), QPointF(620, 60), QStringLiteral("assignment")},
        {QStringLiteral("variable_assignment"), QStringLiteral("variable = expr"), QPointF(620, 120), QStringLiteral("assignment")},
        {QStringLiteral("if_command"), QStringLiteral("if(...)"), QPointF(620, 220), QStringLiteral("command")},
        {QStringLiteral("for_command"), QStringLiteral("for...to...do"), QPointF(620, 280), QStringLiteral("command")},
        {QStringLiteral("logical_xor"), QStringLiteral("logical XOR"), QPointF(620, 400), QStringLiteral("logical_or")},
        {QStringLiteral("logical_and"), QStringLiteral("AND / NAND"), QPointF(830, 400), QStringLiteral("logical_xor")},
        {QStringLiteral("logical_not"), QStringLiteral("NOT"), QPointF(1040, 400), QStringLiteral("logical_and")},
        {QStringLiteral("relational"), QStringLiteral("< <= == != >="), QPointF(1250, 400), QStringLiteral("logical_not")},
        {QStringLiteral("additive"), QStringLiteral("+ / -"), QPointF(1460, 400), QStringLiteral("relational")},
        {QStringLiteral("multiplicative"), QStringLiteral("* /"), QPointF(1670, 400), QStringLiteral("additive")},
        {QStringLiteral("power"), QStringLiteral("power"), QPointF(1880, 400), QStringLiteral("multiplicative")},
        {QStringLiteral("unary"), QStringLiteral("unary +/-"), QPointF(2090, 400), QStringLiteral("power")},
        {QStringLiteral("primary"), QStringLiteral("primary"), QPointF(2300, 400), QStringLiteral("unary")},
        {QStringLiteral("number"), QStringLiteral("number"), QPointF(2530, 220), QStringLiteral("primary")},
        {QStringLiteral("parentheses"), QStringLiteral("( expression )"), QPointF(2530, 280), QStringLiteral("primary")},
        {QStringLiteral("function"), QStringLiteral("function"), QPointF(2530, 400), QStringLiteral("primary")},
        {QStringLiteral("model_symbol"), QStringLiteral("model symbol"), QPointF(2530, 540), QStringLiteral("primary")},
        {QStringLiteral("math_function"), QStringLiteral("math / trig"), QPointF(2760, 280), QStringLiteral("function")},
        {QStringLiteral("logic_function"), QStringLiteral("logic"), QPointF(2760, 340), QStringLiteral("function")},
        {QStringLiteral("probability_function"), QStringLiteral("probability"), QPointF(2760, 400), QStringLiteral("function")},
        {QStringLiteral("simulation_function"), QStringLiteral("simulation"), QPointF(2760, 460), QStringLiteral("function")},
        {QStringLiteral("plugin_function"), QStringLiteral("queues / resources"), QPointF(2760, 520), QStringLiteral("function")},
        {QStringLiteral("attribute"), QStringLiteral("attribute"), QPointF(2760, 600), QStringLiteral("model_symbol")},
        {QStringLiteral("variable"), QStringLiteral("variable"), QPointF(2760, 660), QStringLiteral("model_symbol")},
        {QStringLiteral("formula"), QStringLiteral("formula"), QPointF(2760, 720), QStringLiteral("model_symbol")},
        {QStringLiteral("response"), QStringLiteral("simulation response"), QPointF(2760, 780), QStringLiteral("model_symbol")},
        {QStringLiteral("control"), QStringLiteral("simulation control"), QPointF(2760, 840), QStringLiteral("model_symbol")}
    };
    QMap<QString, QRectF> grammarRects;
    QMap<QString, QGraphicsRectItem*> grammarRectItems;
    QMap<QString, QGraphicsTextItem*> grammarTextItems;
    QMap<QString, QStringList> grammarChildren;
    QMap<QString, QGraphicsLineItem*> grammarEdgeItems;
    QSet<QString> collapsedGrammarNodes;
    for (const auto& node : grammarNodes) {
        const QRectF rect(node.position, QSizeF(170, 40));
        grammarRects.insert(node.id, rect);
        if (!node.parentId.isEmpty()) {
            grammarChildren[node.parentId].append(node.id);
        }
        QGraphicsRectItem* rectItem = grammarScene->addRect(rect, QPen(QColor(69, 90, 100)), QBrush(QColor(247, 249, 251)));
        rectItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        rectItem->setData(0, node.id);
        rectItem->setZValue(0);
        rectItem->setToolTip(QObject::tr("Click to expand or collapse this grammar branch."));
        grammarRectItems.insert(node.id, rectItem);
        QGraphicsTextItem* text = grammarScene->addText(node.label);
        text->setPos(rect.left() + 10, rect.top() + 10);
        text->setDefaultTextColor(QColor(38, 50, 56));
        text->setAcceptedMouseButtons(Qt::NoButton);
        text->setZValue(1);
        grammarTextItems.insert(node.id, text);
    }
    auto addEdge = [grammarScene, &grammarRects, &grammarEdgeItems](const QString& from, const QString& to) {
        const QPointF a(grammarRects.value(from).right(), grammarRects.value(from).center().y());
        const QPointF b(grammarRects.value(to).left(), grammarRects.value(to).center().y());
        QGraphicsLineItem* edge = grammarScene->addLine(QLineF(a, b), QPen(QColor(0, 105, 92), 2));
        edge->setZValue(-1);
        grammarEdgeItems.insert(to, edge);
    };
    for (const auto& node : grammarNodes) {
        if (!node.parentId.isEmpty()) {
            addEdge(node.parentId, node.id);
        }
    }
    auto setGrammarSubtreeVisible = [&](auto&& self, const QString& id, bool visible) -> void {
        if (grammarRectItems.contains(id)) {
            grammarRectItems.value(id)->setVisible(visible);
        }
        if (grammarTextItems.contains(id)) {
            grammarTextItems.value(id)->setVisible(visible);
        }
        if (grammarEdgeItems.contains(id)) {
            grammarEdgeItems.value(id)->setVisible(visible);
        }
        const bool showChildren = visible && !collapsedGrammarNodes.contains(id);
        for (const QString& child : grammarChildren.value(id)) {
            self(self, child, showChildren);
        }
    };
    auto updateGrammarLabels = [&]() {
        for (const ParserGrammarNodeInfo& node : grammarNodes) {
            QGraphicsTextItem* text = grammarTextItems.value(node.id, nullptr);
            if (text == nullptr) {
                continue;
            }
            const bool hasChildren = !grammarChildren.value(node.id).isEmpty();
            const QString prefix = hasChildren ? (collapsedGrammarNodes.contains(node.id) ? QStringLiteral("[+] ") : QStringLiteral("[-] ")) : QStringLiteral("    ");
            text->setPlainText(prefix + node.label);
        }
    };
    auto toggleGrammarNode = [&](const QString& id) {
        if (grammarChildren.value(id).isEmpty()) {
            return;
        }
        if (collapsedGrammarNodes.contains(id)) {
            collapsedGrammarNodes.remove(id);
        } else {
            collapsedGrammarNodes.insert(id);
        }
        updateGrammarLabels();
        setGrammarSubtreeVisible(setGrammarSubtreeVisible, QStringLiteral("input"), true);
    };
    updateGrammarLabels();
    grammarScene->setSceneRect(grammarScene->itemsBoundingRect().adjusted(-20, -20, 20, 20));

    auto* grammarSummary = new QTextEdit(overviewTab);
    grammarSummary->setReadOnly(true);
    grammarSummary->setPlainText(parserGrammarSummary());
    auto* grammarToolsLayout = new QVBoxLayout();
    auto* grammarButtons = new QHBoxLayout();
    auto* expandGrammarButton = new QPushButton(QObject::tr("Expand all"), overviewTab);
    auto* compactGrammarButton = new QPushButton(QObject::tr("Compact"), overviewTab);
    grammarButtons->addWidget(expandGrammarButton);
    grammarButtons->addWidget(compactGrammarButton);
    grammarButtons->addStretch();
    grammarToolsLayout->addLayout(grammarButtons);
    grammarToolsLayout->addWidget(grammarView, 1);
    overviewLayout->addLayout(grammarToolsLayout, 3);
    overviewLayout->addWidget(grammarSummary, 2);
    tabs->addTab(overviewTab, QObject::tr("Grammar Map"));

    auto* dotTab = new QWidget(&dialog);
    auto* dotLayout = new QVBoxLayout(dotTab);
    auto* dotText = new QPlainTextEdit(dotTab);
    dotText->setReadOnly(false);
    dotText->setPlainText(parserDotText());
    auto* saveDotButton = new QPushButton(QObject::tr("Save DOT..."), dotTab);
    dotLayout->addWidget(new QLabel(QObject::tr("Simplified DOT representation. This can be rendered by Graphviz or replaced later by a Bison-generated automaton graph."), dotTab));
    dotLayout->addWidget(dotText, 1);
    dotLayout->addWidget(saveDotButton, 0, Qt::AlignRight);
    tabs->addTab(dotTab, QObject::tr("DOT"));

    auto* functionsTab = new QWidget(&dialog);
    auto* functionsLayout = new QVBoxLayout(functionsTab);
    auto* functionFilter = new QLineEdit(functionsTab);
    functionFilter->setPlaceholderText(QObject::tr("Filter functions by name, category, syntax or description"));
    auto* functionsTable = new QTableWidget(functionsTab);
    functionsTable->setColumnCount(5);
    functionsTable->setHorizontalHeaderLabels({QObject::tr("Category"), QObject::tr("Name"), QObject::tr("Syntax"), QObject::tr("Description"), QObject::tr("Example")});
    const QVector<ParserFunctionInfo> functions = parserFunctionCatalog();
    functionsTable->setRowCount(functions.size());
    for (int row = 0; row < functions.size(); ++row) {
        const ParserFunctionInfo& info = functions.at(row);
        functionsTable->setItem(row, 0, readOnlyItem(info.category));
        functionsTable->setItem(row, 1, readOnlyItem(info.name));
        functionsTable->setItem(row, 2, readOnlyItem(info.syntax));
        functionsTable->setItem(row, 3, readOnlyItem(info.description));
        functionsTable->setItem(row, 4, readOnlyItem(info.example));
    }
    functionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    functionsTable->horizontalHeader()->setStretchLastSection(true);
    functionsTable->verticalHeader()->setVisible(false);
    functionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    functionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    auto* functionDetails = new QTextBrowser(functionsTab);
    functionDetails->setOpenExternalLinks(false);
    functionDetails->setMinimumHeight(110);
    auto* insertExampleButton = new QPushButton(QObject::tr("Insert Example in Console"), functionsTab);
    functionsLayout->addWidget(functionFilter);
    functionsLayout->addWidget(functionsTable, 1);
    functionsLayout->addWidget(functionDetails);
    functionsLayout->addWidget(insertExampleButton, 0, Qt::AlignRight);
    tabs->addTab(functionsTab, QObject::tr("Functions"));

    auto* syntaxTab = new QWidget(&dialog);
    auto* syntaxLayout = new QVBoxLayout(syntaxTab);
    auto* syntaxText = new QPlainTextEdit(syntaxTab);
    syntaxText->setReadOnly(true);
    QFile grammarFile(QStringLiteral("source/parser/parserBisonFlex/bisonparser.yy"));
    QFile lexerFile(QStringLiteral("source/parser/parserBisonFlex/lexerparser.ll"));
    QString syntaxSource = QObject::tr("Grammar source file: source/parser/parserBisonFlex/bisonparser.yy\nLexer source file: source/parser/parserBisonFlex/lexerparser.ll\n\n");
    if (grammarFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        syntaxSource += QObject::tr("=== Bison grammar excerpt ===\n");
        syntaxSource += QString::fromUtf8(grammarFile.readAll()).left(24000);
        grammarFile.close();
    }
    if (lexerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        syntaxSource += QObject::tr("\n\n=== Flex lexer excerpt ===\n");
        syntaxSource += QString::fromUtf8(lexerFile.readAll()).left(16000);
        lexerFile.close();
    }
    syntaxText->setPlainText(syntaxSource);
    syntaxLayout->addWidget(syntaxText);
    tabs->addTab(syntaxTab, QObject::tr("Bison/Flex Source"));

    auto* consoleTab = new QWidget(&dialog);
    auto* consoleLayout = new QVBoxLayout(consoleTab);
    auto* contextLabel = new QLabel(consoleTab);
    contextLabel->setWordWrap(true);
    contextLabel->setText(model != nullptr
        ? QObject::tr("Parser context: current model is available. Expressions can reference model data definitions, simulation responses and simulation controls.")
        : QObject::tr("Parser context: no current model is available. Open a model to evaluate expressions that require Genesys model symbols."));
    auto* console = new ParserConsoleEdit(consoleTab);
    console->setPlaceholderText(QObject::tr("Type an expression after the prompt. Press Enter to evaluate; use Shift+Enter for a new line."));
    QString selectedText;
    if (_ui->TextCodeEditor != nullptr) {
        selectedText = _ui->TextCodeEditor->textCursor().selectedText().trimmed();
    }
    console->setPlainText(selectedText.isEmpty()
        ? QStringLiteral("> 2 + 3 * 4")
        : QStringLiteral("> ") + selectedText);
    auto* consoleStatus = new QLabel(QObject::tr("Ready."), consoleTab);
    consoleStatus->setWordWrap(true);
    auto* consoleButtons = new QHBoxLayout();
    auto* evaluateButton = new QPushButton(QObject::tr("Evaluate"), consoleTab);
    auto* clearConsoleButton = new QPushButton(QObject::tr("Clear Console"), consoleTab);
    consoleButtons->addWidget(evaluateButton);
    consoleButtons->addWidget(clearConsoleButton);
    consoleButtons->addStretch();
    consoleLayout->addWidget(contextLabel);
    consoleLayout->addWidget(console, 1);
    consoleLayout->addLayout(consoleButtons);
    consoleLayout->addWidget(consoleStatus);
    tabs->addTab(consoleTab, QObject::tr("Expression Console"));

    auto* closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    layout->addWidget(closeButtons);

    auto expressionFromConsole = [console]() {
        const QStringList lines = console->toPlainText().split('\n');
        if (!lines.isEmpty()) {
            QString lastLine = lines.last().trimmed();
            if (lastLine == QStringLiteral(">") || lastLine.isEmpty()) {
                return QString();
            }
        }
        for (int i = lines.size() - 1; i >= 0; --i) {
            QString line = lines.at(i).trimmed();
            if (line.isEmpty() || line.startsWith(QStringLiteral("=")) || line.startsWith(QStringLiteral("!")) || line.startsWith(QStringLiteral("#"))) {
                continue;
            }
            if (line.startsWith(QStringLiteral(">"))) {
                line = line.mid(1).trimmed();
            }
            if (!line.isEmpty()) {
                return line;
            }
        }
        return QString();
    };
    auto appendConsole = [console](const QString& text) {
        QTextCursor cursor = console->textCursor();
        cursor.movePosition(QTextCursor::End);
        console->setTextCursor(cursor);
        if (!console->toPlainText().endsWith('\n')) {
            console->insertPlainText(QStringLiteral("\n"));
        }
        console->insertPlainText(text);
        console->moveCursor(QTextCursor::End);
    };
    auto insertConsoleExpression = [console, appendConsole](const QString& expression) {
        QString current = console->toPlainText();
        if (!current.endsWith(QStringLiteral("> ")) && !current.endsWith(QStringLiteral(">\n")) && !current.trimmed().isEmpty()) {
            appendConsole(QStringLiteral("> "));
        }
        QTextCursor cursor = console->textCursor();
        cursor.movePosition(QTextCursor::End);
        console->setTextCursor(cursor);
        console->insertPlainText(expression);
        console->moveCursor(QTextCursor::End);
    };
    auto refreshFunctionDetails = [functionsTable, functionDetails]() {
        const int row = functionsTable->currentRow();
        if (row < 0) {
            functionDetails->setHtml(QObject::tr("<b>Select a function to see syntax and usage details.</b>"));
            return;
        }
        const QString category = functionsTable->item(row, 0)->text();
        const QString name = functionsTable->item(row, 1)->text();
        const QString syntax = functionsTable->item(row, 2)->text();
        const QString description = functionsTable->item(row, 3)->text();
        const QString example = functionsTable->item(row, 4)->text();
        functionDetails->setHtml(QObject::tr("<h3>%1</h3><p><b>Category:</b> %2</p><p><b>Syntax:</b> <code>%3</code></p><p>%4</p><p><b>Example:</b> <code>%5</code></p>")
                                 .arg(name.toHtmlEscaped(), category.toHtmlEscaped(), syntax.toHtmlEscaped(), description.toHtmlEscaped(), example.toHtmlEscaped()));
    };
    auto evaluateExpression = [this, model, consoleStatus, expressionFromConsole, appendConsole]() {
        const QString expression = expressionFromConsole();
        if (expression.isEmpty()) {
            consoleStatus->setText(QObject::tr("Type an expression before evaluating."));
            return;
        }
        if (model == nullptr) {
            appendConsole(QObject::tr("! No current model. Open a model before evaluating parser expressions.\n> "));
            consoleStatus->setText(QObject::tr("No current model."));
            return;
        }

        bool success = false;
        std::string errorMessage;
        const double value = model->parseExpression(expression.toStdString(), success, errorMessage);
        if (success) {
            appendConsole(QObject::tr("= %1\n> ").arg(formatNumber(value)));
            consoleStatus->setText(QObject::tr("Expression evaluated successfully."));
        } else {
            const QString errorText = QString::fromStdString(errorMessage.empty() ? std::string("Unknown parser error.") : errorMessage);
            appendConsole(QObject::tr("! %1\n> ").arg(errorText));
            consoleStatus->setText(QObject::tr("Expression failed: %1").arg(errorText));
        }
    };

    QObject::connect(functionsTable, &QTableWidget::currentCellChanged, &dialog, [refreshFunctionDetails](int, int, int, int) {
        refreshFunctionDetails();
    });
    QObject::connect(functionFilter, &QLineEdit::textChanged, &dialog, [functionsTable](const QString& filter) {
        const QString needle = filter.trimmed().toLower();
        for (int row = 0; row < functionsTable->rowCount(); ++row) {
            QString haystack;
            for (int col = 0; col < functionsTable->columnCount(); ++col) {
                if (functionsTable->item(row, col) != nullptr) {
                    haystack += functionsTable->item(row, col)->text().toLower() + QStringLiteral(" ");
                }
            }
            functionsTable->setRowHidden(row, !needle.isEmpty() && !haystack.contains(needle));
        }
    });
    QObject::connect(insertExampleButton, &QPushButton::clicked, &dialog, [functionsTable, insertConsoleExpression, tabs, consoleTab]() {
        const int row = functionsTable->currentRow();
        if (row >= 0 && functionsTable->item(row, 4) != nullptr) {
            tabs->setCurrentWidget(consoleTab);
            insertConsoleExpression(functionsTable->item(row, 4)->text());
        }
    });
    QObject::connect(functionsTable, &QTableWidget::cellDoubleClicked, &dialog, [functionsTable, insertConsoleExpression, tabs, consoleTab](int row, int) {
        if (row >= 0 && functionsTable->item(row, 4) != nullptr) {
            tabs->setCurrentWidget(consoleTab);
            insertConsoleExpression(functionsTable->item(row, 4)->text());
        }
    });
    QObject::connect(saveDotButton, &QPushButton::clicked, &dialog, [this, dotText]() {
        const QString path = QFileDialog::getSaveFileName(_ownerWidget,
                                                          QObject::tr("Save Parser DOT"),
                                                          QDir::currentPath() + QStringLiteral("/genesys-parser.dot"),
                                                          QObject::tr("DOT files (*.dot);;All files (*.*)"),
                                                          nullptr,
                                                          QFileDialog::DontUseNativeDialog);
        if (path.isEmpty()) {
            return;
        }
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(_ownerWidget, QObject::tr("Parser Manager"), QObject::tr("Could not save DOT file."));
            return;
        }
        QTextStream out(&file);
        out << dotText->toPlainText();
    });
    QObject::connect(grammarScene, &QGraphicsScene::selectionChanged, &dialog, [grammarScene, toggleGrammarNode]() {
        const QList<QGraphicsItem*> selected = grammarScene->selectedItems();
        if (selected.isEmpty()) {
            return;
        }
        const QString id = selected.first()->data(0).toString();
        selected.first()->setSelected(false);
        toggleGrammarNode(id);
    });
    QObject::connect(expandGrammarButton, &QPushButton::clicked, &dialog, [&]() {
        collapsedGrammarNodes.clear();
        updateGrammarLabels();
        setGrammarSubtreeVisible(setGrammarSubtreeVisible, QStringLiteral("input"), true);
    });
    QObject::connect(compactGrammarButton, &QPushButton::clicked, &dialog, [&]() {
        collapsedGrammarNodes = QSet<QString>{QStringLiteral("assignment"), QStringLiteral("command"), QStringLiteral("logical_or"), QStringLiteral("primary"), QStringLiteral("function"), QStringLiteral("model_symbol")};
        updateGrammarLabels();
        setGrammarSubtreeVisible(setGrammarSubtreeVisible, QStringLiteral("input"), true);
    });
    QObject::connect(evaluateButton, &QPushButton::clicked, &dialog, evaluateExpression);
    QObject::connect(clearConsoleButton, &QPushButton::clicked, &dialog, [console, consoleStatus]() {
        console->setPlainText(QStringLiteral("> "));
        consoleStatus->setText(QObject::tr("Console cleared."));
    });
    auto* evaluateShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), console);
    QObject::connect(evaluateShortcut, &QShortcut::activated, &dialog, evaluateExpression);
    console->setReturnHandler(evaluateExpression);
    QObject::connect(closeButtons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    functionsTable->selectRow(0);
    refreshFunctionDetails();
    dialog.exec();
}

// Launch the standalone-leaning Optimizer workstation connected to the current model when available.
void DialogUtilityController::onActionToolsOptimizatorTriggered() {
    auto* window = new OptimizerWindow(_simulator, _ownerWidget);
    window->setAttribute(Qt::WA_DeleteOnClose, true);
    window->show();
    window->raise();
    window->activateWindow();
}

// Launch the standalone-leaning Data Analyzer workstation without forcing an initial dataset prompt.
void DialogUtilityController::onActionToolsDataAnalyzerTriggered() {
    auto* window = new DataAnalyzerWindow(_simulator, _lastDataAnalyzerPath, _ownerWidget);
    window->setAttribute(Qt::WA_DeleteOnClose, true);
    window->show();
    window->raise();
    window->activateWindow();
}

// Preserve view-configuration workflow and existing action-based synchronization path.
void DialogUtilityController::onActionViewConfigureTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Configure View"));
    auto* layout = new QFormLayout(&dialog);
    auto* showGrid = new QCheckBox(QObject::tr("Show grid"), &dialog);
    auto* showRule = new QCheckBox(QObject::tr("Show ruler"), &dialog);
    auto* showGuides = new QCheckBox(QObject::tr("Show guides"), &dialog);
    auto* snapToGrid = new QCheckBox(QObject::tr("Snap to grid"), &dialog);
    auto* showStatistics = new QCheckBox(QObject::tr("Statistics elements"), &dialog);
    auto* showEditable = new QCheckBox(QObject::tr("Editable elements"), &dialog);
    auto* showShared = new QCheckBox(QObject::tr("Shared elements"), &dialog);
    auto* showRecursive = new QCheckBox(QObject::tr("Recursive expansion"), &dialog);
    auto* gridInterval = new QSpinBox(&dialog);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    showGrid->setChecked(scene->isGridVisible());
    showRule->setChecked(_graphicsView->isRuleVisible());
    showGuides->setChecked(_graphicsView->isGuidesVisible());
    snapToGrid->setChecked(_ui->actionShowSnap->isChecked());
    showStatistics->setChecked(_ui->actionShowInternalElements->isChecked());
    showEditable->setChecked(_ui->actionShowEditableElements->isChecked());
    showShared->setChecked(_ui->actionShowAttachedElements->isChecked());
    showRecursive->setChecked(_ui->actionShowRecursiveElements->isChecked());
    gridInterval->setRange(5, 200);
    gridInterval->setValue(static_cast<int>(scene->grid()->interval));

    layout->addRow(showGrid);
    layout->addRow(showRule);
    layout->addRow(showGuides);
    layout->addRow(snapToGrid);
    layout->addRow(QObject::tr("Grid interval"), gridInterval);
    layout->addRow(new QLabel(QObject::tr("Model data definitions"), &dialog));
    layout->addRow(showStatistics);
    layout->addRow(showEditable);
    layout->addRow(showShared);
    layout->addRow(showRecursive);
    layout->addRow(buttons);

    // Keep the same application path by toggling existing QAction handlers.
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, scene, showGrid, showRule, showGuides, snapToGrid, showStatistics, showEditable, showShared, showRecursive, gridInterval, &dialog]() {
        scene->grid()->interval = static_cast<unsigned int>(gridInterval->value());
        if (scene->isGridVisible()) {
            scene->setGridVisible(false);
            scene->setGridVisible(true);
        }
        _ui->actionShowGrid->setChecked(showGrid->isChecked());
        _ui->actionShowGrid->trigger();
        _ui->actionShowRule->setChecked(showRule->isChecked());
        _ui->actionShowRule->trigger();
        _ui->actionShowGuides->setChecked(showGuides->isChecked());
        _ui->actionShowGuides->trigger();
        _ui->actionShowSnap->setChecked(snapToGrid->isChecked());
        scene->setSnapToGrid(snapToGrid->isChecked());

        const bool statisticsVisible = showStatistics->isChecked();
        const bool editableVisible = showEditable->isChecked();
        const bool sharedVisible = showShared->isChecked();
        const bool recursiveVisible = showRecursive->isChecked();

        // Apply data-definition category visibility directly so Configure View does not rely on action toggling order.
        {
            const QSignalBlocker blockStatisticsAction(_ui->actionShowInternalElements);
            const QSignalBlocker blockEditableAction(_ui->actionShowEditableElements);
            const QSignalBlocker blockSharedAction(_ui->actionShowAttachedElements);
            const QSignalBlocker blockRecursiveAction(_ui->actionShowRecursiveElements);
            const QSignalBlocker blockStatisticsCheckbox(_ui->checkBox_ShowInternals);
            const QSignalBlocker blockEditableCheckbox(_ui->checkBox_ShowEditableElements);
            const QSignalBlocker blockSharedCheckbox(_ui->checkBox_ShowElements);
            const QSignalBlocker blockRecursiveCheckbox(_ui->checkBox_ShowRecursive);

            _ui->actionShowInternalElements->setChecked(statisticsVisible);
            _ui->actionShowEditableElements->setChecked(editableVisible);
            _ui->actionShowAttachedElements->setChecked(sharedVisible);
            _ui->actionShowRecursiveElements->setChecked(recursiveVisible);
            _ui->checkBox_ShowInternals->setChecked(statisticsVisible);
            _ui->checkBox_ShowEditableElements->setChecked(editableVisible);
            _ui->checkBox_ShowElements->setChecked(sharedVisible);
            _ui->checkBox_ShowRecursive->setChecked(recursiveVisible);
        }

        scene->setShowStatisticsDataDefinitions(statisticsVisible);
        scene->setShowEditableDataDefinitions(editableVisible);
        scene->setShowSharedDataDefinitions(sharedVisible);
        scene->setShowRecursiveDataDefinitions(recursiveVisible);
        scene->requestGraphicalDataDefinitionsSync();
        if (_createModelImage) {
            _createModelImage();
        }
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

// Preserve simulator preferences dialog parenting and non-blocking show behavior.
void DialogUtilityController::onActionSimulatorPreferencesTriggered() {
    DialogSystemPreferences* dialog = new DialogSystemPreferences(_ownerWidget);
    dialog->show();
}

// Preserve plugin manager dialog parenting and non-blocking show behavior.
void DialogUtilityController::onActionSimulatorsPluginManagerTriggered() {
    DialogPluginManager* dialog = new DialogPluginManager(_ownerWidget);
    dialog->setSimulator(_simulator);
    dialog->setPluginCatalogRefreshCallback(_reloadPluginCatalog);
    dialog->show();
}

// Preserve breakpoint insertion flow and post-update refresh behavior.
void DialogUtilityController::onPushButtonBreakpointInsertClicked() {
    Model* model = _simulator->getModelManager()->current();
    if (model == nullptr) {
        return;
    }
    ModelSimulation* sim = model->getSimulation();
    if (sim == nullptr) {
        return;
    }

    dialogBreakpoint dialog;
    dialog.setModal(true);
    dialog.setMVCModel(_simulator);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    std::unique_ptr<dialogBreakpoint::MVCResult> result(dialog.getMVCResult());
    if (!result) {
        return;
    }

    if (result->type == "Time") {
        const double onTime = QString::fromStdString(result->on).toDouble();
        if (sim->getBreakpointsOnTime()->find(onTime) == sim->getBreakpointsOnTime()->list()->end()) {
            sim->getBreakpointsOnTime()->insert(onTime);
        }
    } else if (result->type == "Entity") {
        ModelDataDefinition* dataDef = model->getDataManager()->getDataDefinition(Util::TypeOf<Entity>(), result->on);
        Entity* entity = dynamic_cast<Entity*> (dataDef);
        if (entity != nullptr && sim->getBreakpointsOnEntity()->find(entity) == sim->getBreakpointsOnEntity()->list()->end()) {
            sim->getBreakpointsOnEntity()->insert(entity);
        }
    } else if (result->type == "Component") {
        ModelComponent* comp = model->getComponentManager()->find(result->on);
        if (comp != nullptr && sim->getBreakpointsOnComponent()->find(comp) == sim->getBreakpointsOnComponent()->list()->end()) {
            sim->getBreakpointsOnComponent()->insert(comp);
        }
    }

    _actualizeDebugBreakpoints(true);
}

// Preserve breakpoint removal workflow based on current selection and typed breakpoint target.
void DialogUtilityController::onPushButtonBreakpointRemoveClicked() {
    Model* model = _simulator->getModelManager()->current();
    if (model == nullptr) {
        return;
    }
    ModelSimulation* sim = model->getSimulation();
    if (sim == nullptr) {
        return;
    }

    int row = _ui->tableWidget_Breakpoints->currentRow();
    if (row < 0 && _ui->tableWidget_Breakpoints->selectionModel() != nullptr) {
        const QModelIndexList selectedRows = _ui->tableWidget_Breakpoints->selectionModel()->selectedRows();
        if (!selectedRows.isEmpty()) {
            row = selectedRows.first().row();
        }
    }
    if (row < 0) {
        return;
    }
    QTableWidgetItem* typeItem = _ui->tableWidget_Breakpoints->item(row, 1);
    QTableWidgetItem* onItem = _ui->tableWidget_Breakpoints->item(row, 2);
    if (typeItem == nullptr || onItem == nullptr) {
        return;
    }

    const std::string type = typeItem->text().toStdString();
    const std::string on = onItem->text().toStdString();
    if (type == "Time") {
        sim->getBreakpointsOnTime()->remove(QString::fromStdString(on).toDouble());
    } else if (type == "Entity") {
        ModelDataDefinition* dataDef = model->getDataManager()->getDataDefinition(Util::TypeOf<Entity>(), on);
        Entity* entity = dynamic_cast<Entity*> (dataDef);
        if (entity != nullptr) {
            sim->getBreakpointsOnEntity()->remove(entity);
        }
    } else if (type == "Component") {
        ModelComponent* comp = model->getComponentManager()->find(on);
        if (comp != nullptr) {
            sim->getBreakpointsOnComponent()->remove(comp);
        }
    }

    _actualizeDebugBreakpoints(true);
}

// Preserve export workflow, including image fallback render and save-format handling.
void DialogUtilityController::onPushButtonExportClicked() {
    QPixmap modelPixmap = _ui->label_ModelGraphic->pixmap();
    if (modelPixmap.isNull()) {
        _createModelImage();
        modelPixmap = _ui->label_ModelGraphic->pixmap();
    }

    if (modelPixmap.isNull()) {
        ModelGraphicsScene* scene = _graphicsView->getScene();
        if (scene == nullptr || scene->items().isEmpty()) {
            QMessageBox::information(_ownerWidget, QObject::tr("Export Diagram"), QObject::tr("There is no diagram/image available to export."));
            return;
        }

        QRectF bounds = scene->itemsBoundingRect();
        if (!bounds.isValid() || bounds.isEmpty()) {
            QMessageBox::information(_ownerWidget, QObject::tr("Export Diagram"), QObject::tr("There is no diagram/image available to export."));
            return;
        }

        QImage image(bounds.size().toSize() + QSize(20, 20), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.translate(-bounds.topLeft() + QPointF(10.0, 10.0));
        scene->render(&painter);
        painter.end();
        modelPixmap = QPixmap::fromImage(image);
    }

    const QString defaultName = QDir::currentPath() + "/model-diagram.png";
    const QString filters = QObject::tr("PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;Bitmap Image (*.bmp)");
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(_ownerWidget, QObject::tr("Export Diagram"), defaultName, filters, &selectedFilter, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) {
        return;
    }

    QString format = "PNG";
    if (selectedFilter.contains("*.jpg") || selectedFilter.contains("*.jpeg")) {
        format = "JPG";
    } else if (selectedFilter.contains("*.bmp")) {
        format = "BMP";
    }

    if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName += "." + format.toLower();
    }

    if (!modelPixmap.save(fileName, format.toStdString().c_str())) {
        QMessageBox::warning(_ownerWidget, QObject::tr("Export Diagram"), QObject::tr("Could not export diagram to file."));
        return;
    }

    QMessageBox::information(_ownerWidget, QObject::tr("Export Diagram"), QObject::tr("Diagram exported successfully."));
}

// Preserve parallelization dialog flow and persisted in-session settings behavior.
void DialogUtilityController::onActionParallelizationTriggered() {
    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Parallelization"));
    auto* layout = new QFormLayout(&dialog);
    auto* enabled = new QCheckBox(QObject::tr("Enable parallel execution preparation"), &dialog);
    auto* threads = new QSpinBox(&dialog);
    auto* batchSize = new QSpinBox(&dialog);
    auto* statusLabel = new QLabel(&dialog);
    statusLabel->setWordWrap(true);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    enabled->setChecked(_parallelizationEnabled);
    threads->setRange(1, 256);
    threads->setValue(_parallelizationThreads);
    batchSize->setRange(1, 1000000);
    batchSize->setValue(_parallelizationBatchSize);

    layout->addRow(enabled);
    layout->addRow(QObject::tr("Worker threads"), threads);
    layout->addRow(QObject::tr("Batch size"), batchSize);
    layout->addRow(statusLabel);
    layout->addRow(buttons);

    // Keep accepted callback behavior and status-bar messaging unchanged.
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, enabled, threads, batchSize, statusLabel, &dialog]() {
        _parallelizationEnabled = enabled->isChecked();
        _parallelizationThreads = threads->value();
        _parallelizationBatchSize = batchSize->value();
        statusLabel->setText(QObject::tr("Configuration saved: enabled=%1, threads=%2, batch size=%3")
                             .arg(_parallelizationEnabled ? QObject::tr("true") : QObject::tr("false"))
                             .arg(_parallelizationThreads)
                             .arg(_parallelizationBatchSize));
        _ownerWidget->statusBar()->showMessage(QObject::tr("Parallelization prepared: enabled=%1, threads=%2, batch size=%3")
                                               .arg(_parallelizationEnabled ? QObject::tr("true") : QObject::tr("false"))
                                               .arg(_parallelizationThreads)
                                               .arg(_parallelizationBatchSize), 5000);
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}
