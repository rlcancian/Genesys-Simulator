#include "WebWorkerDialog.h"

#include "applications/worker/service/WebWorkerRuntime.h"

#include <QCloseEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QShowEvent>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

namespace {
QString _stateText(const WebWorkerRuntime::Snapshot& snapshot) {
    if (snapshot.isRunning) {
        return QObject::tr("Running");
    }
    if (snapshot.isStarting) {
        return QObject::tr("Starting");
    }
    if (snapshot.isStopping) {
        return QObject::tr("Stopping");
    }
    if (snapshot.hasFailed) {
        return QObject::tr("Failed");
    }
    return QObject::tr("Stopped");
}

QString _phaseText(const WebWorkerRuntime::Snapshot& snapshot) {
    if (snapshot.isStarting) {
        return QObject::tr("Binding and starting the HTTP listener");
    }
    if (snapshot.isRunning) {
        return QObject::tr("Serving requests");
    }
    if (snapshot.isStopping) {
        return QObject::tr("Stopping the HTTP listener");
    }
    if (snapshot.hasFailed) {
        return QObject::tr("Failed. Review the history below.");
    }
    return QObject::tr("Idle");
}
}  // namespace

WebWorkerDialog::WebWorkerDialog(QWidget* parent, bool allowClose)
    : QDialog(parent), _allowClose(allowClose), _runtime(std::make_unique<WebWorkerRuntime>()) {
    setWindowTitle(tr("Web Worker"));
    setMinimumWidth(620);
    setAttribute(Qt::WA_DeleteOnClose, _allowClose);

    auto* rootLayout = new QVBoxLayout(this);

    auto* configGroup = new QGroupBox(tr("Configuration"), this);
    auto* configLayout = new QFormLayout(configGroup);

    _portSpin = new QSpinBox(configGroup);
    _portSpin->setRange(1, 65535);
    _portSpin->setValue(_runtime != nullptr ? static_cast<int>(_runtime->configuredPort()) : 8080);
    configLayout->addRow(tr("Port"), _portSpin);

    _maxRequestsSpin = new QSpinBox(configGroup);
    _maxRequestsSpin->setRange(0, 1'000'000);
    _maxRequestsSpin->setSpecialValueText(tr("Unlimited"));
    _maxRequestsSpin->setValue(_runtime != nullptr ? static_cast<int>(_runtime->configuredMaxRequests()) : 0);
    configLayout->addRow(tr("Max requests"), _maxRequestsSpin);

    rootLayout->addWidget(configGroup);

    auto* statusGroup = new QGroupBox(tr("Status"), this);
    auto* statusLayout = new QFormLayout(statusGroup);
    _stateValue = new QLabel(statusGroup);
    _statusValue = new QLabel(statusGroup);
    _phaseValue = new QLabel(statusGroup);
    _urlValue = new QLabel(statusGroup);
    _servedValue = new QLabel(statusGroup);
    _activeValue = new QLabel(statusGroup);
    statusLayout->addRow(tr("State"), _stateValue);
    statusLayout->addRow(tr("Status"), _statusValue);
    statusLayout->addRow(tr("Phase"), _phaseValue);
    statusLayout->addRow(tr("URL"), _urlValue);
    statusLayout->addRow(tr("Requests served"), _servedValue);
    statusLayout->addRow(tr("Active requests"), _activeValue);
    rootLayout->addWidget(statusGroup);

    auto* historyGroup = new QGroupBox(tr("History"), this);
    auto* historyLayout = new QVBoxLayout(historyGroup);
    _historyText = new QPlainTextEdit(historyGroup);
    _historyText->setReadOnly(true);
    _historyText->setMinimumHeight(150);
    historyLayout->addWidget(_historyText);
    rootLayout->addWidget(historyGroup, 1);

    auto* errorGroup = new QGroupBox(tr("Last Error"), this);
    auto* errorLayout = new QVBoxLayout(errorGroup);
    _errorText = new QPlainTextEdit(errorGroup);
    _errorText->setReadOnly(true);
    _errorText->setMinimumHeight(120);
    errorLayout->addWidget(_errorText);
    rootLayout->addWidget(errorGroup, 1);

    connect(_portSpin, &QSpinBox::valueChanged, this, [this]() {
        if (_runtime != nullptr) {
            _runtime->setConfiguredPort(static_cast<unsigned short>(_portSpin->value()));
        }
    });
    connect(_maxRequestsSpin, &QSpinBox::valueChanged, this, [this]() {
        if (_runtime != nullptr) {
            _runtime->setConfiguredMaxRequests(static_cast<unsigned long>(_maxRequestsSpin->value()));
        }
    });

    auto* timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [this]() { _refresh(); });
    timer->start();

    _refresh();
}

WebWorkerDialog::~WebWorkerDialog() = default;

void WebWorkerDialog::closeEvent(QCloseEvent* event) {
    if (_runtime != nullptr) {
        _runtime->stop();
    }
    if (_allowClose) {
        event->accept();
        return;
    }
    hide();
    event->ignore();
}

void WebWorkerDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    if (_runtime != nullptr) {
        _runtime->start();
    }
}

void WebWorkerDialog::_refresh() {
    if (_runtime == nullptr) {
        _stateValue->setText(tr("Unavailable"));
        _statusValue->setText(tr("No runtime bound"));
        _phaseValue->setText(tr("No runtime bound"));
        _urlValue->setText(tr("n/a"));
        _servedValue->setText(tr("0"));
        _activeValue->setText(tr("0"));
        _historyText->setPlainText(tr("Web worker runtime not available."));
        _errorText->setPlainText(tr("Web worker runtime not available."));
        return;
    }

    const WebWorkerRuntime::Snapshot snapshot = _runtime->snapshot();
    _portSpin->blockSignals(true);
    _portSpin->setValue(snapshot.port);
    _portSpin->blockSignals(false);
    _maxRequestsSpin->blockSignals(true);
    _maxRequestsSpin->setValue(static_cast<int>(snapshot.maxRequests));
    _maxRequestsSpin->blockSignals(false);

    _stateValue->setText(_stateText(snapshot));
    _statusValue->setText(QString::fromStdString(snapshot.statusMessage));
    _phaseValue->setText(_phaseText(snapshot));
    _urlValue->setText(QStringLiteral("http://127.0.0.1:%1").arg(snapshot.port));
    _servedValue->setText(QString::number(snapshot.servedRequests));
    _activeValue->setText(QString::number(snapshot.activeRequests));
    _errorText->setPlainText(QString::fromStdString(snapshot.lastError));

    QStringList historyLines;
    historyLines << tr("Recent events:");
    if (snapshot.recentEvents.empty()) {
        historyLines << tr("  - none");
    } else {
        for (const std::string& event : snapshot.recentEvents) {
            historyLines << QStringLiteral("  - ") + QString::fromStdString(event);
        }
    }
    historyLines << QString();
    historyLines << tr("Recent errors:");
    if (snapshot.recentErrors.empty()) {
        historyLines << tr("  - none");
    } else {
        for (const std::string& error : snapshot.recentErrors) {
            historyLines << QStringLiteral("  - ") + QString::fromStdString(error);
        }
    }
    _historyText->setPlainText(historyLines.join('\n'));
}
