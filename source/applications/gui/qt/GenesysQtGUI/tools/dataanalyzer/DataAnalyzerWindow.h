#ifndef DATAANALYZERWINDOW_H
#define DATAANALYZERWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QString>
#include <QStringList>

class QGroupBox;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QSpinBox;
class QSplitter;
class QTableWidget;
class QTabWidget;
class QTextEdit;
class QTextBrowser;
class Simulator;

class DataAnalyzerWindow : public QMainWindow {
public:
    explicit DataAnalyzerWindow(Simulator* simulator, QString& lastDataAnalyzerPath, QWidget* parent = nullptr);

private:
    struct DataSummary {
        int count = 0;
        double min = 0.0;
        double max = 0.0;
        double mean = 0.0;
        double median = 0.0;
        QString mode;
        double firstQuartile = 0.0;
        double thirdQuartile = 0.0;
        double variance = 0.0;
        double stddev = 0.0;
        double cv = 0.0;
        double kurtosis = 0.0;
    };

    struct DatasetObservation {
        int replication = 1;
        double time = 0.0;
        bool hasTime = false;
        double value = 0.0;
    };

    struct DatasetDescriptor {
        QString datasetName;
        QString randomVariableName;
        QString description;
        QString variableType;
        QString source;
        QStringList previewLines;
        QList<double> values;
        bool raw = true;
        QList<DatasetObservation> observations;
        bool recordFile = false;
        bool timeDependent = false;
    };

    class HistogramPreview;
    class DoePreviewPlot;

    void buildMenus();
    void buildWorkspace();
    void connectActions();
    void refreshModelResponses();
    void openDataset();
    void importSimulationResponsesSnapshot();
    void loadDatasetFromFile(const QString& fileName);
    void addDataset(const DatasetDescriptor& dataset);
    void refreshDatasetList();
    void refreshScopeSelector();
    void showAnalysisSetupDialog(const QString& analysisName, int targetTabIndex);
    void refreshAnalysisViews();
    QList<double> scopedValues() const;
    QList<DatasetObservation> scopedObservations() const;
    QString scopedDatasetLabel() const;
    QString scopedDatasetDescription() const;
    QStringList scopedPreviewLines() const;
    void refreshStudySummaryView();
    void refreshDescriptiveView(const DataSummary& summary);
    void refreshDistributionFitView(const DataSummary& summary);
    void refreshInferenceView(const DataSummary& summary);
    void refreshReportView(const DataSummary& summary, const QString& fitConclusion);
    void refreshDoeDemoViews();
    void refreshDoePlanSummary();
    void exportDataset();
    void saveReport();
    void showSkeletonMessage(const QString& featureName);

    static DataSummary summarizeValues(const QList<double>& values);
    static QList<double> sortedValues(const QList<double>& values);
    static double percentile(const QList<double>& sorted, double probability);
    static QString exactMode(const QList<double>& values);
    static QString formatNumber(double value);

private:
    Simulator* _simulator = nullptr;
    QString& _lastDataAnalyzerPath;
    QString _studyName;
    QList<DatasetDescriptor> _datasets;
    QString _lastFitConclusion;

    QAction* _openDatasetAction = nullptr;
    QAction* _saveReportAction = nullptr;
    QAction* _importSimulationResponsesAction = nullptr;
    QAction* _refreshModelResponsesAction = nullptr;
    QAction* _exportDatasetAction = nullptr;

    QListWidget* _datasetsList = nullptr;
    QListWidget* _analysisNavigator = nullptr;
    QTableWidget* _modelResponsesTable = nullptr;
    QTabWidget* _workspaceTabs = nullptr;
    QComboBox* _scopeCombo = nullptr;
    QLabel* _datasetSourceLabel = nullptr;
    QPlainTextEdit* _datasetPreview = nullptr;
    QTableWidget* _studySummaryTable = nullptr;
    QTableWidget* _centralTendencyTable = nullptr;
    QTableWidget* _dispersionTable = nullptr;
    QTableWidget* _rawDataTable = nullptr;
    QTableWidget* _movingAverageTable = nullptr;
    HistogramPreview* _histogramPreview = nullptr;
    QTableWidget* _fitTable = nullptr;
    QLabel* _fitConclusionLabel = nullptr;
    QTableWidget* _inferenceTable = nullptr;
    QTableWidget* _doeFactorsTable = nullptr;
    QTableWidget* _doeResponsesTable = nullptr;
    QTableWidget* _doeDesignTable = nullptr;
    QTableWidget* _doeAnovaTable = nullptr;
    QTableWidget* _doeOptimizationTable = nullptr;
    QTableWidget* _doeAdviceTable = nullptr;
    QTableWidget* _doeDesignQualityTable = nullptr;
    QTableWidget* _doeDesirabilityTable = nullptr;
    QComboBox* _doeDesignFamilyCombo = nullptr;
    QComboBox* _doeModelOrderCombo = nullptr;
    QComboBox* _doePlotSelector = nullptr;
    QSpinBox* _doeReplicationsSpin = nullptr;
    QDoubleSpinBox* _doeAlphaSpin = nullptr;
    QCheckBox* _doeRandomizeCheck = nullptr;
    QLabel* _doeAdvisorLabel = nullptr;
    QLabel* _doePlanSummaryLabel = nullptr;
    DoePreviewPlot* _doePreviewPlot = nullptr;
    QTextBrowser* _startPage = nullptr;
    QTextEdit* _reportText = nullptr;
};

#endif // DATAANALYZERWINDOW_H
