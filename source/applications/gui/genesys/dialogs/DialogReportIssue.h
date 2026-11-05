#ifndef DIALOGREPORTISSUE_H
#define DIALOGREPORTISSUE_H

#include <QDialog>
#include "../services/IssueReportRelayService.h"

class QCloseEvent;
class QComboBox;
class QLabel;
class QLineEdit;
class QNetworkAccessManager;
class QPlainTextEdit;
class QPushButton;

/** Compact, privacy-first issue form. Transport stays asynchronous. */
class DialogReportIssue : public QDialog {
public:
    explicit DialogReportIssue(QWidget* parent = nullptr);

    IssueReportDraft draft() const;
    void setRelayAvailable(bool configured);

protected:
    void reject() override;
    void closeEvent(QCloseEvent* event) override;

private:
    void updateSubmitButton();
    void submit();
    void continueOnGitHub();
    void handleRelayResult(const IssueReportSubmissionResult& result);

    QComboBox* _categoryCombo = nullptr;
    QLineEdit* _titleEdit = nullptr;
    QPlainTextEdit* _descriptionEdit = nullptr;
    QLabel* _hintLabel = nullptr;
    QLabel* _validationLabel = nullptr;
    QPushButton* _submitButton = nullptr;
    QNetworkAccessManager* _networkManager = nullptr;
    bool _relayAvailable = false;
    bool _submitting = false;
};

#endif // DIALOGREPORTISSUE_H
