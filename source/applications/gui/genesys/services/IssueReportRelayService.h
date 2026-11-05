#ifndef ISSUEREPORTRELAYSERVICE_H
#define ISSUEREPORTRELAYSERVICE_H

#include <QByteArray>
#include <QNetworkReply>
#include <QString>
#include <functional>

class QObject;
class QNetworkAccessManager;

/** Schema v1 report. Optional legacy fields remain for relay compatibility. */
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
    bool includeLogTail = false;
    bool includeModelText = false;
};

/** Only non-sensitive fields are populated by the simplified GUI. */
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

struct IssueReportSubmissionResult {
    bool success = false;
    int httpStatus = 0;
    int issueNumber = 0;
    QString issueUrl;
    QString errorMessage;
    QString responseBody;
};

/**
 * HTTPS relay client. A 2xx alone is not proof of GitHub issue creation:
 * success requires a valid issue number and canonical GitHub issue URL.
 * Requests are asynchronous and do not run a nested GUI event loop.
 */
class IssueReportRelayService {
public:
    using Completion = std::function<void(const IssueReportSubmissionResult&)>;

    static QString configuredEndpoint();
    static QString optionalClientKey();
    static int requestTimeoutMs();
    static bool isEndpointConfigured(QString* reason = nullptr);

    /** The caller owns manager and receiver; callback is suppressed if receiver dies. */
    static void submitAsync(const IssueReportDraft& draft,
                            const IssueReportRuntimeContext& runtimeContext,
                            QNetworkAccessManager* manager,
                            QObject* receiver,
                            Completion completion);

    /** Pure response verification, also used by network-mock tests. */
    static IssueReportSubmissionResult parseResponse(int httpStatus,
                                                      const QByteArray& response,
                                                      QNetworkReply::NetworkError networkError =
                                                          QNetworkReply::NoError,
                                                      bool timedOut = false);
    static bool isCanonicalIssueUrl(const QString& url, int issueNumber);
    static QByteArray serializePayload(const IssueReportDraft& draft,
                                       const IssueReportRuntimeContext& runtimeContext);
};

#endif // ISSUEREPORTRELAYSERVICE_H
