#pragma once

#include <QDialog>

class QCloseEvent;
class QShowEvent;
class QPlainTextEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class WebWorkerRuntime;

/**
 * @brief Lightweight control window for the embedded GenESyS web worker.
 *
 * The dialog lets the GUI user inspect the worker lifecycle, change the listen
 * port, and start/stop/restart the HTTP service that runs the web API with its
 * own simulator instance.
 */
class WebWorkerDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Builds the worker control window for a shared runtime instance.
     */
    explicit WebWorkerDialog(WebWorkerRuntime* runtime, QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    /**
     * @brief Refreshes the widgets with the current worker snapshot.
     */
    void _refresh();
    /**
     * @brief Applies the configured controls and starts the worker.
     */
    void _startWorker();
    /**
     * @brief Stops the worker and refreshes the dialog.
     */
    void _stopWorker();
    /**
     * @brief Restarts the worker with the current configuration values.
     */
    void _restartWorker();
    /**
     * @brief Refreshes the worker on dialog open when the auto-restart toggle is enabled.
     */
    void _maybeAutoRestartOnShow();

    WebWorkerRuntime* _runtime = nullptr;
    QSpinBox* _portSpin = nullptr;
    QSpinBox* _maxRequestsSpin = nullptr;
    QLabel* _statusValue = nullptr;
    QLabel* _phaseValue = nullptr;
    QLabel* _urlValue = nullptr;
    QLabel* _servedValue = nullptr;
    QLabel* _activeValue = nullptr;
    QLabel* _stateValue = nullptr;
    QPlainTextEdit* _historyText = nullptr;
    QPlainTextEdit* _errorText = nullptr;
    QPushButton* _startButton = nullptr;
    QPushButton* _stopButton = nullptr;
    QPushButton* _restartButton = nullptr;
    QPushButton* _autoRestartButton = nullptr;
};
