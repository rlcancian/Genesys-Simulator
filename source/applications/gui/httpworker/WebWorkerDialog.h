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
 */
class WebWorkerDialog : public QDialog {
    Q_OBJECT

public:
    explicit WebWorkerDialog(QWidget* parent = nullptr, bool allowClose = false);
    ~WebWorkerDialog() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void _refresh();

    bool _allowClose = false;
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
