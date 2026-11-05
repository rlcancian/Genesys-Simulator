#include "DialogReportIssue.h"

#include "../services/IssueReportBrowserService.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <utility>

DialogReportIssue::DialogReportIssue(QWidget* parent,
                                     QNetworkAccessManager* networkManager,
                                     std::function<bool(const QUrl&)> browserOpener)
    : QDialog(parent),
      _networkManager(networkManager != nullptr ? networkManager : new QNetworkAccessManager(this)),
      _browserOpener(browserOpener ? std::move(browserOpener)
                                   : [](const QUrl& url) { return QDesktopServices::openUrl(url); }) {
    setWindowTitle(tr("Report Issue"));
    setModal(true);
    resize(540, 335);
    setMinimumWidth(440);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(9);

    _hintLabel = new QLabel(this);
    _hintLabel->setWordWrap(true);
    root->addWidget(_hintLabel);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);

    _categoryCombo = new QComboBox(this);
    _categoryCombo->setObjectName(QStringLiteral("issueType"));
    _categoryCombo->addItem(tr("Bug"), QStringLiteral("bug"));
    _categoryCombo->addItem(tr("Suggestion / Enhancement"), QStringLiteral("enhancement"));
    _categoryCombo->addItem(tr("Question"), QStringLiteral("question"));
    form->addRow(tr("Type:"), _categoryCombo);

    _titleEdit = new QLineEdit(this);
    _titleEdit->setObjectName(QStringLiteral("issueTitle"));
    _titleEdit->setMaxLength(256);
    _titleEdit->setPlaceholderText(tr("Short summary of the issue"));
    form->addRow(tr("Title:"), _titleEdit);

    _descriptionEdit = new QPlainTextEdit(this);
    _descriptionEdit->setObjectName(QStringLiteral("issueDescription"));
    _descriptionEdit->setPlaceholderText(tr("Describe the problem, question, or suggestion."));
    _descriptionEdit->setMinimumHeight(155);
    form->addRow(tr("Description:"), _descriptionEdit);
    root->addLayout(form, 1);

    _validationLabel = new QLabel(this);
    _validationLabel->setObjectName(QStringLiteral("issueValidation"));
    _validationLabel->setWordWrap(true);
    root->addWidget(_validationLabel);

    auto* buttons = new QDialogButtonBox(this);
    _submitButton = buttons->addButton(tr("Continue on GitHub"), QDialogButtonBox::AcceptRole);
    _submitButton->setObjectName(QStringLiteral("issueSubmit"));
    _submitButton->setDefault(true);
    buttons->addButton(QDialogButtonBox::Cancel);
    root->addWidget(buttons);

    connect(_submitButton, &QPushButton::clicked, this, [this]() { submit(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &DialogReportIssue::reject);
    connect(_titleEdit, &QLineEdit::textChanged, this, [this]() { updateSubmitButton(); });
    connect(_descriptionEdit, &QPlainTextEdit::textChanged,
            this, [this]() { updateSubmitButton(); });

    setRelayAvailable(false);
    updateSubmitButton();
}

IssueReportDraft DialogReportIssue::draft() const {
    IssueReportDraft value;
    value.categoryKey = _categoryCombo->currentData().toString();
    value.categoryLabel = _categoryCombo->currentText();
    value.title = _titleEdit->text().trimmed();
    value.summary = _descriptionEdit->toPlainText().trimmed();
    // No diagnostics, model text, logs, contact data, or screenshots are collected.
    return value;
}

void DialogReportIssue::setRelayAvailable(bool configured) {
    _relayAvailable = configured;
    _hintLabel->setText(configured
        ? tr("Send this report to the GenESyS project. Only the fields shown below "
             "and basic application/Qt versions are transmitted.")
        : tr("Continue on GitHub to review and submit your issue. "
             "The title and description will be included in the browser URL "
             "and may be saved in browser history. No issue is created automatically."));
    _submitButton->setText(configured ? tr("Send Issue") : tr("Continue on GitHub"));
    updateSubmitButton();
}

void DialogReportIssue::updateSubmitButton() {
    const bool hasTitle = !_titleEdit->text().trimmed().isEmpty();
    const bool hasDescription = !_descriptionEdit->toPlainText().trimmed().isEmpty();
    _submitButton->setEnabled(!_submitting && hasTitle && hasDescription);
    _validationLabel->setText(_submitting ? tr("Submitting report…")
                              : (!hasTitle || !hasDescription)
                                  ? tr("Title and description are required.")
                                  : QString());
}

void DialogReportIssue::submit() {
    if (_submitting || draft().title.isEmpty() || draft().summary.isEmpty()) {
        updateSubmitButton();
        return;
    }
    if (!_relayAvailable) {
        continueOnGitHub();
        return;
    }

    _submitting = true;
    updateSubmitButton();

    IssueReportRuntimeContext context;
    context.applicationName = QCoreApplication::applicationName();
    context.applicationVersion = QCoreApplication::applicationVersion();
    context.qtVersion = QString::fromLatin1(qVersion());

    IssueReportRelayService::submitAsync(
        draft(), context, _networkManager, this,
        [this](const IssueReportSubmissionResult& result) {
            _submitting = false;
            updateSubmitButton();
            handleRelayResult(result);
        });
}

void DialogReportIssue::continueOnGitHub() {
    const IssueReportDraft report = draft();
    if (report.title.isEmpty() || report.summary.isEmpty()) {
        updateSubmitButton();
        return;
    }

    const auto link = IssueReportBrowserService::build(report);
    if (!_browserOpener(link.url)) {
        QMessageBox::warning(this, tr("Report Issue"),
                             tr("Could not open the browser. Your report is still in this form."));
        return;
    }

    if (link.prefill != IssueReportBrowserService::Prefill::Complete) {
        QString copied;
        QString instruction;
        if (link.prefill == IssueReportBrowserService::Prefill::BodyRequiresPaste) {
            copied = IssueReportBrowserService::body(report);
            instruction = tr("The description was too long for a browser URL. "
                             "It has been copied to the clipboard. Paste it into GitHub's description field.");
        } else {
            copied = report.title + QStringLiteral("\n\n") + IssueReportBrowserService::body(report);
            instruction = tr("The title and description were too long for a browser URL. "
                             "Both have been copied to the clipboard. Paste them into the GitHub form.");
        }
        QApplication::clipboard()->setText(copied);
        QMessageBox::information(this, tr("Finish on GitHub"), instruction
            + tr("\n\nYour issue has not been submitted yet."));
    } else {
        QMessageBox::information(this, tr("Finish on GitHub"),
            tr("The GitHub issue form was opened with your title and description. "
               "Review it and click Submit new issue on GitHub to finish. "
               "No issue has been created by GenESyS."));
    }
    QDialog::accept();
}

void DialogReportIssue::handleRelayResult(const IssueReportSubmissionResult& result) {
    if (!result.success) {
        QMessageBox box(QMessageBox::Warning, tr("Report Issue"),
                        tr("The issue could not be confirmed.\n\n%1\n\n"
                           "Your text has been preserved. You may retry or continue on GitHub.")
                            .arg(result.errorMessage),
                        QMessageBox::NoButton, this);
        QPushButton* browser = box.addButton(tr("Continue on GitHub"),
                                            QMessageBox::ActionRole);
        box.addButton(QMessageBox::Close);
        box.exec();
        if (box.clickedButton() == browser) {
            continueOnGitHub();
        }
        return;
    }

    QMessageBox box(QMessageBox::Information, tr("Issue created"),
                    tr("GitHub issue #%1 was confirmed by the report service.")
                        .arg(result.issueNumber),
                    QMessageBox::NoButton, this);
    QPushButton* open = box.addButton(tr("Open Issue"), QMessageBox::ActionRole);
    box.addButton(QMessageBox::Close);
    box.exec();
    if (box.clickedButton() == open) {
        // The service validated the canonical GitHub URL and its issue number.
        _browserOpener(QUrl(result.issueUrl));
    }
    QDialog::accept();
}

void DialogReportIssue::reject() {
    if (!_submitting) {
        QDialog::reject();
    }
}

void DialogReportIssue::closeEvent(QCloseEvent* event) {
    if (_submitting) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}
