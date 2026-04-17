#include "DataAnalyzerWindow.h"

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/Simulator.h"
#include "tools/FitterDefaultImpl.h"
#include "tools/HypothesisTesterDefaultImpl1.h"
#include "tools/SimulationResultsDataset.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSet>
#include <QSplitter>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QTemporaryFile>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTextStream>
#include <QToolBar>
#include <QVariant>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

namespace {
    static QTableWidgetItem* readOnlyItem(const QString& text) {
        auto* item = new QTableWidgetItem(text);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        return item;
    }

    static double normalCdf(double x, double mean, double stddev) {
        if (!(stddev > 0.0) || !std::isfinite(stddev)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return 0.5 * std::erfc(-(x - mean) / (stddev * std::sqrt(2.0)));
    }

    static double triangularCdf(double x, double min, double mode, double max) {
        if (!(max > min) || mode < min || mode > max) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        if (x <= min) {
            return 0.0;
        }
        if (x >= max) {
            return 1.0;
        }
        if (x <= mode) {
            return ((x - min) * (x - min)) / ((max - min) * (mode - min));
        }
        return 1.0 - ((max - x) * (max - x)) / ((max - min) * (max - mode));
    }

    static double erlangCdf(double x, double mean, int m) {
        if (x < 0.0 || !(mean > 0.0) || m < 1) {
            return x < 0.0 ? 0.0 : std::numeric_limits<double>::quiet_NaN();
        }
        const double scale = mean / static_cast<double>(m);
        const double z = x / scale;
        double sum = 1.0;
        double term = 1.0;
        for (int k = 1; k < m; ++k) {
            term *= z / static_cast<double>(k);
            sum += term;
        }
        return 1.0 - std::exp(-z) * sum;
    }

    static double approximateNormalSurvival(double z) {
        return 0.5 * std::erfc(z / std::sqrt(2.0));
    }

    static double approximateChiSquareSurvival(double statistic, int degreesOfFreedom) {
        if (!std::isfinite(statistic) || degreesOfFreedom <= 0) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        const double k = static_cast<double>(degreesOfFreedom);
        const double z = (std::pow(statistic / k, 1.0 / 3.0) - (1.0 - 2.0 / (9.0 * k))) / std::sqrt(2.0 / (9.0 * k));
        return std::max(0.0, std::min(1.0, approximateNormalSurvival(z)));
    }

    static double approximateKolmogorovPValue(double statistic, int count) {
        if (!std::isfinite(statistic) || count <= 0) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        const double sqrtN = std::sqrt(static_cast<double>(count));
        const double lambda = (sqrtN + 0.12 + 0.11 / sqrtN) * statistic;
        double sum = 0.0;
        for (int j = 1; j <= 80; ++j) {
            const double term = std::exp(-2.0 * j * j * lambda * lambda);
            sum += (j % 2 == 1 ? 1.0 : -1.0) * term;
            if (term < 1e-10) {
                break;
            }
        }
        return std::max(0.0, std::min(1.0, 2.0 * sum));
    }

    static QString fitDecisionText(double pValue) {
        if (!std::isfinite(pValue)) {
            return QObject::tr("not available");
        }
        const QString recommendation = pValue >= 0.05
                                           ? QObject::tr("suggest keeping this distribution")
                                           : QObject::tr("suggest rejecting this distribution");
        return QObject::tr("The risk of rejecting this fit hypothesis is %1; %2.")
                .arg(QString::number(pValue, 'g', 5), recommendation);
    }

    static QString datasetScopeKey(int datasetIndex) {
        return QStringLiteral("dataset:%1").arg(datasetIndex);
    }

    static QString replicationScopeKey(int datasetIndex, int replication) {
        return QStringLiteral("dataset:%1:rep:%2").arg(datasetIndex).arg(replication);
    }

    static bool parseScopeKey(const QString& key, int* datasetIndex, int* replication) {
        if (datasetIndex != nullptr) {
            *datasetIndex = -1;
        }
        if (replication != nullptr) {
            *replication = 0;
        }
        const QStringList parts = key.split(QStringLiteral(":"));
        if (parts.size() < 2 || parts.at(0) != QStringLiteral("dataset")) {
            return false;
        }
        bool ok = false;
        const int parsedDataset = parts.at(1).toInt(&ok);
        if (!ok) {
            return false;
        }
        int parsedReplication = 0;
        if (parts.size() >= 4 && parts.at(2) == QStringLiteral("rep")) {
            parsedReplication = parts.at(3).toInt(&ok);
            if (!ok) {
                return false;
            }
        }
        if (datasetIndex != nullptr) {
            *datasetIndex = parsedDataset;
        }
        if (replication != nullptr) {
            *replication = parsedReplication;
        }
        return true;
    }

    static QString dataAnalyzerStyleSheet() {
        return QStringLiteral(
                "QMainWindow { background: #f5f7f8; }"
                "QGroupBox { font-weight: 600; border: 1px solid #cfd8dc; border-radius: 6px; margin-top: 12px; padding-top: 10px; background: #ffffff; }"
                "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }"
                "QTabWidget::pane { border: 1px solid #cfd8dc; background: #ffffff; }"
                "QTableWidget, QPlainTextEdit, QTextEdit, QTextBrowser, QListWidget { border: 1px solid #cfd8dc; background: #ffffff; }"
                "QPushButton { border-radius: 6px; padding: 5px 12px; }");
    }
}

class DataAnalyzerWindow::HistogramPreview : public QWidget {
public:
    explicit HistogramPreview(QWidget* parent = nullptr)
        : QWidget(parent) {
        setMinimumHeight(180);
    }

    void setValues(const QList<double>& values) {
        _values = values;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor(250, 251, 252));
        const QRect plotRect = rect().adjusted(44, 18, -18, -34);
        painter.setPen(QPen(QColor(190, 198, 206)));
        painter.drawRect(plotRect);
        painter.setPen(QColor(69, 90, 100));
        painter.drawText(rect().adjusted(8, 0, -8, -8), Qt::AlignBottom | Qt::AlignLeft,
                         QObject::tr("Histogram preview"));

        if (_values.isEmpty()) {
            painter.setPen(QColor(96, 112, 128));
            painter.drawText(plotRect, Qt::AlignCenter, QObject::tr("Open a dataset or import simulation responses."));
            return;
        }

        auto minmax = std::minmax_element(_values.begin(), _values.end());
        const double minValue = *minmax.first;
        const double maxValue = *minmax.second;
        const int bins = std::min(12, std::max(4, static_cast<int>(std::sqrt(static_cast<double>(_values.size())))));
        QVector<int> counts(bins, 0);
        const double range = std::max(1e-12, maxValue - minValue);
        for (double value: _values) {
            int bin = static_cast<int>(((value - minValue) / range) * bins);
            bin = std::min(bins - 1, std::max(0, bin));
            counts[bin]++;
        }
        const int maxCount = *std::max_element(counts.begin(), counts.end());
        if (maxCount == 0) {
            return;
        }

        const double barWidth = static_cast<double>(plotRect.width()) / bins;
        for (int i = 0; i < bins; ++i) {
            const double height = plotRect.height() * static_cast<double>(counts.at(i)) / static_cast<double>(maxCount);
            const QRectF bar(plotRect.left() + i * barWidth + 3,
                             plotRect.bottom() - height,
                             std::max(2.0, barWidth - 6),
                             height);
            painter.fillRect(bar, QColor(0, 105, 92));
        }
        painter.setPen(QColor(96, 112, 128));
        painter.drawText(plotRect.adjusted(2, 2, -2, -2), Qt::AlignTop | Qt::AlignLeft,
                         QObject::tr("n=%1").arg(_values.size()));
    }

private:
    QList<double> _values;
};

class DataAnalyzerWindow::DoePreviewPlot : public QWidget {
public:
    explicit DoePreviewPlot(QWidget* parent = nullptr)
        : QWidget(parent) {
        setMinimumHeight(260);
    }

    void setMode(const QString& mode) {
        _mode = mode;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor(250, 251, 252));
        const QRect plotRect = rect().adjusted(54, 28, -28, -44);
        painter.setPen(QPen(QColor(190, 198, 206)));
        painter.drawRect(plotRect);

        if (_mode == QObject::tr("3D Surface")) {
            paintSurface(painter, plotRect);
        } else if (_mode == QObject::tr("Profiler")) {
            paintProfiler(painter, plotRect);
        } else if (_mode == QObject::tr("Desirability")) {
            paintDesirability(painter, plotRect);
        } else if (_mode == QObject::tr("Diagnostics")) {
            paintDiagnostics(painter, plotRect);
        } else {
            paintContour(painter, plotRect);
        }

        painter.setPen(QColor(69, 90, 100));
        painter.drawText(rect().adjusted(10, 4, -10, -8), Qt::AlignTop | Qt::AlignLeft,
                         QObject::tr("DOE/RSM preview: %1 (demonstration data)").arg(_mode));
        painter.drawText(rect().adjusted(10, 0, -10, -8), Qt::AlignBottom | Qt::AlignRight,
                         QObject::tr("Factor A: service capacity    Factor B: arrival rate"));
    }

private:
    void paintContour(QPainter& painter, const QRect& plotRect) {
        const int nx = 28;
        const int ny = 18;
        for (int ix = 0; ix < nx; ++ix) {
            for (int iy = 0; iy < ny; ++iy) {
                const double x = -1.0 + 2.0 * ix / static_cast<double>(nx - 1);
                const double y = -1.0 + 2.0 * iy / static_cast<double>(ny - 1);
                const double z = 0.75 - 0.45 * x + 0.35 * y + 0.7 * x * x + 0.45 * y * y - 0.5 * x * y;
                const int intensity = std::max(0, std::min(255, static_cast<int>(80 + 80 * z)));
                const QColor color(255 - intensity / 2, 240 - intensity / 3, 130 + intensity / 4);
                painter.fillRect(QRectF(plotRect.left() + ix * plotRect.width() / static_cast<double>(nx),
                                        plotRect.top() + iy * plotRect.height() / static_cast<double>(ny),
                                        plotRect.width() / static_cast<double>(nx) + 1,
                                        plotRect.height() / static_cast<double>(ny) + 1), color);
            }
        }
        painter.setPen(QPen(QColor(38, 50, 56), 2));
        for (int k = 1; k <= 5; ++k) {
            QRectF ellipse = plotRect.adjusted(22 * k, 14 * k, -28 * k, -18 * k);
            painter.drawEllipse(ellipse);
        }
    }

    void paintSurface(QPainter& painter, const QRect& plotRect) {
        painter.setPen(QPen(QColor(0, 105, 92), 2));
        for (int line = 0; line < 9; ++line) {
            QPainterPath path;
            for (int i = 0; i < 60; ++i) {
                const double x = i / 59.0;
                const double y = line / 8.0;
                const double wave = std::sin((x * 2.2 + y * 1.4) * 3.14159);
                const double px = plotRect.left() + x * plotRect.width();
                const double py = plotRect.bottom() - (0.20 + 0.55 * y + 0.12 * wave) * plotRect.height();
                if (i == 0) {
                    path.moveTo(px, py);
                } else {
                    path.lineTo(px, py);
                }
            }
            painter.drawPath(path);
        }
        painter.setPen(QPen(QColor(255, 152, 0), 2, Qt::DashLine));
        painter.drawLine(plotRect.left() + plotRect.width() * 0.62, plotRect.top(),
                         plotRect.left() + plotRect.width() * 0.62, plotRect.bottom());
    }

    void paintProfiler(QPainter& painter, const QRect& plotRect) {
        painter.setPen(QPen(QColor(0, 105, 92), 3));
        QPainterPath responsePath;
        for (int i = 0; i < 80; ++i) {
            const double x = i / 79.0;
            const double y = 0.70 - 0.42 * std::pow(x - 0.62, 2.0) - 0.08 * std::sin(12.0 * x);
            const QPointF p(plotRect.left() + x * plotRect.width(), plotRect.bottom() - y * plotRect.height());
            if (i == 0) {
                responsePath.moveTo(p);
            } else {
                responsePath.lineTo(p);
            }
        }
        painter.drawPath(responsePath);
        painter.setPen(QPen(QColor(198, 40, 40), 2, Qt::DashLine));
        painter.drawLine(plotRect.left(), plotRect.bottom() - plotRect.height() * 0.62,
                         plotRect.right(), plotRect.bottom() - plotRect.height() * 0.62);
        painter.drawText(plotRect.adjusted(8, 8, -8, -8), Qt::AlignTop | Qt::AlignRight,
                         QObject::tr("target <= 8.0 min"));
    }

    void paintDesirability(QPainter& painter, const QRect& plotRect) {
        const QVector<double> values = {0.28, 0.44, 0.61, 0.76, 0.83, 0.81, 0.74};
        const double barWidth = plotRect.width() / static_cast<double>(values.size());
        for (int i = 0; i < values.size(); ++i) {
            const QRectF bar(plotRect.left() + i * barWidth + 8,
                             plotRect.bottom() - values.at(i) * plotRect.height(),
                             barWidth - 16,
                             values.at(i) * plotRect.height());
            painter.fillRect(bar, QColor(0, 105, 92));
            painter.setPen(QColor(69, 90, 100));
            painter.drawText(QRectF(bar.left(), plotRect.bottom() + 4, bar.width(), 22),
                             Qt::AlignCenter, QString::number(i + 1));
        }
        painter.setPen(QPen(QColor(255, 152, 0), 2));
        painter.drawLine(plotRect.left(), plotRect.bottom() - 0.8 * plotRect.height(),
                         plotRect.right(), plotRect.bottom() - 0.8 * plotRect.height());
    }

    void paintDiagnostics(QPainter& painter, const QRect& plotRect) {
        painter.setPen(QPen(QColor(0, 105, 92), 2));
        painter.drawLine(plotRect.topLeft(), plotRect.bottomRight());
        painter.setBrush(QColor(255, 152, 0));
        painter.setPen(Qt::NoPen);
        const QVector<QPointF> points = {
            {0.08, 0.10}, {0.18, 0.16}, {0.25, 0.28}, {0.36, 0.34}, {0.48, 0.46},
            {0.57, 0.61}, {0.65, 0.58}, {0.78, 0.80}, {0.88, 0.84}
        };
        for (const QPointF& point : points) {
            painter.drawEllipse(QPointF(plotRect.left() + point.x() * plotRect.width(),
                                        plotRect.bottom() - point.y() * plotRect.height()), 5, 5);
        }
    }

private:
    QString _mode = QObject::tr("Contour");
};

DataAnalyzerWindow::DataAnalyzerWindow(Simulator* simulator, QString& lastDataAnalyzerPath, QWidget* parent)
    : QMainWindow(parent),
      _simulator(simulator),
      _lastDataAnalyzerPath(lastDataAnalyzerPath),
      _studyName(tr("Untitled Analysis Study")) {
    setWindowTitle(tr("Genesys Data Analyzer"));
    resize(1180, 760);
    setStyleSheet(dataAnalyzerStyleSheet());
    buildMenus();
    buildWorkspace();
    connectActions();
    refreshScopeSelector();
    refreshModelResponses();
    refreshAnalysisViews();
}

void DataAnalyzerWindow::buildMenus() {
    QMenu* fileMenu = menuBar()->addMenu(tr("Arquivo"));
    _openDatasetAction = fileMenu->addAction(tr("Abrir dataset..."));
    _importSimulationResponsesAction = fileMenu->addAction(tr("Importar respostas da simulacao"));
    fileMenu->addSeparator();
    _saveReportAction = fileMenu->addAction(tr("Salvar relatorio..."));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Fechar"), this, &QWidget::close);

    QMenu* editMenu = menuBar()->addMenu(tr("Editar"));
    editMenu->addAction(tr("Copiar selecao"), this, [this]() { showSkeletonMessage(tr("copy and edit workflow")); });
    editMenu->addAction(tr("Preferencias da ferramenta"), this,
                        [this]() { showSkeletonMessage(tr("Data Analyzer preferences")); });

    QMenu* dataMenu = menuBar()->addMenu(tr("Dados"));
    dataMenu->addAction(tr("Novo dataset vazio"), this, [this]() {
        addDataset({
                tr("Empty dataset"), tr("Random variable"), tr("Manual dataset placeholder"), tr("Continuous numeric"),
                tr("Manual dataset"), {}, {}, true
        });
    });
    dataMenu->addAction(tr("Transformar / filtrar dados"), this,
                        [this]() { showSkeletonMessage(tr("data transformation")); });
    dataMenu->addAction(tr("Gerenciar datasets"), this, [this]() { _workspaceTabs->setCurrentIndex(0); });

    QMenu* analysisMenu = menuBar()->addMenu(tr("Analises"));
    analysisMenu->addAction(tr("Analise exploratoria de dados"), this, [this]() {
        showAnalysisSetupDialog(tr("Exploratory data analysis"), 2);
    });
    analysisMenu->addAction(tr("Sintese estatistica"), this,
                            [this]() { showAnalysisSetupDialog(tr("Statistical synthesis"), 2); });
    analysisMenu->addAction(tr("Ajuste de distribuicoes"), this,
                            [this]() { showAnalysisSetupDialog(tr("Distribution fitting"), 3); });
    analysisMenu->addAction(tr("Inferencia estatistica"), this,
                            [this]() { showAnalysisSetupDialog(tr("Statistical inference"), 4); });
    analysisMenu->addAction(tr("Modelos de regressao"), this,
                            [this]() { showAnalysisSetupDialog(tr("Regression models"), 5); });
    analysisMenu->addAction(tr("Analise de variancia"), this,
                            [this]() { showAnalysisSetupDialog(tr("Analysis of variance"), 5); });

    QMenu* doeMenu = menuBar()->addMenu(tr("Projeto experimental"));
    doeMenu->addAction(tr("Planejamento fatorial"), this,
                       [this]() { showAnalysisSetupDialog(tr("Factorial design"), 6); });
    doeMenu->addAction(tr("Superficies de resposta"), this,
                       [this]() { showAnalysisSetupDialog(tr("Response surfaces"), 6); });
    doeMenu->addAction(tr("Otimizacao experimental"), this,
                       [this]() { showSkeletonMessage(tr("experimental optimization")); });

    QMenu* toolsMenu = menuBar()->addMenu(tr("Ferramentas"));
    _refreshModelResponsesAction = toolsMenu->addAction(tr("Atualizar respostas do modelo"));
    toolsMenu->addAction(tr("Exportar dataset"), this, [this]() { showSkeletonMessage(tr("dataset export")); });

    QMenu* helpMenu = menuBar()->addMenu(tr("Ajuda"));
    helpMenu->addAction(tr("Sobre o Data Analyzer"), this, [this]() {
        QMessageBox::information(this, tr("Genesys Data Analyzer"),
                                 tr(
                                         "Prototype of a professional statistical-analysis workstation for simulation output, combining input analysis, output analysis, inference and design of experiments."));
    });

    QToolBar* toolbar = addToolBar(tr("Data Analyzer"));
    toolbar->addAction(_openDatasetAction);
    toolbar->addAction(_importSimulationResponsesAction);
    toolbar->addAction(_saveReportAction);
    toolbar->addSeparator();
    toolbar->addAction(_refreshModelResponsesAction);
}

void DataAnalyzerWindow::buildWorkspace() {
    auto* central = new QWidget(this);
    auto* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(6, 6, 6, 6);
    auto* scopeBar = new QHBoxLayout();
    auto* studyLabel = new QLabel(tr("Analysis Study: %1").arg(_studyName), central);
    studyLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));
    _scopeCombo = new QComboBox(central);
    _scopeCombo->setMinimumWidth(280);
    scopeBar->addWidget(studyLabel);
    scopeBar->addStretch();
    scopeBar->addWidget(new QLabel(tr("Analysis scope:"), central));
    scopeBar->addWidget(_scopeCombo);
    centralLayout->addLayout(scopeBar);

    auto* splitter = new QSplitter(Qt::Horizontal, central);
    centralLayout->addWidget(splitter, 1);
    setCentralWidget(central);

    auto* leftPanel = new QWidget(splitter);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(8, 8, 8, 8);

    auto* datasetsBox = new QGroupBox(tr("Analysis Study Datasets"), leftPanel);
    auto* datasetsLayout = new QVBoxLayout(datasetsBox);
    _datasetsList = new QListWidget(datasetsBox);
    auto* datasetButtons = new QHBoxLayout();
    auto* openButton = new QPushButton(tr("Abrir"), datasetsBox);
    auto* importButton = new QPushButton(tr("Importar"), datasetsBox);
    datasetButtons->addWidget(openButton);
    datasetButtons->addWidget(importButton);
    datasetsLayout->addWidget(_datasetsList);
    datasetsLayout->addLayout(datasetButtons);
    leftLayout->addWidget(datasetsBox, 2);

    auto* modelBox = new QGroupBox(tr("Current Genesys model"), leftPanel);
    auto* modelLayout = new QVBoxLayout(modelBox);
    _modelResponsesTable = new QTableWidget(modelBox);
    _modelResponsesTable->setColumnCount(4);
    _modelResponsesTable->setHorizontalHeaderLabels({tr("Element"), tr("Class"), tr("Response"), tr("Value")});
    _modelResponsesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _modelResponsesTable->horizontalHeader()->setStretchLastSection(true);
    _modelResponsesTable->verticalHeader()->setVisible(false);
    modelLayout->addWidget(_modelResponsesTable);
    leftLayout->addWidget(modelBox, 3);

    auto* analysisBox = new QGroupBox(tr("Analysis workflow"), leftPanel);
    auto* analysisLayout = new QVBoxLayout(analysisBox);
    _analysisNavigator = new QListWidget(analysisBox);
    _analysisNavigator->addItems({
            tr("Project overview"),
            tr("Dataset preview"),
            tr("Exploratory analysis"),
            tr("Distribution fitting"),
            tr("Statistical inference"),
            tr("Regression and ANOVA"),
            tr("DOE and response surfaces"),
            tr("Report")
    });
    analysisLayout->addWidget(_analysisNavigator);
    leftLayout->addWidget(analysisBox, 2);
    splitter->addWidget(leftPanel);

    _workspaceTabs = new QTabWidget(splitter);
    _startPage = new QTextBrowser(_workspaceTabs);
    _startPage->setOpenExternalLinks(false);
    _startPage->setHtml(tr(
            "<h2>Genesys Data Analyzer</h2>"
            "<p>This workspace is organized as an <b>Analysis Study</b>: a collection of one or more datasets that are analyzed together.</p>"
            "<ul>"
            "<li>Each dataset stores raw numeric values and metadata for the represented random variable.</li>"
            "<li>Analyses default to all datasets together, but the scope selector can focus on any single dataset.</li>"
            "<li>Open numeric datasets or import a snapshot of current simulation responses from the open model.</li>"
            "<li>Prepare interfaces for regression, ANOVA, design of experiments and response-surface analysis.</li>"
            "</ul>"
            "<p>Use <b>Arquivo/Abrir dataset</b> or <b>Importar respostas da simulacao</b> to start.</p>"));
    _workspaceTabs->addTab(_startPage, tr("Start"));

    auto* datasetTab = new QWidget(_workspaceTabs);
    auto* datasetLayout = new QVBoxLayout(datasetTab);
    _datasetSourceLabel = new QLabel(tr("No dataset loaded."), datasetTab);
    _datasetSourceLabel->setWordWrap(true);
    _datasetPreview = new QPlainTextEdit(datasetTab);
    _datasetPreview->setReadOnly(true);
    _studySummaryTable = new QTableWidget(datasetTab);
    _studySummaryTable->setColumnCount(11);
    _studySummaryTable->setHorizontalHeaderLabels({
            tr("Scope"), tr("Variable"), tr("Type"), tr("Replications"), tr("Time"), tr("Samples"), tr("Mean"),
            tr("Std dev"), tr("Min"), tr("Max"), tr("CV")
    });
    _studySummaryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _studySummaryTable->verticalHeader()->setVisible(false);
    datasetLayout->addWidget(_datasetSourceLabel);
    datasetLayout->addWidget(_datasetPreview, 2);
    datasetLayout->addWidget(new QLabel(tr("Analysis Study summary:"), datasetTab));
    datasetLayout->addWidget(_studySummaryTable, 1);
    _workspaceTabs->addTab(datasetTab, tr("Dataset"));

    auto* descriptiveTab = new QWidget(_workspaceTabs);
    auto* descriptiveLayout = new QVBoxLayout(descriptiveTab);
    auto* statsLayout = new QHBoxLayout();
    auto* centralBox = new QGroupBox(tr("Central tendency"), descriptiveTab);
    auto* centralStatsLayout = new QVBoxLayout(centralBox);
    _centralTendencyTable = new QTableWidget(centralBox);
    _centralTendencyTable->setColumnCount(2);
    _centralTendencyTable->setHorizontalHeaderLabels({tr("Statistic"), tr("Value")});
    _centralTendencyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    centralStatsLayout->addWidget(_centralTendencyTable);
    auto* dispersionBox = new QGroupBox(tr("Dispersion and shape"), descriptiveTab);
    auto* dispersionLayout = new QVBoxLayout(dispersionBox);
    _dispersionTable = new QTableWidget(dispersionBox);
    _dispersionTable->setColumnCount(2);
    _dispersionTable->setHorizontalHeaderLabels({tr("Statistic"), tr("Value")});
    _dispersionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    dispersionLayout->addWidget(_dispersionTable);
    statsLayout->addWidget(centralBox);
    statsLayout->addWidget(dispersionBox);
    _histogramPreview = new HistogramPreview(descriptiveTab);
    auto* rawLayout = new QHBoxLayout();
    _rawDataTable = new QTableWidget(descriptiveTab);
    _rawDataTable->setColumnCount(4);
    _rawDataTable->setHorizontalHeaderLabels({tr("Index"), tr("Replication"), tr("Time"), tr("Raw value")});
    _rawDataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _movingAverageTable = new QTableWidget(descriptiveTab);
    _movingAverageTable->setColumnCount(2);
    _movingAverageTable->setHorizontalHeaderLabels({tr("Index"), tr("Moving average (window=5)")});
    _movingAverageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    rawLayout->addWidget(_rawDataTable);
    rawLayout->addWidget(_movingAverageTable);
    descriptiveLayout->addLayout(statsLayout, 2);
    descriptiveLayout->addWidget(_histogramPreview, 1);
    descriptiveLayout->addLayout(rawLayout, 2);
    _workspaceTabs->addTab(descriptiveTab, tr("Exploratory"));

    auto* fitTab = new QWidget(_workspaceTabs);
    auto* fitLayout = new QVBoxLayout(fitTab);
    _fitConclusionLabel = new QLabel(tr("No dataset loaded."), fitTab);
    _fitConclusionLabel->setWordWrap(true);
    _fitTable = new QTableWidget(fitTab);
    _fitTable->setColumnCount(7);
    _fitTable->setHorizontalHeaderLabels({
            tr("Distribution"), tr("Parameters"), tr("SSE"), tr("Chi-square p-value"), tr("Chi-square decision"),
            tr("K-S p-value"), tr("K-S decision")
    });
    _fitTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    fitLayout->addWidget(_fitConclusionLabel);
    fitLayout->addWidget(_fitTable, 1);
    _workspaceTabs->addTab(fitTab, tr("Distribution Fit"));

    auto* inferenceTab = new QWidget(_workspaceTabs);
    auto* inferenceLayout = new QVBoxLayout(inferenceTab);
    _inferenceTable = new QTableWidget(inferenceTab);
    _inferenceTable->setColumnCount(4);
    _inferenceTable->setHorizontalHeaderLabels({tr("Measure"), tr("Lower"), tr("Upper"), tr("Half width")});
    _inferenceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    inferenceLayout->addWidget(new QLabel(
            tr("Confidence intervals use HypothesisTesterDefaultImpl1 for the loaded numeric dataset."), inferenceTab));
    inferenceLayout->addWidget(_inferenceTable, 1);
    _workspaceTabs->addTab(inferenceTab, tr("Inference"));

    auto* regressionTab = new QWidget(_workspaceTabs);
    auto* regressionLayout = new QVBoxLayout(regressionTab);
    auto* regressionText = new QTextBrowser(regressionTab);
    regressionText->setHtml(tr(
            "<h3>Regression and ANOVA workspace</h3>"
            "<p>Planned interfaces:</p>"
            "<ul>"
            "<li>response and factor selection from datasets or simulation responses;</li>"
            "<li>linear, polynomial and interaction regression models;</li>"
            "<li>residual analysis, lack-of-fit and ANOVA tables;</li>"
            "<li>model comparison and prediction intervals.</li>"
            "</ul>"
            "<p>This tab is intentionally structured as a shell until the numerical interfaces are defined.</p>"));
    regressionLayout->addWidget(regressionText);
    _workspaceTabs->addTab(regressionTab, tr("Regression / ANOVA"));

    auto* doeTab = new QWidget(_workspaceTabs);
    auto* doeLayout = new QVBoxLayout(doeTab);
    auto* doeIntro = new QTextBrowser(doeTab);
    doeIntro->setMaximumHeight(92);
    doeIntro->setHtml(tr(
        "<b>DOE/RSM preview.</b> This is a navigable prototype inspired by Design-Expert and JMP workflows. "
        "Tables and plots use demonstration data until Genesys experiment execution, model fitting and optimization interfaces are connected."));
    auto* doeWorkflowTabs = new QTabWidget(doeTab);

    auto* designBuilderTab = new QWidget(doeWorkflowTabs);
    auto* designBuilderLayout = new QVBoxLayout(designBuilderTab);
    auto* setupBox = new QGroupBox(tr("Design setup"), designBuilderTab);
    auto* setupLayout = new QVBoxLayout(setupBox);
    auto* setupControls = new QHBoxLayout();
    _doeDesignFamilyCombo = new QComboBox(setupBox);
    _doeDesignFamilyCombo->addItems({
            tr("Full factorial"),
            tr("Fractional factorial screening"),
            tr("Central composite RSM"),
            tr("Box-Behnken RSM"),
            tr("Custom Genesys experiment")
    });
    _doeModelOrderCombo = new QComboBox(setupBox);
    _doeModelOrderCombo->addItems({tr("Main effects"), tr("Two-factor interactions"), tr("Quadratic")});
    _doeModelOrderCombo->setCurrentText(tr("Quadratic"));
    _doeReplicationsSpin = new QSpinBox(setupBox);
    _doeReplicationsSpin->setRange(1, 100);
    _doeReplicationsSpin->setValue(3);
    _doeAlphaSpin = new QDoubleSpinBox(setupBox);
    _doeAlphaSpin->setRange(0.001, 0.25);
    _doeAlphaSpin->setDecimals(3);
    _doeAlphaSpin->setSingleStep(0.005);
    _doeAlphaSpin->setValue(0.05);
    _doeRandomizeCheck = new QCheckBox(tr("Randomize run order"), setupBox);
    _doeRandomizeCheck->setChecked(true);
    auto* previewDesignButton = new QPushButton(tr("Generate preview design"), setupBox);
    auto* runGenesysButton = new QPushButton(tr("Run in Genesys"), setupBox);
    setupControls->addWidget(new QLabel(tr("Design:"), setupBox));
    setupControls->addWidget(_doeDesignFamilyCombo, 2);
    setupControls->addWidget(new QLabel(tr("Model:"), setupBox));
    setupControls->addWidget(_doeModelOrderCombo, 2);
    setupControls->addWidget(new QLabel(tr("Replications:"), setupBox));
    setupControls->addWidget(_doeReplicationsSpin);
    setupControls->addWidget(new QLabel(tr("Alpha:"), setupBox));
    setupControls->addWidget(_doeAlphaSpin);
    setupControls->addWidget(_doeRandomizeCheck);
    setupControls->addStretch();
    setupControls->addWidget(previewDesignButton);
    setupControls->addWidget(runGenesysButton);
    _doePlanSummaryLabel = new QLabel(setupBox);
    _doePlanSummaryLabel->setWordWrap(true);
    _doeAdvisorLabel = new QLabel(setupBox);
    _doeAdvisorLabel->setWordWrap(true);
    _doeAdvisorLabel->setStyleSheet(QStringLiteral("color: #37474f;"));
    setupLayout->addLayout(setupControls);
    setupLayout->addWidget(_doePlanSummaryLabel);
    setupLayout->addWidget(_doeAdvisorLabel);
    designBuilderLayout->addWidget(setupBox);

    auto* designTablesLayout = new QHBoxLayout();
    _doeFactorsTable = new QTableWidget(designBuilderTab);
    _doeFactorsTable->setColumnCount(5);
    _doeFactorsTable->setHorizontalHeaderLabels({tr("Factor"), tr("Role"), tr("Low"), tr("Center"), tr("High")});
    _doeFactorsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _doeResponsesTable = new QTableWidget(designBuilderTab);
    _doeResponsesTable->setColumnCount(4);
    _doeResponsesTable->setHorizontalHeaderLabels({tr("Response"), tr("Goal"), tr("Lower"), tr("Target")});
    _doeResponsesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    designTablesLayout->addWidget(_doeFactorsTable);
    designTablesLayout->addWidget(_doeResponsesTable);
    designBuilderLayout->addLayout(designTablesLayout, 1);

    _doeAdviceTable = new QTableWidget(designBuilderTab);
    _doeAdviceTable->setColumnCount(3);
    _doeAdviceTable->setHorizontalHeaderLabels({tr("Workflow step"), tr("Current preview"), tr("Future Genesys integration")});
    _doeAdviceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _doeAdviceTable->verticalHeader()->setVisible(false);
    _doeAdviceTable->setMaximumHeight(150);
    designBuilderLayout->addWidget(_doeAdviceTable);
    _doeDesignQualityTable = new QTableWidget(designBuilderTab);
    _doeDesignQualityTable->setColumnCount(4);
    _doeDesignQualityTable->setHorizontalHeaderLabels({tr("Design diagnostic"), tr("Preview value"), tr("Interpretation"), tr("Action")});
    _doeDesignQualityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _doeDesignQualityTable->verticalHeader()->setVisible(false);
    _doeDesignQualityTable->setMaximumHeight(170);
    designBuilderLayout->addWidget(_doeDesignQualityTable);
    doeWorkflowTabs->addTab(designBuilderTab, tr("Design Builder"));

    auto* runSheetTab = new QWidget(doeWorkflowTabs);
    auto* runSheetLayout = new QVBoxLayout(runSheetTab);
    _doeDesignTable = new QTableWidget(runSheetTab);
    _doeDesignTable->setColumnCount(6);
    _doeDesignTable->setHorizontalHeaderLabels({tr("Run"), tr("Block"), tr("A: Capacity"), tr("B: Arrival"), tr("C: Priority rule"), tr("Y: Avg wait")});
    _doeDesignTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    runSheetLayout->addWidget(_doeDesignTable);
    doeWorkflowTabs->addTab(runSheetTab, tr("Run Sheet"));

    auto* modelTab = new QWidget(doeWorkflowTabs);
    auto* modelAnalysisLayout = new QHBoxLayout(modelTab);
    _doeAnovaTable = new QTableWidget(modelTab);
    _doeAnovaTable->setColumnCount(6);
    _doeAnovaTable->setHorizontalHeaderLabels({tr("Source"), tr("SS"), tr("df"), tr("MS"), tr("F-value"), tr("p-value")});
    _doeAnovaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    auto* modelText = new QTextBrowser(modelTab);
    modelText->setHtml(tr(
        "<h3>Quadratic response-surface model</h3>"
        "<p><code>AvgWait = 6.8 - 1.4A + 0.9B - 0.6C + 1.2A^2 + 0.7B^2 - 0.8AB</code></p>"
        "<p><b>Suggested interpretation:</b> capacity has the strongest main effect, arrival rate introduces curvature, and the AB interaction matters near high load.</p>"));
    modelAnalysisLayout->addWidget(_doeAnovaTable, 3);
    modelAnalysisLayout->addWidget(modelText, 2);
    doeWorkflowTabs->addTab(modelTab, tr("Model / ANOVA"));

    auto* graphTab = new QWidget(doeWorkflowTabs);
    auto* graphLayout = new QVBoxLayout(graphTab);
    auto* graphControls = new QHBoxLayout();
    _doePlotSelector = new QComboBox(graphTab);
    _doePlotSelector->addItems({tr("Contour"), tr("3D Surface"), tr("Profiler"), tr("Desirability"), tr("Diagnostics")});
    graphControls->addWidget(new QLabel(tr("Plot:"), graphTab));
    graphControls->addWidget(_doePlotSelector);
    graphControls->addStretch();
    _doePreviewPlot = new DoePreviewPlot(graphTab);
    graphLayout->addLayout(graphControls);
    graphLayout->addWidget(_doePreviewPlot, 1);
    doeWorkflowTabs->addTab(graphTab, tr("Graphs"));

    auto* optimizationTab = new QWidget(doeWorkflowTabs);
    auto* optimizationLayout = new QVBoxLayout(optimizationTab);
    _doeOptimizationTable = new QTableWidget(optimizationTab);
    _doeOptimizationTable->setColumnCount(6);
    _doeOptimizationTable->setHorizontalHeaderLabels({tr("Rank"), tr("Capacity"), tr("Arrival"), tr("Rule"), tr("Predicted wait"), tr("Desirability")});
    _doeOptimizationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _doeDesirabilityTable = new QTableWidget(optimizationTab);
    _doeDesirabilityTable->setColumnCount(6);
    _doeDesirabilityTable->setHorizontalHeaderLabels({tr("Response"), tr("Goal"), tr("Low"), tr("Target"), tr("High"), tr("Importance")});
    _doeDesirabilityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _doeDesirabilityTable->verticalHeader()->setVisible(false);
    auto* optimizationText = new QTextBrowser(optimizationTab);
    optimizationText->setMaximumHeight(96);
    optimizationText->setHtml(tr(
        "<b>Optimization preview:</b> future integration should run Genesys scenarios, fit response surfaces, then search factor settings using desirability and constraints."));
    optimizationLayout->addWidget(optimizationText);
    optimizationLayout->addWidget(new QLabel(tr("Response desirability setup:"), optimizationTab));
    optimizationLayout->addWidget(_doeDesirabilityTable, 1);
    optimizationLayout->addWidget(new QLabel(tr("Candidate solutions:"), optimizationTab));
    optimizationLayout->addWidget(_doeOptimizationTable, 1);
    doeWorkflowTabs->addTab(optimizationTab, tr("Optimization"));

    doeLayout->addWidget(doeIntro);
    doeLayout->addWidget(doeWorkflowTabs, 1);
    _workspaceTabs->addTab(doeTab, tr("DOE / RSM"));

    _reportText = new QTextEdit(_workspaceTabs);
    _reportText->setReadOnly(true);
    _workspaceTabs->addTab(_reportText, tr("Report"));

    splitter->addWidget(_workspaceTabs);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);

    connect(openButton, &QPushButton::clicked, this, [this]() { openDataset(); });
    connect(importButton, &QPushButton::clicked, this, [this]() { importSimulationResponsesSnapshot(); });
    connect(previewDesignButton, &QPushButton::clicked, this, [this]() {
        refreshDoeDemoViews();
        statusBar()->showMessage(tr("DOE preview design refreshed with demonstration runs."), 4000);
    });
    connect(runGenesysButton, &QPushButton::clicked, this, [this]() {
        showSkeletonMessage(tr("DOE execution through Genesys scenarios"));
    });
}

void DataAnalyzerWindow::connectActions() {
    connect(_openDatasetAction, &QAction::triggered, this, [this]() { openDataset(); });
    connect(_importSimulationResponsesAction, &QAction::triggered, this,
            [this]() { importSimulationResponsesSnapshot(); });
    connect(_refreshModelResponsesAction, &QAction::triggered, this, [this]() { refreshModelResponses(); });
    connect(_saveReportAction, &QAction::triggered, this, [this]() { saveReport(); });
    connect(_analysisNavigator, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < _workspaceTabs->count()) {
            _workspaceTabs->setCurrentIndex(row);
        }
    });
    connect(_scopeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        refreshAnalysisViews();
    });
    connect(_datasetsList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < _datasets.size()) {
            const int scopeIndex = _scopeCombo->findData(datasetScopeKey(row));
            if (scopeIndex >= 0) {
                _scopeCombo->setCurrentIndex(scopeIndex);
            }
        }
    });
    connect(_doePlotSelector, &QComboBox::currentTextChanged, this, [this](const QString& mode) {
        if (_doePreviewPlot != nullptr) {
            _doePreviewPlot->setMode(mode);
        }
    });
    connect(_doeDesignFamilyCombo, &QComboBox::currentTextChanged, this, [this](const QString&) {
        refreshDoeDemoViews();
    });
    connect(_doeModelOrderCombo, &QComboBox::currentTextChanged, this, [this](const QString&) {
        refreshDoePlanSummary();
    });
    connect(_doeReplicationsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        refreshDoePlanSummary();
    });
    connect(_doeAlphaSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
        refreshDoeDemoViews();
    });
    connect(_doeRandomizeCheck, &QCheckBox::toggled, this, [this](bool) {
        refreshDoePlanSummary();
    });
}

void DataAnalyzerWindow::refreshModelResponses() {
    _modelResponsesTable->setRowCount(0);
    Model* model = _simulator != nullptr && _simulator->getModelManager() != nullptr
                       ? _simulator->getModelManager()->current()
                       : nullptr;
    if (model == nullptr || model->getResponses() == nullptr) {
        statusBar()->showMessage(tr("No current Genesys model is available."), 3000);
        return;
    }

    for (SimulationResponse* response: *model->getResponses()->list()) {
        if (response == nullptr) {
            continue;
        }
        const int row = _modelResponsesTable->rowCount();
        _modelResponsesTable->insertRow(row);
        _modelResponsesTable->setItem(row, 0, readOnlyItem(QString::fromStdString(response->getElementName())));
        _modelResponsesTable->setItem(row, 1, readOnlyItem(QString::fromStdString(response->getClassname())));
        _modelResponsesTable->setItem(row, 2, readOnlyItem(QString::fromStdString(response->getName())));
        _modelResponsesTable->setItem(row, 3, readOnlyItem(QString::fromStdString(response->getValue())));
    }
    statusBar()->showMessage(tr("Model responses refreshed."), 3000);
}

void DataAnalyzerWindow::openDataset() {
    const QString initialPath = _lastDataAnalyzerPath.isEmpty() ? QDir::currentPath() : _lastDataAnalyzerPath;
    const QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open Dataset"),
            initialPath,
            tr("Data files (*.csv *.txt *.dat);;All files (*.*)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);
    if (!fileName.isEmpty()) {
        loadDatasetFromFile(fileName);
    }
}

void DataAnalyzerWindow::importSimulationResponsesSnapshot() {
    refreshModelResponses();
    Model* model = _simulator != nullptr && _simulator->getModelManager() != nullptr
                       ? _simulator->getModelManager()->current()
                       : nullptr;
    const int replication = model != nullptr && model->getSimulation() != nullptr
                                ? static_cast<int>(model->getSimulation()->getCurrentReplicationNumber())
                                : 1;
    QList<double> values;
    QList<DatasetObservation> observations;
    QStringList preview;
    for (int row = 0; row < _modelResponsesTable->rowCount(); ++row) {
        QTableWidgetItem* valueItem = _modelResponsesTable->item(row, 3);
        if (valueItem == nullptr) {
            continue;
        }
        bool ok = false;
        const double value = valueItem->text().toDouble(&ok);
        if (ok && std::isfinite(value)) {
            values.append(value);
            DatasetObservation observation;
            observation.replication = std::max(1, replication);
            observation.value = value;
            observations.append(observation);
            const QString name = _modelResponsesTable->item(row, 2) != nullptr
                                     ? _modelResponsesTable->item(row, 2)->text()
                                     : tr("response");
            preview.append(QStringLiteral("%1,%2").arg(name, formatNumber(value)));
        }
    }
    addDataset({
            tr("Current simulation responses"),
            tr("Simulation response value"),
            tr("Numeric snapshot of the current Genesys model responses."),
            tr("Continuous numeric"),
            tr("Genesys current model snapshot"),
            preview,
            values,
            true,
            observations
    });
    statusBar()->showMessage(tr("Imported %1 numeric response value(s).").arg(values.size()), 4000);
}

void DataAnalyzerWindow::loadDatasetFromFile(const QString& fileName) {
    SimulationResultsDataset parsedDataset;
    std::string errorMessage;
    if (!SimulationResultsDatasetParser::loadFromTextFile(fileName.toStdString(), &parsedDataset, &errorMessage)) {
        QMessageBox::warning(this, tr("Data Analyzer"), QString::fromStdString(errorMessage));
        return;
    }

    QList<double> numericValues;
    QList<DatasetObservation> observations;
    for (const SimulationResultsObservation& parsedObservation : parsedDataset.observations) {
        DatasetObservation observation;
        observation.replication = static_cast<int>(parsedObservation.replication);
        observation.time = parsedObservation.time;
        observation.hasTime = parsedObservation.hasTime;
        observation.value = parsedObservation.value;
        observations.append(observation);
        numericValues.append(parsedObservation.value);
    }
    QStringList previewLines;
    for (const std::string& line : parsedDataset.previewLines) {
        previewLines.append(QString::fromStdString(line));
    }
    const QString detectedVariableName = !parsedDataset.expressionName.empty()
                                             ? QString::fromStdString(parsedDataset.expressionName)
                                             : QFileInfo(fileName).completeBaseName();
    const QString detectedDescription = parsedDataset.recordFile
                                            ? tr("Genesys Record output loaded from %1; %2 replication(s) detected.")
                                                      .arg(QFileInfo(fileName).fileName())
                                                      .arg(static_cast<int>(parsedDataset.replications().size()))
                                            : tr("Raw observations loaded from %1").arg(QFileInfo(fileName).fileName());

    QDialog metadataDialog(this);
    metadataDialog.setWindowTitle(tr("Dataset metadata"));
    auto* form = new QFormLayout(&metadataDialog);
    auto* datasetName = new QLineEdit(QFileInfo(fileName).fileName(), &metadataDialog);
    auto* variableName = new QLineEdit(detectedVariableName, &metadataDialog);
    auto* variableType = new QComboBox(&metadataDialog);
    variableType->addItems({tr("Continuous numeric"), tr("Discrete numeric")});
    auto* description = new QLineEdit(detectedDescription, &metadataDialog);
    auto* detectedFormat = new QLabel(
            parsedDataset.recordFile
                    ? tr("Genesys Record file, %1 observation(s), %2 replication(s), %3 time column.")
                              .arg(numericValues.size())
                              .arg(static_cast<int>(parsedDataset.replications().size()))
                              .arg(parsedDataset.timeDependent ? tr("with") : tr("without"))
                    : tr("Numeric text file, %1 observation(s).").arg(numericValues.size()),
            &metadataDialog);
    detectedFormat->setWordWrap(true);
    form->addRow(tr("Dataset name:"), datasetName);
    form->addRow(tr("Random variable name:"), variableName);
    form->addRow(tr("Random variable type:"), variableType);
    form->addRow(tr("Description:"), description);
    form->addRow(tr("Detected format:"), detectedFormat);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &metadataDialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &metadataDialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &metadataDialog, &QDialog::reject);
    if (metadataDialog.exec() != QDialog::Accepted) {
        return;
    }

    _lastDataAnalyzerPath = QFileInfo(fileName).absolutePath();
    addDataset({
            datasetName->text().trimmed().isEmpty() ? QFileInfo(fileName).fileName() : datasetName->text().trimmed(),
            variableName->text().trimmed().isEmpty()
                ? QFileInfo(fileName).completeBaseName()
                : variableName->text().trimmed(),
            description->text().trimmed(),
            variableType->currentText(),
            fileName,
            previewLines,
            numericValues,
            true,
            observations,
            parsedDataset.recordFile,
            parsedDataset.timeDependent
    });
}

void DataAnalyzerWindow::addDataset(const DatasetDescriptor& dataset) {
    _datasets.append(dataset);
    refreshDatasetList();
    refreshScopeSelector();
    refreshAnalysisViews();
    _workspaceTabs->setCurrentIndex(2);
}

void DataAnalyzerWindow::refreshDatasetList() {
    _datasetsList->clear();
    for (const DatasetDescriptor& dataset: _datasets) {
        _datasetsList->addItem(tr("%1 | %2 | %3 values")
                               .arg(dataset.datasetName, dataset.randomVariableName)
                               .arg(dataset.values.size()));
    }
}

void DataAnalyzerWindow::refreshScopeSelector() {
    const QString previousKey = _scopeCombo->currentData().toString();
    _scopeCombo->blockSignals(true);
    _scopeCombo->clear();
    for (int datasetIndex = 0; datasetIndex < _datasets.size(); ++datasetIndex) {
        const DatasetDescriptor& dataset = _datasets.at(datasetIndex);
        _scopeCombo->addItem(dataset.datasetName, datasetScopeKey(datasetIndex));
        QSet<int> replications;
        for (const DatasetObservation& observation : dataset.observations) {
            replications.insert(observation.replication);
        }
        if (replications.size() > 1) {
            QList<int> sortedReplications = replications.values();
            std::sort(sortedReplications.begin(), sortedReplications.end());
            for (int replication : sortedReplications) {
                _scopeCombo->addItem(tr("  %1 / replication %2").arg(dataset.datasetName).arg(replication),
                                     replicationScopeKey(datasetIndex, replication));
            }
        }
    }
    _scopeCombo->addItem(tr("All datasets together"), QStringLiteral("all"));
    const int restoredIndex = previousKey.isEmpty() ? -1 : _scopeCombo->findData(previousKey);
    _scopeCombo->setCurrentIndex(restoredIndex >= 0 ? restoredIndex : _scopeCombo->count() - 1);
    _scopeCombo->blockSignals(false);
}

void DataAnalyzerWindow::showAnalysisSetupDialog(const QString& analysisName, int targetTabIndex) {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("%1 parameters").arg(analysisName));
    auto* form = new QFormLayout(&dialog);
    auto* scope = new QComboBox(&dialog);
    for (int i = 0; i < _scopeCombo->count(); ++i) {
        scope->addItem(_scopeCombo->itemText(i));
    }
    scope->setCurrentIndex(_scopeCombo->currentIndex());
    auto* confidence = new QDoubleSpinBox(&dialog);
    confidence->setRange(50.0, 99.9);
    confidence->setDecimals(1);
    confidence->setValue(95.0);
    confidence->setSuffix(QStringLiteral("%"));
    form->addRow(tr("Analysis:"), new QLabel(analysisName, &dialog));
    form->addRow(tr("Scope:"), scope);
    form->addRow(tr("Confidence level:"), confidence);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        _scopeCombo->setCurrentIndex(scope->currentIndex());
        refreshAnalysisViews();
        _workspaceTabs->setCurrentIndex(targetTabIndex);
        statusBar()->showMessage(tr("%1 configured for %2 at %3 confidence.")
                                 .arg(analysisName, _scopeCombo->currentText(), confidence->text()), 4000);
    }
}

void DataAnalyzerWindow::refreshAnalysisViews() {
    const DataSummary summary = summarizeValues(scopedValues());
    refreshStudySummaryView();
    refreshDescriptiveView(summary);
    refreshDistributionFitView(summary);
    refreshInferenceView(summary);
    refreshReportView(summary, _lastFitConclusion);
    refreshDoeDemoViews();
}

void DataAnalyzerWindow::refreshStudySummaryView() {
    if (_studySummaryTable == nullptr) {
        return;
    }

    auto addSummaryRow = [this](const QString& scope, const QString& variable, const QString& type,
                                const QString& replications, const QString& timeColumn,
                                const DataSummary& summary) {
        const int row = _studySummaryTable->rowCount();
        _studySummaryTable->insertRow(row);
        _studySummaryTable->setItem(row, 0, readOnlyItem(scope));
        _studySummaryTable->setItem(row, 1, readOnlyItem(variable));
        _studySummaryTable->setItem(row, 2, readOnlyItem(type));
        _studySummaryTable->setItem(row, 3, readOnlyItem(replications));
        _studySummaryTable->setItem(row, 4, readOnlyItem(timeColumn));
        _studySummaryTable->setItem(row, 5, readOnlyItem(QString::number(summary.count)));
        _studySummaryTable->setItem(row, 6, readOnlyItem(formatNumber(summary.mean)));
        _studySummaryTable->setItem(row, 7, readOnlyItem(formatNumber(summary.stddev)));
        _studySummaryTable->setItem(row, 8, readOnlyItem(formatNumber(summary.min)));
        _studySummaryTable->setItem(row, 9, readOnlyItem(formatNumber(summary.max)));
        _studySummaryTable->setItem(row, 10, readOnlyItem(formatNumber(summary.cv)));
    };

    _studySummaryTable->setRowCount(0);
    for (const DatasetDescriptor& dataset : _datasets) {
        QSet<int> replications;
        for (const DatasetObservation& observation : dataset.observations) {
            replications.insert(observation.replication);
        }
        addSummaryRow(dataset.datasetName, dataset.randomVariableName, dataset.variableType,
                      replications.isEmpty() ? QStringLiteral("1") : QString::number(replications.size()),
                      dataset.timeDependent ? tr("yes") : tr("no"),
                      summarizeValues(dataset.values));
        if (replications.size() > 1) {
            QList<int> sortedReplications = replications.values();
            std::sort(sortedReplications.begin(), sortedReplications.end());
            for (int replication : sortedReplications) {
                QList<double> replicationValues;
                for (const DatasetObservation& observation : dataset.observations) {
                    if (observation.replication == replication) {
                        replicationValues.append(observation.value);
                    }
                }
                addSummaryRow(tr("%1 / replication %2").arg(dataset.datasetName).arg(replication),
                              dataset.randomVariableName, dataset.variableType, QString::number(replication),
                              dataset.timeDependent ? tr("yes") : tr("no"), summarizeValues(replicationValues));
            }
        }
    }
    QList<double> pooledValues;
    for (const DatasetDescriptor& dataset : _datasets) {
        pooledValues.append(dataset.values);
    }
    addSummaryRow(tr("All datasets together"), tr("Pooled random variable values"), tr("Mixed numeric"), tr("-"), tr("-"),
                  summarizeValues(pooledValues));
}

QList<double> DataAnalyzerWindow::scopedValues() const {
    QList<double> values;
    int datasetIndex = -1;
    int replication = 0;
    const QString scopeKey = _scopeCombo != nullptr ? _scopeCombo->currentData().toString() : QStringLiteral("all");
    if (parseScopeKey(scopeKey, &datasetIndex, &replication) && datasetIndex >= 0 && datasetIndex < _datasets.size()) {
        const DatasetDescriptor& dataset = _datasets.at(datasetIndex);
        if (replication > 0 && !dataset.observations.isEmpty()) {
            for (const DatasetObservation& observation : dataset.observations) {
                if (observation.replication == replication) {
                    values.append(observation.value);
                }
            }
            return values;
        }
        return dataset.values;
    }
    for (const DatasetDescriptor& dataset: _datasets) {
        values.append(dataset.values);
    }
    return values;
}

QList<DataAnalyzerWindow::DatasetObservation> DataAnalyzerWindow::scopedObservations() const {
    QList<DatasetObservation> observations;
    int datasetIndex = -1;
    int replication = 0;
    const QString scopeKey = _scopeCombo != nullptr ? _scopeCombo->currentData().toString() : QStringLiteral("all");
    if (parseScopeKey(scopeKey, &datasetIndex, &replication) && datasetIndex >= 0 && datasetIndex < _datasets.size()) {
        const DatasetDescriptor& dataset = _datasets.at(datasetIndex);
        if (!dataset.observations.isEmpty()) {
            for (const DatasetObservation& observation : dataset.observations) {
                if (replication <= 0 || observation.replication == replication) {
                    observations.append(observation);
                }
            }
        } else {
            for (double value : dataset.values) {
                DatasetObservation observation;
                observation.value = value;
                observations.append(observation);
            }
        }
        return observations;
    }
    for (const DatasetDescriptor& dataset : _datasets) {
        if (!dataset.observations.isEmpty()) {
            observations.append(dataset.observations);
        } else {
            for (double value : dataset.values) {
                DatasetObservation observation;
                observation.value = value;
                observations.append(observation);
            }
        }
    }
    return observations;
}

QString DataAnalyzerWindow::scopedDatasetLabel() const {
    int datasetIndex = -1;
    int replication = 0;
    const QString scopeKey = _scopeCombo != nullptr ? _scopeCombo->currentData().toString() : QStringLiteral("all");
    if (parseScopeKey(scopeKey, &datasetIndex, &replication) && datasetIndex >= 0 && datasetIndex < _datasets.size()) {
        const DatasetDescriptor& dataset = _datasets.at(datasetIndex);
        if (replication > 0) {
            return tr("%1 (%2), replication %3").arg(dataset.datasetName, dataset.randomVariableName).arg(replication);
        }
        return tr("%1 (%2)").arg(dataset.datasetName, dataset.randomVariableName);
    }
    return tr("All datasets together");
}

QString DataAnalyzerWindow::scopedDatasetDescription() const {
    int datasetIndex = -1;
    int replication = 0;
    const QString scopeKey = _scopeCombo != nullptr ? _scopeCombo->currentData().toString() : QStringLiteral("all");
    if (parseScopeKey(scopeKey, &datasetIndex, &replication) && datasetIndex >= 0 && datasetIndex < _datasets.size()) {
        const DatasetDescriptor& dataset = _datasets.at(datasetIndex);
        QSet<int> replications;
        for (const DatasetObservation& observation : dataset.observations) {
            replications.insert(observation.replication);
        }
        return tr("Dataset: %1\nRandom variable: %2\nType: %3\nSource: %4\nDescription: %5\nFormat: %6, %7 replication(s), %8 time column%9")
                .arg(dataset.datasetName, dataset.randomVariableName, dataset.variableType, dataset.source,
                     dataset.description,
                     dataset.recordFile ? tr("Genesys Record") : tr("numeric text"))
                .arg(replications.isEmpty() ? 1 : replications.size())
                .arg(dataset.timeDependent ? tr("with") : tr("without"))
                .arg(replication > 0 ? tr("\nCurrent subset: replication %1").arg(replication) : QString());
    }
    return tr("Analysis Study: %1\nDatasets: %2\nScope: all datasets pooled together.")
           .arg(_studyName)
           .arg(_datasets.size());
}

QStringList DataAnalyzerWindow::scopedPreviewLines() const {
    int datasetIndex = -1;
    int replication = 0;
    const QString scopeKey = _scopeCombo != nullptr ? _scopeCombo->currentData().toString() : QStringLiteral("all");
    if (parseScopeKey(scopeKey, &datasetIndex, &replication) && datasetIndex >= 0 && datasetIndex < _datasets.size()) {
        const DatasetDescriptor& dataset = _datasets.at(datasetIndex);
        if (replication > 0 && !dataset.observations.isEmpty()) {
            QStringList lines;
            lines << tr("Replication %1 observations:").arg(replication);
            for (const DatasetObservation& observation : dataset.observations) {
                if (observation.replication != replication) {
                    continue;
                }
                if (lines.size() >= 21) {
                    break;
                }
                lines << (observation.hasTime
                                  ? tr("time=%1, value=%2").arg(formatNumber(observation.time),
                                                                formatNumber(observation.value))
                                  : tr("value=%1").arg(formatNumber(observation.value)));
            }
            return lines;
        }
        return dataset.previewLines;
    }
    QStringList lines;
    for (const DatasetDescriptor& dataset: _datasets) {
        lines << tr("=== %1 | %2 ===").arg(dataset.datasetName, dataset.randomVariableName);
        lines << dataset.previewLines;
    }
    return lines;
}

void DataAnalyzerWindow::refreshDescriptiveView(const DataSummary& summary) {
    const QList<double> values = scopedValues();
    const QList<DatasetObservation> observations = scopedObservations();
    _datasetSourceLabel->setText(scopedDatasetDescription());
    _datasetPreview->setPlainText(scopedPreviewLines().join(QStringLiteral("\n")));

    _centralTendencyTable->setRowCount(0);
    const QList<QPair<QString,QString>> centralRows = {
            {tr("Samples"), QString::number(summary.count)},
            {tr("Arithmetic mean"), formatNumber(summary.mean)},
            {tr("Median"), formatNumber(summary.median)},
            {tr("Mode"), summary.mode},
            {tr("Minimum"), formatNumber(summary.min)},
            {tr("Maximum"), formatNumber(summary.max)}
    };
    for (const auto& rowData: centralRows) {
        const int row = _centralTendencyTable->rowCount();
        _centralTendencyTable->insertRow(row);
        _centralTendencyTable->setItem(row, 0, readOnlyItem(rowData.first));
        _centralTendencyTable->setItem(row, 1, readOnlyItem(rowData.second));
    }

    _dispersionTable->setRowCount(0);
    const QList<QPair<QString,QString>> dispersionRows = {
            {tr("Variance"), formatNumber(summary.variance)},
            {tr("Standard deviation"), formatNumber(summary.stddev)},
            {tr("Coefficient of variation"), formatNumber(summary.cv)},
            {tr("First quartile"), formatNumber(summary.firstQuartile)},
            {tr("Third quartile"), formatNumber(summary.thirdQuartile)},
            {tr("Interquartile range"), formatNumber(summary.thirdQuartile - summary.firstQuartile)},
            {tr("Kurtosis"), formatNumber(summary.kurtosis)}
    };
    for (const auto& rowData: dispersionRows) {
        const int row = _dispersionTable->rowCount();
        _dispersionTable->insertRow(row);
        _dispersionTable->setItem(row, 0, readOnlyItem(rowData.first));
        _dispersionTable->setItem(row, 1, readOnlyItem(rowData.second));
    }

    _rawDataTable->setRowCount(0);
    _movingAverageTable->setRowCount(0);
    const int maxRows = std::min(static_cast<int>(values.size()), 200);
    for (int i = 0; i < maxRows; ++i) {
        int row = _rawDataTable->rowCount();
        _rawDataTable->insertRow(row);
        _rawDataTable->setItem(row, 0, readOnlyItem(QString::number(i + 1)));
        if (i < observations.size()) {
            const DatasetObservation& observation = observations.at(i);
            _rawDataTable->setItem(row, 1, readOnlyItem(QString::number(observation.replication)));
            _rawDataTable->setItem(row, 2, readOnlyItem(observation.hasTime ? formatNumber(observation.time) : tr("n/a")));
            _rawDataTable->setItem(row, 3, readOnlyItem(formatNumber(observation.value)));
        } else {
            _rawDataTable->setItem(row, 1, readOnlyItem(tr("n/a")));
            _rawDataTable->setItem(row, 2, readOnlyItem(tr("n/a")));
            _rawDataTable->setItem(row, 3, readOnlyItem(formatNumber(values.at(i))));
        }

        const int first = std::max(0, i - 4);
        double sum = 0.0;
        for (int j = first; j <= i; ++j) {
            sum += values.at(j);
        }
        row = _movingAverageTable->rowCount();
        _movingAverageTable->insertRow(row);
        _movingAverageTable->setItem(row, 0, readOnlyItem(QString::number(i + 1)));
        _movingAverageTable->setItem(row, 1, readOnlyItem(formatNumber(sum / static_cast<double>(i - first + 1))));
    }
    _histogramPreview->setValues(values);
}

void DataAnalyzerWindow::refreshDistributionFitView(const DataSummary&) {
    _fitTable->setRowCount(0);
    const QList<double> values = scopedValues();
    _lastFitConclusion = tr("Distribution fitting requires at least two numeric samples.");
    if (values.size() >= 2) {
        QTemporaryFile binaryDataset(QDir::tempPath() + QStringLiteral("/genesys-data-analyzer-XXXXXX.bin"));
        if (binaryDataset.open()) {
            for (double value: values) {
                binaryDataset.write(reinterpret_cast<const char*>(&value), static_cast<qint64>(sizeof(double)));
            }
            binaryDataset.flush();

            FitterDefaultImpl fitter;
            fitter.setDataFilename(binaryDataset.fileName().toStdString());
            double error = std::numeric_limits<double>::infinity();
            double p1 = 0.0;
            double p2 = 0.0;
            double p3 = 0.0;
            double p4 = 0.0;
            std::string bestName;

            auto kolmogorovStatistic = [&values](const std::function<double(double)>& cdf) {
                const QList<double> sorted = DataAnalyzerWindow::sortedValues(values);
                double statistic = 0.0;
                for (int i = 0; i < sorted.size(); ++i) {
                    const double rawTheoretical = cdf(sorted.at(i));
                    if (!std::isfinite(rawTheoretical)) {
                        return std::numeric_limits<double>::quiet_NaN();
                    }
                    const double theoretical = std::max(0.0, std::min(1.0, rawTheoretical));
                    statistic = std::max(
                            statistic, std::fabs(static_cast<double>(i + 1) / sorted.size() - theoretical));
                    statistic = std::max(statistic, std::fabs(theoretical - static_cast<double>(i) / sorted.size()));
                }
                return statistic;
            };
            auto chiSquareStatistic = [&values](const std::function<double(double)>& cdf, int parameters) {
                auto minmax = std::minmax_element(values.begin(), values.end());
                const double minValue = *minmax.first;
                const double maxValue = *minmax.second;
                const int bins = std::min(
                        10, std::max(4, static_cast<int>(std::sqrt(static_cast<double>(values.size())))));
                const double range = maxValue - minValue;
                if (!(range > 0.0)) {
                    return QPair<double,int>(std::numeric_limits<double>::quiet_NaN(), 0);
                }
                QVector<int> observed(bins, 0);
                for (double value: values) {
                    int bin = static_cast<int>(((value - minValue) / range) * bins);
                    bin = std::min(bins - 1, std::max(0, bin));
                    observed[bin]++;
                }
                double statistic = 0.0;
                int usableBins = 0;
                for (int i = 0; i < bins; ++i) {
                    const double lower = minValue + range * static_cast<double>(i) / bins;
                    const double upper = minValue + range * static_cast<double>(i + 1) / bins;
                    const double upperProbability = cdf(upper);
                    const double lowerProbability = cdf(lower);
                    if (!std::isfinite(upperProbability) || !std::isfinite(lowerProbability)) {
                        return QPair<double,int>(std::numeric_limits<double>::quiet_NaN(), 0);
                    }
                    const double expectedProbability = std::max(0.0, upperProbability - lowerProbability);
                    const double expected = expectedProbability * values.size();
                    if (expected >= 1e-9) {
                        const double diff = observed.at(i) - expected;
                        statistic += (diff * diff) / expected;
                        usableBins++;
                    }
                }
                return QPair<double,int>(statistic, std::max(1, usableBins - 1 - parameters));
            };
            auto addFitRow = [&](const QString& distribution, const QString& parameters, double fitError,
                                 const std::function<double(double)>& cdf, int parameterCount) {
                double chiP = std::numeric_limits<double>::quiet_NaN();
                double ksP = std::numeric_limits<double>::quiet_NaN();
                if (std::isfinite(fitError)) {
                    const QPair<double,int> chi = chiSquareStatistic(cdf, parameterCount);
                    chiP = approximateChiSquareSurvival(chi.first, chi.second);
                    ksP = approximateKolmogorovPValue(kolmogorovStatistic(cdf), values.size());
                }
                const int row = _fitTable->rowCount();
                _fitTable->insertRow(row);
                _fitTable->setItem(row, 0, readOnlyItem(distribution));
                _fitTable->setItem(row, 1, readOnlyItem(parameters));
                _fitTable->setItem(row, 2, readOnlyItem(formatNumber(fitError)));
                _fitTable->setItem(row, 3, readOnlyItem(formatNumber(chiP)));
                _fitTable->setItem(row, 4, readOnlyItem(fitDecisionText(chiP)));
                _fitTable->setItem(row, 5, readOnlyItem(formatNumber(ksP)));
                _fitTable->setItem(row, 6, readOnlyItem(fitDecisionText(ksP)));
            };

            fitter.fitUniform(&error, &p1, &p2);
            addFitRow(tr("Uniform"), tr("min=%1, max=%2").arg(formatNumber(p1), formatNumber(p2)), error,
                      [=](double x) { return x <= p1 ? 0.0 : (x >= p2 ? 1.0 : (x - p1) / (p2 - p1)); }, 2);
            fitter.fitTriangular(&error, &p1, &p2, &p3);
            addFitRow(tr("Triangular"),
                      tr("min=%1, mode=%2, max=%3").arg(formatNumber(p1), formatNumber(p2), formatNumber(p3)), error,
                      [=](double x) { return triangularCdf(x, p1, p2, p3); }, 3);
            fitter.fitNormal(&error, &p1, &p2);
            addFitRow(tr("Normal"), tr("mean=%1, stddev=%2").arg(formatNumber(p1), formatNumber(p2)), error,
                      [=](double x) { return normalCdf(x, p1, p2); }, 2);
            fitter.fitExpo(&error, &p1);
            addFitRow(tr("Exponential"), tr("mean=%1").arg(formatNumber(p1)), error,
                      [=](double x) { return x < 0.0 ? 0.0 : 1.0 - std::exp(-x / p1); }, 1);
            fitter.fitErlang(&error, &p1, &p2);
            addFitRow(tr("Erlang"), tr("mean=%1, m=%2").arg(formatNumber(p1), formatNumber(p2)), error,
                      [=](double x) { return erlangCdf(x, p1, static_cast<int>(std::llround(p2))); }, 2);
            fitter.fitBeta(&error, &p1, &p2, &p3, &p4);
            addFitRow(tr("Beta"), tr("alpha=%1, beta=%2, inf=%3, sup=%4").arg(
                              formatNumber(p1), formatNumber(p2), formatNumber(p3), formatNumber(p4)), error,
                      [](double) { return std::numeric_limits<double>::quiet_NaN(); }, 4);
            fitter.fitWeibull(&error, &p1, &p2);
            addFitRow(tr("Weibull"), tr("shape=%1, scale=%2").arg(formatNumber(p1), formatNumber(p2)), error,
                      [=](double x) { return x < 0.0 ? 0.0 : 1.0 - std::exp(-std::pow(x / p2, p1)); }, 2);
            fitter.fitAll(&error, &bestName);
            _lastFitConclusion = tr("Best current fit by SSE: %1 (SSE=%2). Normality check at 95%%: %3.")
                    .arg(QString::fromStdString(bestName), formatNumber(error),
                         fitter.isNormalDistributed(0.95) ? tr("accepted") : tr("not accepted"));
        }
        else {
            _lastFitConclusion = tr("Could not create a temporary binary dataset for the fitter.");
        }
    }
    _fitConclusionLabel->setText(_lastFitConclusion);
}

void DataAnalyzerWindow::refreshInferenceView(const DataSummary& summary) {
    _inferenceTable->setRowCount(0);
    if (scopedValues().size() < 2) {
        return;
    }
    HypothesisTesterDefaultImpl1 tester;
    auto addInferenceRow = [this](const QString& measure, HypothesisTester_if::ConfidenceInterval interval) {
        const int row = _inferenceTable->rowCount();
        _inferenceTable->insertRow(row);
        _inferenceTable->setItem(row, 0, readOnlyItem(measure));
        _inferenceTable->setItem(row, 1, readOnlyItem(formatNumber(interval.inferiorLimit())));
        _inferenceTable->setItem(row, 2, readOnlyItem(formatNumber(interval.superiorLimit())));
        _inferenceTable->setItem(row, 3, readOnlyItem(formatNumber(interval.halfWidth())));
    };
    addInferenceRow(tr("Mean, 95% confidence"),
                    tester.averageConfidenceInterval(summary.mean, summary.stddev,
                                                     static_cast<unsigned int>(summary.count), 0.95));
    addInferenceRow(tr("Variance, 95% confidence"),
                    tester.varianceConfidenceInterval(summary.variance, static_cast<unsigned int>(summary.count),
                                                      0.95));
}

void DataAnalyzerWindow::refreshReportView(const DataSummary& summary, const QString& fitConclusion) {
    QString text;
    if (_datasets.isEmpty()) {
        text = tr("No dataset loaded.\n\nUse File/Open Dataset or import current Genesys simulation responses.");
    }
    else {
        QTextStream stream(&text);
        stream << tr("Analysis Study: %1\n").arg(_studyName);
        stream << tr("Scope: %1\n").arg(scopedDatasetLabel());
        stream << scopedDatasetDescription() << "\n";
        stream << tr("Numeric samples: %1\n\n").arg(summary.count);
        stream << tr("Minimum: %1\n").arg(formatNumber(summary.min));
        stream << tr("Maximum: %1\n").arg(formatNumber(summary.max));
        stream << tr("Mean: %1\n").arg(formatNumber(summary.mean));
        stream << tr("Median: %1\n").arg(formatNumber(summary.median));
        stream << tr("Mode: %1\n").arg(summary.mode);
        stream << tr("Variance: %1\n").arg(formatNumber(summary.variance));
        stream << tr("Standard deviation: %1\n").arg(formatNumber(summary.stddev));
        stream << tr("Coefficient of variation: %1\n").arg(formatNumber(summary.cv));
        stream << tr("Q1: %1\n").arg(formatNumber(summary.firstQuartile));
        stream << tr("Q3: %1\n").arg(formatNumber(summary.thirdQuartile));
        stream << tr("Kurtosis: %1\n\n").arg(formatNumber(summary.kurtosis));
        stream << fitConclusion << "\n\n";
        stream << tr("Next planned analyses: regression, ANOVA, design of experiments and response surfaces.");
    }
    _reportText->setPlainText(text);
}

void DataAnalyzerWindow::refreshDoePlanSummary() {
    if (_doePlanSummaryLabel == nullptr) {
        return;
    }
    const QString designFamily = _doeDesignFamilyCombo != nullptr
                                     ? _doeDesignFamilyCombo->currentText()
                                     : tr("Central composite RSM");
    const QString modelOrder = _doeModelOrderCombo != nullptr
                                   ? _doeModelOrderCombo->currentText()
                                   : tr("Quadratic");
    const int replications = _doeReplicationsSpin != nullptr ? _doeReplicationsSpin->value() : 1;
    const bool randomized = _doeRandomizeCheck != nullptr && _doeRandomizeCheck->isChecked();
    const int designPoints = _doeDesignTable != nullptr ? _doeDesignTable->rowCount() : 0;
    const double alpha = _doeAlphaSpin != nullptr ? _doeAlphaSpin->value() : 0.05;
    _doePlanSummaryLabel->setText(tr(
            "<b>Current DOE plan:</b> %1 with %2 model terms, %3 design point(s), %4 replication(s) per point, "
            "%5 run order and alpha=%6. This preview is ready to become a Genesys experiment plan once controls, responses, "
            "scenario execution and result collectors are wired to the tool layer.")
            .arg(designFamily, modelOrder)
            .arg(designPoints)
            .arg(replications)
            .arg(randomized ? tr("randomized") : tr("fixed"))
            .arg(formatNumber(alpha)));

    if (_doeAdvisorLabel != nullptr) {
        QString recommendation;
        if (designFamily == tr("Fractional factorial screening")) {
            recommendation = tr("Advisor: use this screening design to identify active controls before fitting curvature.");
        } else if (designFamily == tr("Full factorial")) {
            recommendation = tr("Advisor: use this when the number of factors is small and interaction clarity is more important than run economy.");
        } else if (designFamily == tr("Box-Behnken RSM")) {
            recommendation = tr("Advisor: use this RSM design when avoiding extreme corner combinations is useful.");
        } else if (designFamily == tr("Custom Genesys experiment")) {
            recommendation = tr("Advisor: use this to preserve domain-specific scenarios and later compare them with generated designs.");
        } else {
            recommendation = tr("Advisor: central composite RSM is the current default for estimating curvature and finding robust optima.");
        }
        _doeAdvisorLabel->setText(recommendation);
    }

    if (_doeAdviceTable == nullptr) {
        return;
    }
    auto setRow = [](QTableWidget* table, const QStringList& values) {
        const int row = table->rowCount();
        table->insertRow(row);
        for (int col = 0; col < values.size(); ++col) {
            table->setItem(row, col, readOnlyItem(values.at(col)));
        }
    };

    _doeAdviceTable->setRowCount(0);
    setRow(_doeAdviceTable, {
            tr("1. Define design"),
            tr("%1, %2").arg(designFamily, modelOrder),
            tr("Map Genesys controls to factors and responses to measured outputs")
    });
    setRow(_doeAdviceTable, {
            tr("2. Generate runs"),
            tr("%1 preview runs, %2 replication(s)").arg(designPoints).arg(replications),
            tr("Create scenario variants and replicate each run through Simulator instances")
    });
    setRow(_doeAdviceTable, {
            tr("3. Fit model"),
            tr("Demonstration ANOVA and quadratic equation"),
            tr("Route collected responses into regression, ANOVA and lack-of-fit services")
    });
    setRow(_doeAdviceTable, {
            tr("4. Optimize"),
            tr("Desirability-ranked candidate settings"),
            tr("Connect objectives, constraints and confirmation runs")
    });
}

void DataAnalyzerWindow::refreshDoeDemoViews() {
    if (_doeFactorsTable == nullptr || _doeResponsesTable == nullptr || _doeDesignTable == nullptr
        || _doeAnovaTable == nullptr || _doeOptimizationTable == nullptr
        || _doeDesignQualityTable == nullptr || _doeDesirabilityTable == nullptr) {
        return;
    }

    auto setRow = [](QTableWidget* table, const QStringList& values) {
        const int row = table->rowCount();
        table->insertRow(row);
        for (int col = 0; col < values.size(); ++col) {
            table->setItem(row, col, readOnlyItem(values.at(col)));
        }
    };

    _doeFactorsTable->setRowCount(0);
    setRow(_doeFactorsTable, {tr("A: Service capacity"), tr("Control"), tr("2"), tr("4"), tr("6")});
    setRow(_doeFactorsTable, {tr("B: Arrival rate"), tr("Control"), tr("4.0"), tr("6.5"), tr("9.0")});
    setRow(_doeFactorsTable, {tr("C: Priority rule"), tr("Categorical"), tr("FIFO"), tr("SPT"), tr("EDD")});

    _doeResponsesTable->setRowCount(0);
    setRow(_doeResponsesTable, {tr("Average waiting time"), tr("Minimize"), tr("0.0"), tr("8.0")});
    setRow(_doeResponsesTable, {tr("Throughput"), tr("Maximize"), tr("80"), tr("120")});
    setRow(_doeResponsesTable, {tr("Resource utilization"), tr("Target"), tr("0.65"), tr("0.85")});

    _doeDesignTable->setRowCount(0);
    QVector<QStringList> runRows;
    const QString designFamily = _doeDesignFamilyCombo != nullptr
                                     ? _doeDesignFamilyCombo->currentText()
                                     : tr("Central composite RSM");
    if (designFamily == tr("Fractional factorial screening")) {
        runRows = {
            {QStringLiteral("1"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("4.0"), QStringLiteral("FIFO"), QStringLiteral("12.4")},
            {QStringLiteral("2"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("4.0"), QStringLiteral("SPT"), QStringLiteral("4.9")},
            {QStringLiteral("3"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("9.0"), QStringLiteral("SPT"), QStringLiteral("16.8")},
            {QStringLiteral("4"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("9.0"), QStringLiteral("FIFO"), QStringLiteral("9.7")},
            {QStringLiteral("5"), QStringLiteral("1"), QStringLiteral("4"), QStringLiteral("6.5"), QStringLiteral("EDD"), QStringLiteral("7.4")},
            {QStringLiteral("6"), QStringLiteral("1"), QStringLiteral("4"), QStringLiteral("8.0"), QStringLiteral("FIFO"), QStringLiteral("10.9")}
        };
    } else if (designFamily == tr("Full factorial")) {
        runRows = {
            {QStringLiteral("1"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("4.0"), QStringLiteral("FIFO"), QStringLiteral("11.8")},
            {QStringLiteral("2"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("4.0"), QStringLiteral("SPT"), QStringLiteral("10.5")},
            {QStringLiteral("3"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("9.0"), QStringLiteral("FIFO"), QStringLiteral("18.2")},
            {QStringLiteral("4"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("9.0"), QStringLiteral("SPT"), QStringLiteral("17.4")},
            {QStringLiteral("5"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("4.0"), QStringLiteral("FIFO"), QStringLiteral("5.2")},
            {QStringLiteral("6"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("4.0"), QStringLiteral("SPT"), QStringLiteral("4.7")},
            {QStringLiteral("7"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("9.0"), QStringLiteral("FIFO"), QStringLiteral("9.3")},
            {QStringLiteral("8"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("9.0"), QStringLiteral("SPT"), QStringLiteral("8.9")}
        };
    } else if (designFamily == tr("Box-Behnken RSM")) {
        runRows = {
            {QStringLiteral("1"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("4.0"), QStringLiteral("SPT"), QStringLiteral("10.7")},
            {QStringLiteral("2"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("4.0"), QStringLiteral("SPT"), QStringLiteral("5.1")},
            {QStringLiteral("3"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("9.0"), QStringLiteral("SPT"), QStringLiteral("17.4")},
            {QStringLiteral("4"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("9.0"), QStringLiteral("SPT"), QStringLiteral("8.9")},
            {QStringLiteral("5"), QStringLiteral("2"), QStringLiteral("4"), QStringLiteral("4.0"), QStringLiteral("FIFO"), QStringLiteral("8.2")},
            {QStringLiteral("6"), QStringLiteral("2"), QStringLiteral("4"), QStringLiteral("9.0"), QStringLiteral("EDD"), QStringLiteral("7.6")},
            {QStringLiteral("7"), QStringLiteral("2"), QStringLiteral("2"), QStringLiteral("6.5"), QStringLiteral("EDD"), QStringLiteral("13.8")},
            {QStringLiteral("8"), QStringLiteral("2"), QStringLiteral("6"), QStringLiteral("6.5"), QStringLiteral("FIFO"), QStringLiteral("6.2")},
            {QStringLiteral("9"), QStringLiteral("2"), QStringLiteral("4"), QStringLiteral("6.5"), QStringLiteral("SPT"), QStringLiteral("6.6")}
        };
    } else if (designFamily == tr("Custom Genesys experiment")) {
        runRows = {
            {QStringLiteral("1"), QStringLiteral("baseline"), QStringLiteral("4"), QStringLiteral("6.5"), QStringLiteral("FIFO"), QStringLiteral("7.8")},
            {QStringLiteral("2"), QStringLiteral("capacity"), QStringLiteral("5"), QStringLiteral("6.5"), QStringLiteral("FIFO"), QStringLiteral("6.7")},
            {QStringLiteral("3"), QStringLiteral("policy"), QStringLiteral("5"), QStringLiteral("6.5"), QStringLiteral("SPT"), QStringLiteral("5.9")},
            {QStringLiteral("4"), QStringLiteral("stress"), QStringLiteral("5"), QStringLiteral("8.0"), QStringLiteral("SPT"), QStringLiteral("8.3")},
            {QStringLiteral("5"), QStringLiteral("confirm"), QStringLiteral("6"), QStringLiteral("5.5"), QStringLiteral("EDD"), QStringLiteral("6.1")}
        };
    } else {
        runRows = {
            {QStringLiteral("1"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("4.0"), QStringLiteral("FIFO"), QStringLiteral("11.8")},
            {QStringLiteral("2"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("4.0"), QStringLiteral("FIFO"), QStringLiteral("5.2")},
            {QStringLiteral("3"), QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("9.0"), QStringLiteral("SPT"), QStringLiteral("17.4")},
            {QStringLiteral("4"), QStringLiteral("1"), QStringLiteral("6"), QStringLiteral("9.0"), QStringLiteral("SPT"), QStringLiteral("8.9")},
            {QStringLiteral("5"), QStringLiteral("2"), QStringLiteral("4"), QStringLiteral("6.5"), QStringLiteral("EDD"), QStringLiteral("7.1")},
            {QStringLiteral("6"), QStringLiteral("2"), QStringLiteral("4"), QStringLiteral("6.5"), QStringLiteral("FIFO"), QStringLiteral("7.8")},
            {QStringLiteral("7"), QStringLiteral("2"), QStringLiteral("4"), QStringLiteral("6.5"), QStringLiteral("SPT"), QStringLiteral("6.6")},
            {QStringLiteral("8"), QStringLiteral("2"), QStringLiteral("6"), QStringLiteral("6.5"), QStringLiteral("EDD"), QStringLiteral("5.9")},
            {QStringLiteral("9"), QStringLiteral("2"), QStringLiteral("3"), QStringLiteral("8.0"), QStringLiteral("FIFO"), QStringLiteral("12.6")}
        };
    }
    for (const QStringList& row : runRows) {
        setRow(_doeDesignTable, row);
    }

    _doeDesignQualityTable->setRowCount(0);
    const int runs = runRows.size();
    const int replications = _doeReplicationsSpin != nullptr ? _doeReplicationsSpin->value() : 1;
    const double alpha = _doeAlphaSpin != nullptr ? _doeAlphaSpin->value() : 0.05;
    if (designFamily == tr("Fractional factorial screening")) {
        setRow(_doeDesignQualityTable, {tr("Run economy"), tr("%1 base runs").arg(runs), tr("High"), tr("Screen factors, then augment to RSM")});
        setRow(_doeDesignQualityTable, {tr("Aliasing risk"), tr("Moderate"), tr("Main effects are prioritized"), tr("Inspect active effects before interpreting interactions")});
        setRow(_doeDesignQualityTable, {tr("Power at alpha"), tr("%1 at alpha=%2").arg(formatNumber(0.78 + 0.02 * replications), formatNumber(alpha)), tr("Preview"), tr("Increase replications if weak effects matter")});
        setRow(_doeDesignQualityTable, {tr("Curvature estimate"), tr("Limited"), tr("Screening only"), tr("Add center and axial points for RSM")});
    } else if (designFamily == tr("Full factorial")) {
        setRow(_doeDesignQualityTable, {tr("Interaction clarity"), tr("Strong"), tr("All low/high combinations are visible"), tr("Use when factor count is controlled")});
        setRow(_doeDesignQualityTable, {tr("Run economy"), tr("%1 base runs").arg(runs), tr("Moderate"), tr("Fractionate if many controls are selected")});
        setRow(_doeDesignQualityTable, {tr("Power at alpha"), tr("%1 at alpha=%2").arg(formatNumber(0.82 + 0.015 * replications), formatNumber(alpha)), tr("Preview"), tr("Confirm with actual replication variance")});
        setRow(_doeDesignQualityTable, {tr("Curvature estimate"), tr("Needs center points"), tr("Linear/interactions first"), tr("Augment before response-surface optimization")});
    } else if (designFamily == tr("Box-Behnken RSM")) {
        setRow(_doeDesignQualityTable, {tr("Quadratic model support"), tr("Good"), tr("Curvature without corner extremes"), tr("Use for safer simulation stress regions")});
        setRow(_doeDesignQualityTable, {tr("Prediction variance"), tr("Balanced center region"), tr("Preview"), tr("Add axial points if edge prediction matters")});
        setRow(_doeDesignQualityTable, {tr("Adeq precision"), tr("15.7"), tr("Strong signal preview"), tr("Validate after real simulation runs")});
        setRow(_doeDesignQualityTable, {tr("Lack-of-fit protection"), tr("%1 center-like runs").arg(std::max(1, replications)), tr("Preview"), tr("Keep pure-error degrees of freedom")});
    } else if (designFamily == tr("Custom Genesys experiment")) {
        setRow(_doeDesignQualityTable, {tr("Domain fidelity"), tr("High"), tr("Runs preserve named scenarios"), tr("Use as baseline or confirmation set")});
        setRow(_doeDesignQualityTable, {tr("Model estimability"), tr("Depends on coverage"), tr("Preview"), tr("Add generated points around gaps")});
        setRow(_doeDesignQualityTable, {tr("Prediction variance"), tr("Uneven"), tr("Custom designs need diagnostics"), tr("Evaluate leverage before optimization")});
        setRow(_doeDesignQualityTable, {tr("Design augmentation"), tr("Recommended"), tr("Hybrid path"), tr("Append RSM points after scenario review")});
    } else {
        setRow(_doeDesignQualityTable, {tr("Quadratic model support"), tr("Strong"), tr("Main effects, interactions and curvature"), tr("Default for RSM optimization")});
        setRow(_doeDesignQualityTable, {tr("Rotatability"), tr("Approximate"), tr("Good graphical exploration"), tr("Tune axial distance in backend phase")});
        setRow(_doeDesignQualityTable, {tr("Adeq precision"), tr("17.3"), tr("Strong signal preview"), tr("Replace with fitted-model metric later")});
        setRow(_doeDesignQualityTable, {tr("Predicted R-squared"), tr("0.86"), tr("Good demonstration fit"), tr("Compute from actual holdout/PRESS later")});
    }

    _doeAnovaTable->setRowCount(0);
    setRow(_doeAnovaTable, {tr("Model"), QStringLiteral("186.42"), QStringLiteral("6"), QStringLiteral("31.07"), QStringLiteral("18.9"), QStringLiteral("0.0021")});
    setRow(_doeAnovaTable, {tr("A-Capacity"), QStringLiteral("74.31"), QStringLiteral("1"), QStringLiteral("74.31"), QStringLiteral("45.2"), QStringLiteral("0.0007")});
    setRow(_doeAnovaTable, {tr("B-Arrival"), QStringLiteral("38.74"), QStringLiteral("1"), QStringLiteral("38.74"), QStringLiteral("23.6"), QStringLiteral("0.0038")});
    setRow(_doeAnovaTable, {tr("AB"), QStringLiteral("18.66"), QStringLiteral("1"), QStringLiteral("18.66"), QStringLiteral("11.3"), QStringLiteral("0.0152")});
    setRow(_doeAnovaTable, {tr("Residual"), QStringLiteral("13.15"), QStringLiteral("8"), QStringLiteral("1.64"), QStringLiteral(""), QStringLiteral("")});
    setRow(_doeAnovaTable, {tr("Lack of fit"), QStringLiteral("4.10"), QStringLiteral("3"), QStringLiteral("1.37"), QStringLiteral("0.76"), QStringLiteral("0.5530")});

    _doeOptimizationTable->setRowCount(0);
    _doeDesirabilityTable->setRowCount(0);
    setRow(_doeDesirabilityTable, {tr("Average waiting time"), tr("Minimize"), tr("0.0"), tr("5.0"), tr("8.0"), tr("5")});
    setRow(_doeDesirabilityTable, {tr("Throughput"), tr("Maximize"), tr("80"), tr("115"), tr("130"), tr("4")});
    setRow(_doeDesirabilityTable, {tr("Resource utilization"), tr("Target"), tr("0.65"), tr("0.82"), tr("0.90"), tr("3")});
    setRow(_doeDesirabilityTable, {tr("Service stability"), tr("Maximize"), tr("0.70"), tr("0.90"), tr("1.00"), tr("2")});

    setRow(_doeOptimizationTable, {QStringLiteral("1"), QStringLiteral("5.7"), QStringLiteral("5.1"), QStringLiteral("SPT"), QStringLiteral("5.8"), QStringLiteral("0.83")});
    setRow(_doeOptimizationTable, {QStringLiteral("2"), QStringLiteral("6.0"), QStringLiteral("5.5"), QStringLiteral("EDD"), QStringLiteral("6.1"), QStringLiteral("0.81")});
    setRow(_doeOptimizationTable, {QStringLiteral("3"), QStringLiteral("5.4"), QStringLiteral("4.9"), QStringLiteral("FIFO"), QStringLiteral("6.4"), QStringLiteral("0.76")});
    setRow(_doeOptimizationTable, {QStringLiteral("4"), QStringLiteral("5.9"), QStringLiteral("6.0"), QStringLiteral("SPT"), QStringLiteral("6.7"), QStringLiteral("0.74")});

    if (_doePreviewPlot != nullptr && _doePlotSelector != nullptr) {
        _doePreviewPlot->setMode(_doePlotSelector->currentText());
    }
    refreshDoePlanSummary();
}

void DataAnalyzerWindow::saveReport() {
    const QString exportPath = QFileDialog::getSaveFileName(
            this,
            tr("Save Data Analyzer Report"),
            QDir::currentPath() + QStringLiteral("/data-analyzer-report.txt"),
            tr("Text files (*.txt);;All files (*.*)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);
    if (exportPath.isEmpty()) {
        return;
    }
    QFile outputFile(exportPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Data Analyzer"), tr("Could not save report file."));
        return;
    }
    QTextStream out(&outputFile);
    out << _reportText->toPlainText() << "\n";
    statusBar()->showMessage(tr("Data Analyzer report saved to %1").arg(exportPath), 4000);
}

void DataAnalyzerWindow::showSkeletonMessage(const QString& featureName) {
    statusBar()->showMessage(tr("%1 interface is reserved for the next Data Analyzer iteration.").arg(featureName),
                             4000);
}

DataAnalyzerWindow::DataSummary DataAnalyzerWindow::summarizeValues(const QList<double>& values) {
    DataSummary summary;
    summary.count = values.size();
    if (values.isEmpty()) {
        summary.mode = QObject::tr("n/a");
        return summary;
    }
    const QList<double> sorted = sortedValues(values);
    summary.min = values.first();
    summary.max = values.first();
    double sum = 0.0;
    for (double value: values) {
        summary.min = std::min(summary.min, value);
        summary.max = std::max(summary.max, value);
        sum += value;
    }
    summary.mean = sum / static_cast<double>(values.size());
    summary.median = percentile(sorted, 0.5);
    summary.firstQuartile = percentile(sorted, 0.25);
    summary.thirdQuartile = percentile(sorted, 0.75);
    summary.mode = exactMode(values);
    double squaredDiffSum = 0.0;
    double fourthMomentSum = 0.0;
    for (double value: values) {
        const double diff = value - summary.mean;
        squaredDiffSum += diff * diff;
        fourthMomentSum += diff * diff * diff * diff;
    }
    summary.variance = values.size() > 1 ? squaredDiffSum / static_cast<double>(values.size() - 1) : 0.0;
    summary.stddev = std::sqrt(summary.variance);
    summary.cv = summary.mean != 0.0 ? summary.stddev / summary.mean : 0.0;
    summary.kurtosis = summary.stddev > 0.0
                           ? fourthMomentSum / (static_cast<double>(values.size()) * std::pow(summary.stddev, 4.0))
                           : 0.0;
    return summary;
}

QList<double> DataAnalyzerWindow::sortedValues(const QList<double>& values) {
    QList<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    return sorted;
}

double DataAnalyzerWindow::percentile(const QList<double>& sorted, double probability) {
    if (sorted.isEmpty()) {
        return 0.0;
    }
    if (sorted.size() == 1) {
        return sorted.first();
    }
    const double position = std::max(0.0, std::min(1.0, probability)) * static_cast<double>(sorted.size() - 1);
    const int lower = static_cast<int>(std::floor(position));
    const int upper = static_cast<int>(std::ceil(position));
    const double fraction = position - lower;
    return sorted.at(lower) * (1.0 - fraction) + sorted.at(upper) * fraction;
}

QString DataAnalyzerWindow::exactMode(const QList<double>& values) {
    if (values.isEmpty()) {
        return QObject::tr("n/a");
    }
    QMap<QString,int> frequencies;
    for (double value: values) {
        frequencies[formatNumber(value)]++;
    }
    int bestFrequency = 0;
    QStringList modes;
    for (auto it = frequencies.cbegin(); it != frequencies.cend(); ++it) {
        if (it.value() > bestFrequency) {
            bestFrequency = it.value();
            modes = {it.key()};
        }
        else if (it.value() == bestFrequency) {
            modes.append(it.key());
        }
    }
    if (bestFrequency <= 1) {
        return QObject::tr("no repeated exact value");
    }
    if (modes.size() > 4) {
        return QObject::tr("%1 values tied, frequency %2").arg(modes.size()).arg(bestFrequency);
    }
    return QObject::tr("%1 (frequency %2)").arg(modes.join(QStringLiteral(", "))).arg(bestFrequency);
}

QString DataAnalyzerWindow::formatNumber(double value) {
    if (!std::isfinite(value)) {
        return QObject::tr("n/a");
    }
    return QString::number(value, 'g', 8);
}
