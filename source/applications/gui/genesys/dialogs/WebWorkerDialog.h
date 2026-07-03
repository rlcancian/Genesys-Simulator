#pragma once

#include <QDialog>
#include <memory>

class QCloseEvent;
class QShowEvent;
class QPlainTextEdit;
class QSpinBox;
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
     * @brief Builds the worker control window and owns its runtime instance.
     */
    explicit WebWorkerDialog(QWidget* parent = nullptr);
    ~WebWorkerDialog() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    /**
     * @brief Refreshes the widgets with the current worker snapshot.
     */
    void _refresh();
    std::unique_ptr<WebWorkerRuntime> _runtime;
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
};
