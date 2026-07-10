#ifndef ISSUEREPORTRELAYSERVICE_H
#define ISSUEREPORTRELAYSERVICE_H

#include <QByteArray>
#include <QString>

/**
 * @brief User-authored issue report collected by the GUI form before transport.
 */
struct IssueReportDraft {
    QString categoryKey;
    QString categoryLabel;
    QString title;
    QString summary;
    QString reproductionSteps;
    QString expectedBehavior;
    QString observedBehavior;
    QString additionalContext;
    QString contactEmail;
    bool includeScreenshot = false;
    bool includeLogTail = true;
    bool includeModelText = false;
};

/**
 * @brief Runtime diagnostics captured by the GUI after the user approves submission.
 */
struct IssueReportRuntimeContext {
    QString applicationName;
    QString applicationVersion;
    QString qtVersion;
    QString operatingSystem;
    QString currentModelName;
    QString currentModelSimulanguage;
    QString consoleTail;
    QString simulationTail;
    QString reportsTail;
    QString processLogTail;
    QByteArray screenshotPngBase64;
};

/**
 * @brief Relay submission outcome returned to the controller layer.
 */
struct IssueReportSubmissionResult {
    bool success = false;
    int httpStatus = 0;
    int issueNumber = 0;
    QString issueUrl;
    QString errorMessage;
    QString responseBody;
};

/**
 * @brief HTTPS client service that sends issue reports to a secure relay endpoint.
 *
 * Responsibilities:
 * - validate relay configuration from environment;
 * - serialize the GUI report and runtime diagnostics into JSON;
 * - submit the payload via HTTPS and normalize the relay response.
 *
 * Boundaries:
 * - it does not talk directly to GitHub with a secret embedded in the client;
 * - it does not own UI widgets or present dialogs;
 * - it assumes an external relay exists and enforces server-side policy.
 */
class IssueReportRelayService {
public:
    static QString configuredEndpoint();
    static QString optionalClientKey();
    static int requestTimeoutMs();
    static bool isEndpointConfigured(QString* reason = nullptr);
    static IssueReportSubmissionResult submit(const IssueReportDraft& draft,
                                              const IssueReportRuntimeContext& runtimeContext);
};

#endif // ISSUEREPORTRELAYSERVICE_H
