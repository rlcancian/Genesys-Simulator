#include "DialogReportIssue.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

DialogReportIssue::DialogReportIssue(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Report Issue"));
    setModal(true);
    resize(820, 700);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(14, 14, 14, 14);
    rootLayout->setSpacing(10);

    QLabel* introLabel = new QLabel(
        tr("Submit a bug report, error/inconformity, question, or suggestion through the configured secure relay. "
           "No GitHub credential is stored in this desktop client."), this);
    introLabel->setWordWrap(true);
    rootLayout->addWidget(introLabel);

    _relayStatusLabel = new QLabel(this);
    _relayStatusLabel->setWordWrap(true);
    _relayStatusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    rootLayout->addWidget(_relayStatusLabel);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    formLayout->setFormAlignment(Qt::AlignTop);

    _categoryCombo = new QComboBox(this);
    _categoryCombo->addItem(tr("Bug"), QStringLiteral("bug"));
    _categoryCombo->addItem(tr("Error / Inconformity"), QStringLiteral("error"));
    _categoryCombo->addItem(tr("Suggestion / Enhancement"), QStringLiteral("enhancement"));
    _categoryCombo->addItem(tr("Question"), QStringLiteral("question"));
    formLayout->addRow(tr("Category:"), _categoryCombo);

    _titleEdit = new QLineEdit(this);
    _titleEdit->setPlaceholderText(tr("Short title for the issue"));
    formLayout->addRow(tr("Title:"), _titleEdit);

    _summaryEdit = new QPlainTextEdit(this);
    _summaryEdit->setPlaceholderText(tr("Describe the problem or suggestion."));
    _summaryEdit->setMinimumHeight(90);
    formLayout->addRow(tr("Summary:"), _summaryEdit);

    _stepsEdit = new QPlainTextEdit(this);
    _stepsEdit->setPlaceholderText(tr("Step-by-step reproduction, if applicable."));
    _stepsEdit->setMinimumHeight(80);
    formLayout->addRow(tr("Reproduction steps:"), _stepsEdit);

    _expectedEdit = new QPlainTextEdit(this);
    _expectedEdit->setPlaceholderText(tr("What did you expect to happen?"));
    _expectedEdit->setMinimumHeight(70);
    formLayout->addRow(tr("Expected behavior:"), _expectedEdit);

    _observedEdit = new QPlainTextEdit(this);
    _observedEdit->setPlaceholderText(tr("What actually happened?"));
    _observedEdit->setMinimumHeight(70);
    formLayout->addRow(tr("Observed behavior:"), _observedEdit);

    _additionalContextEdit = new QPlainTextEdit(this);
    _additionalContextEdit->setPlaceholderText(tr("Any extra information that may help the maintainers."));
    _additionalContextEdit->setMinimumHeight(70);
    formLayout->addRow(tr("Additional context:"), _additionalContextEdit);

    _contactEdit = new QLineEdit(this);
    _contactEdit->setPlaceholderText(tr("Optional e-mail or contact handle"));
    formLayout->addRow(tr("Contact:"), _contactEdit);

    rootLayout->addLayout(formLayout);

    QLabel* consentLabel = new QLabel(
        tr("Optional diagnostics to include in the payload sent to the relay:"), this);
    consentLabel->setWordWrap(true);
    rootLayout->addWidget(consentLabel);

    _includeScreenshotCheck = new QCheckBox(tr("Attach screenshot of the main window"), this);
    _includeScreenshotCheck->setChecked(false);
    rootLayout->addWidget(_includeScreenshotCheck);

    _includeLogTailCheck = new QCheckBox(tr("Attach tail of crashesAndLogs.log"), this);
    _includeLogTailCheck->setChecked(true);
    rootLayout->addWidget(_includeLogTailCheck);

    _includeModelTextCheck = new QCheckBox(tr("Attach current SimulLang editor contents"), this);
    _includeModelTextCheck->setChecked(false);
    rootLayout->addWidget(_includeModelTextCheck);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    _sendButton = buttonBox->addButton(tr("Send Issue"), QDialogButtonBox::AcceptRole);
    _sendButton->setDefault(true);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    rootLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(_categoryCombo, &QComboBox::currentIndexChanged, this, [this](int) { _updateSendButton(); });
    connect(_titleEdit, &QLineEdit::textChanged, this, [this](const QString&) { _updateSendButton(); });
    connect(_summaryEdit, &QPlainTextEdit::textChanged, this, [this]() { _updateSendButton(); });

    _updateSendButton();
}

IssueReportDraft DialogReportIssue::draft() const {
    IssueReportDraft value;
    value.categoryKey = _categoryCombo->currentData().toString().trimmed();
    value.categoryLabel = _categoryCombo->currentText().trimmed();
    value.title = _titleEdit->text().trimmed();
    value.summary = _summaryEdit->toPlainText().trimmed();
    value.reproductionSteps = _stepsEdit->toPlainText().trimmed();
    value.expectedBehavior = _expectedEdit->toPlainText().trimmed();
    value.observedBehavior = _observedEdit->toPlainText().trimmed();
    value.additionalContext = _additionalContextEdit->toPlainText().trimmed();
    value.contactEmail = _contactEdit->text().trimmed();
    value.includeScreenshot = _includeScreenshotCheck->isChecked();
    value.includeLogTail = _includeLogTailCheck->isChecked();
    value.includeModelText = _includeModelTextCheck->isChecked();
    return value;
}

void DialogReportIssue::setRelayConfigurationStatus(const QString& endpoint,
                                                    bool configured,
                                                    const QString& detail) {
    _relayConfigured = configured;
    if (configured) {
        _relayStatusLabel->setText(
            tr("Relay endpoint configured: %1")
                .arg(endpoint.isEmpty() ? tr("(not disclosed)") : endpoint));
    } else {
        _relayStatusLabel->setText(
            tr("Relay endpoint is not configured.\n%1")
                .arg(detail.trimmed()));
    }
    _updateSendButton();
}

void DialogReportIssue::_updateSendButton() {
    if (_sendButton == nullptr) {
        return;
    }

    const bool hasCategory = !_categoryCombo->currentData().toString().trimmed().isEmpty();
    const bool hasTitle = !_titleEdit->text().trimmed().isEmpty();
    const bool hasSummary = !_summaryEdit->toPlainText().trimmed().isEmpty();
    _sendButton->setEnabled(_relayConfigured && hasCategory && hasTitle && hasSummary);
}
