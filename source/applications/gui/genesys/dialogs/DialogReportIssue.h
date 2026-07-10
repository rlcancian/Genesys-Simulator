#ifndef DIALOGREPORTISSUE_H
#define DIALOGREPORTISSUE_H

#include <QDialog>

#include "../services/IssueReportRelayService.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

/**
 * @brief Modal form used to collect a user-facing issue report before relay submission.
 *
 * Responsibilities:
 * - collect category, title, description, and optional diagnostic consent flags;
 * - expose a validated IssueReportDraft for the controller layer;
 * - reflect whether the report relay is configured and available.
 *
 * Boundaries:
 * - it does not perform HTTP transport;
 * - it does not talk to GitHub directly;
 * - it does not inspect application runtime state on its own.
 */
class DialogReportIssue : public QDialog {
public:
    explicit DialogReportIssue(QWidget* parent = nullptr);

    /** @brief Returns a normalized draft from the current form fields. */
    IssueReportDraft draft() const;
    /** @brief Updates relay availability text and enables/disables the send action accordingly. */
    void setRelayConfigurationStatus(const QString& endpoint, bool configured, const QString& detail);

private:
    /** @brief Recomputes whether the Send Issue button should be enabled. */
    void _updateSendButton();

    QComboBox* _categoryCombo = nullptr;
    QLineEdit* _titleEdit = nullptr;
    QPlainTextEdit* _summaryEdit = nullptr;
    QPlainTextEdit* _stepsEdit = nullptr;
    QPlainTextEdit* _expectedEdit = nullptr;
    QPlainTextEdit* _observedEdit = nullptr;
    QPlainTextEdit* _additionalContextEdit = nullptr;
    QLineEdit* _contactEdit = nullptr;
    QCheckBox* _includeScreenshotCheck = nullptr;
    QCheckBox* _includeLogTailCheck = nullptr;
    QCheckBox* _includeModelTextCheck = nullptr;
    QLabel* _relayStatusLabel = nullptr;
    QPushButton* _sendButton = nullptr;
    bool _relayConfigured = true;
};

#endif // DIALOGREPORTISSUE_H
