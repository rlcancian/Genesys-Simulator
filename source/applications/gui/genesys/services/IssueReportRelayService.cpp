#include "IssueReportRelayService.h"

#include <QDateTime>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace {
static void insertIfNotEmpty(QJsonObject* object, const QString& key, const QString& value) {
    if (object == nullptr) {
        return;
    }
    const QString trimmed = value.trimmed();
    if (!trimmed.isEmpty()) {
        object->insert(key, trimmed);
    }
}

static QJsonObject buildPayload(const IssueReportDraft& draft,
                                const IssueReportRuntimeContext& runtimeContext) {
    QJsonObject reportObject;
    reportObject.insert(QStringLiteral("repository"), QStringLiteral("rlcancian/Genesys-Simulator"));
    reportObject.insert(QStringLiteral("category_key"), draft.categoryKey);
    reportObject.insert(QStringLiteral("category_label"), draft.categoryLabel);
    reportObject.insert(QStringLiteral("title"), draft.title);
    reportObject.insert(QStringLiteral("summary"), draft.summary);
    insertIfNotEmpty(&reportObject, QStringLiteral("reproduction_steps"), draft.reproductionSteps);
    insertIfNotEmpty(&reportObject, QStringLiteral("expected_behavior"), draft.expectedBehavior);
    insertIfNotEmpty(&reportObject, QStringLiteral("observed_behavior"), draft.observedBehavior);
    insertIfNotEmpty(&reportObject, QStringLiteral("additional_context"), draft.additionalContext);
    insertIfNotEmpty(&reportObject, QStringLiteral("contact_email"), draft.contactEmail);

    QJsonObject clientObject;
    insertIfNotEmpty(&clientObject, QStringLiteral("application_name"), runtimeContext.applicationName);
    insertIfNotEmpty(&clientObject, QStringLiteral("application_version"), runtimeContext.applicationVersion);
    insertIfNotEmpty(&clientObject, QStringLiteral("qt_version"), runtimeContext.qtVersion);
    insertIfNotEmpty(&clientObject, QStringLiteral("operating_system"), runtimeContext.operatingSystem);
    insertIfNotEmpty(&clientObject, QStringLiteral("current_model_name"), runtimeContext.currentModelName);
    clientObject.insert(QStringLiteral("ui_surface"), QStringLiteral("GenesysQtGUI"));
    clientObject.insert(QStringLiteral("entrypoint"), QStringLiteral("About/Report Issue"));
    clientObject.insert(QStringLiteral("submitted_at_utc"),
                        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    QJsonObject diagnosticsObject;
    insertIfNotEmpty(&diagnosticsObject, QStringLiteral("current_model_simulang"),
                     runtimeContext.currentModelSimulanguage);
    insertIfNotEmpty(&diagnosticsObject, QStringLiteral("console_tail"), runtimeContext.consoleTail);
    insertIfNotEmpty(&diagnosticsObject, QStringLiteral("simulation_tail"), runtimeContext.simulationTail);
    insertIfNotEmpty(&diagnosticsObject, QStringLiteral("reports_tail"), runtimeContext.reportsTail);
    insertIfNotEmpty(&diagnosticsObject, QStringLiteral("process_log_tail"), runtimeContext.processLogTail);
    if (!runtimeContext.screenshotPngBase64.isEmpty()) {
        diagnosticsObject.insert(QStringLiteral("screenshot_png_base64"),
                                 QString::fromLatin1(runtimeContext.screenshotPngBase64));
    }

    QJsonObject rootObject;
    rootObject.insert(QStringLiteral("schema_version"), 1);
    rootObject.insert(QStringLiteral("report"), reportObject);
    rootObject.insert(QStringLiteral("client"), clientObject);
    rootObject.insert(QStringLiteral("diagnostics"), diagnosticsObject);
    return rootObject;
}

static QString extractRelayMessage(const QJsonObject& responseObject, const QString& rawBody) {
    const QString message = responseObject.value(QStringLiteral("message")).toString().trimmed();
    if (!message.isEmpty()) {
        return message;
    }
    const QString error = responseObject.value(QStringLiteral("error")).toString().trimmed();
    if (!error.isEmpty()) {
        return error;
    }
    const QString trimmedBody = rawBody.trimmed();
    if (trimmedBody.isEmpty()) {
        return QStringLiteral("No additional details were returned by the relay.");
    }
    return trimmedBody.left(1000);
}
} // namespace

QString IssueReportRelayService::configuredEndpoint() {
    return qEnvironmentVariable("GENESYS_REPORT_ISSUE_RELAY_URL").trimmed();
}

QString IssueReportRelayService::optionalClientKey() {
    return qEnvironmentVariable("GENESYS_REPORT_ISSUE_CLIENT_KEY").trimmed();
}

int IssueReportRelayService::requestTimeoutMs() {
    bool ok = false;
    const int value = qEnvironmentVariableIntValue("GENESYS_REPORT_ISSUE_TIMEOUT_MS", &ok);
    if (ok && value > 0) {
        return value;
    }
    return 15000;
}

bool IssueReportRelayService::isEndpointConfigured(QString* reason) {
    const QString endpoint = configuredEndpoint();
    if (endpoint.isEmpty()) {
        if (reason != nullptr) {
            *reason = QStringLiteral(
                "Set GENESYS_REPORT_ISSUE_RELAY_URL to an HTTPS relay endpoint before using Send Issue.");
        }
        return false;
    }

    const QUrl url(endpoint);
    if (!url.isValid() || url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0) {
        if (reason != nullptr) {
            *reason = QStringLiteral(
                "GENESYS_REPORT_ISSUE_RELAY_URL must be a valid HTTPS URL.");
        }
        return false;
    }

    if (reason != nullptr) {
        *reason = QString();
    }
    return true;
}

IssueReportSubmissionResult IssueReportRelayService::submit(const IssueReportDraft& draft,
                                                            const IssueReportRuntimeContext& runtimeContext) {
    IssueReportSubmissionResult result;

    QString endpointReason;
    if (!isEndpointConfigured(&endpointReason)) {
        result.errorMessage = endpointReason;
        return result;
    }

    const QUrl endpoint(configuredEndpoint());
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Genesys-Simulator/ReportIssueRelay"));

    const QString clientKey = optionalClientKey();
    if (!clientKey.isEmpty()) {
        request.setRawHeader("X-Genesys-Client-Key", clientKey.toUtf8());
    }

    const QByteArray payload =
            QJsonDocument(buildPayload(draft, runtimeContext)).toJson(QJsonDocument::Compact);

    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.post(request, payload);

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    bool timedOut = false;

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        timedOut = true;
        if (reply != nullptr && reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });

    timeoutTimer.start(requestTimeoutMs());
    loop.exec();
    timeoutTimer.stop();

    const QByteArray rawResponse = reply->readAll();
    result.responseBody = QString::fromUtf8(rawResponse);
    result.httpStatus =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QJsonParseError parseError{};
    const QJsonDocument responseDocument = QJsonDocument::fromJson(rawResponse, &parseError);
    const QJsonObject responseObject =
            (parseError.error == QJsonParseError::NoError && responseDocument.isObject())
            ? responseDocument.object()
            : QJsonObject();

    if (responseObject.contains(QStringLiteral("issue"))
        && responseObject.value(QStringLiteral("issue")).isObject()) {
        const QJsonObject issueObject = responseObject.value(QStringLiteral("issue")).toObject();
        result.issueNumber = issueObject.value(QStringLiteral("number")).toInt();
        result.issueUrl = issueObject.value(QStringLiteral("html_url")).toString().trimmed();
    }
    if (result.issueUrl.isEmpty()) {
        result.issueUrl = responseObject.value(QStringLiteral("issue_url")).toString().trimmed();
    }
    if (result.issueUrl.isEmpty()) {
        result.issueUrl = responseObject.value(QStringLiteral("html_url")).toString().trimmed();
    }
    if (result.issueNumber == 0) {
        result.issueNumber = responseObject.value(QStringLiteral("issue_number")).toInt();
    }

    if (timedOut) {
        result.errorMessage = QStringLiteral(
            "The report relay did not answer within the configured timeout.");
        reply->deleteLater();
        return result;
    }

    if (reply->error() != QNetworkReply::NoError && result.httpStatus == 0) {
        result.errorMessage = QStringLiteral("Relay request failed: %1")
                                  .arg(reply->errorString());
        reply->deleteLater();
        return result;
    }

    if (result.httpStatus < 200 || result.httpStatus >= 300) {
        result.errorMessage = QStringLiteral("Relay rejected the report (HTTP %1). %2")
                                  .arg(result.httpStatus)
                                  .arg(extractRelayMessage(responseObject, result.responseBody));
        reply->deleteLater();
        return result;
    }

    result.success = true;
    reply->deleteLater();
    return result;
}
