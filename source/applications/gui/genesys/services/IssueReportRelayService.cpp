#include "IssueReportRelayService.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace {
void insertIfNotEmpty(QJsonObject& object, const QString& key, const QString& value) {
    if (!value.trimmed().isEmpty()) {
        object.insert(key, value.trimmed());
    }
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
    return ok && value > 0 ? value : 15000;
}

bool IssueReportRelayService::isEndpointConfigured(QString* reason) {
    const QString endpoint = configuredEndpoint();
    const QUrl url(endpoint);
    const bool valid = !endpoint.isEmpty() && url.isValid() && !url.host().isEmpty()
                       && url.scheme() == QStringLiteral("https")
                       && url.userInfo().isEmpty() && url.fragment().isEmpty();
    if (reason != nullptr) {
        *reason = valid ? QString()
                        : QStringLiteral("A valid HTTPS report relay is not configured.");
    }
    return valid;
}

QByteArray IssueReportRelayService::serializePayload(
    const IssueReportDraft& draft, const IssueReportRuntimeContext& context) {
    QJsonObject report;
    report.insert(QStringLiteral("repository"), QStringLiteral("rlcancian/Genesys-Simulator"));
    report.insert(QStringLiteral("category_key"), draft.categoryKey);
    report.insert(QStringLiteral("category_label"), draft.categoryLabel);
    report.insert(QStringLiteral("title"), draft.title);
    report.insert(QStringLiteral("summary"), draft.summary);

    // Legacy optional schema-v1 fields remain accepted. The GUI does not collect them.
    insertIfNotEmpty(report, QStringLiteral("reproduction_steps"), draft.reproductionSteps);
    insertIfNotEmpty(report, QStringLiteral("expected_behavior"), draft.expectedBehavior);
    insertIfNotEmpty(report, QStringLiteral("observed_behavior"), draft.observedBehavior);
    insertIfNotEmpty(report, QStringLiteral("additional_context"), draft.additionalContext);
    insertIfNotEmpty(report, QStringLiteral("contact_email"), draft.contactEmail);

    QJsonObject client;
    insertIfNotEmpty(client, QStringLiteral("application_name"), context.applicationName);
    insertIfNotEmpty(client, QStringLiteral("application_version"), context.applicationVersion);
    insertIfNotEmpty(client, QStringLiteral("qt_version"), context.qtVersion);
    client.insert(QStringLiteral("ui_surface"), QStringLiteral("GenesysQtGUI"));
    client.insert(QStringLiteral("entrypoint"), QStringLiteral("About/Report Issue"));
    client.insert(QStringLiteral("submitted_at_utc"),
                  QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    QJsonObject payload;
    payload.insert(QStringLiteral("schema_version"), 1);
    payload.insert(QStringLiteral("report"), report);
    payload.insert(QStringLiteral("client"), client);
    payload.insert(QStringLiteral("diagnostics"), QJsonObject()); // schema-v1 compatibility
    return QJsonDocument(payload).toJson(QJsonDocument::Compact);
}

bool IssueReportRelayService::isCanonicalIssueUrl(const QString& rawUrl, int number) {
    if (number <= 0) {
        return false;
    }
    const QUrl url(rawUrl);
    return url.isValid()
           && url.scheme() == QStringLiteral("https")
           && url.host() == QStringLiteral("github.com")
           && url.userInfo().isEmpty()
           && (url.port(-1) == -1 || url.port(-1) == 443)
           && url.query().isEmpty() && url.fragment().isEmpty()
           && url.path() == QStringLiteral("/rlcancian/Genesys-Simulator/issues/%1")
                                .arg(number);
}

IssueReportSubmissionResult IssueReportRelayService::parseResponse(
    int httpStatus, const QByteArray& response, QNetworkReply::NetworkError error, bool timedOut) {
    IssueReportSubmissionResult result;
    result.httpStatus = httpStatus;
    result.responseBody = QString::fromUtf8(response.left(65536));

    if (timedOut) {
        result.errorMessage = QStringLiteral(
            "The relay timed out. Issue creation is unconfirmed; check GitHub before retrying.");
        return result;
    }
    if (httpStatus == 0 && error != QNetworkReply::NoError) {
        result.errorMessage = QStringLiteral("Could not connect to the report service.");
        return result;
    }
    if (httpStatus == 401 || httpStatus == 403) {
        result.errorMessage = QStringLiteral("The report service rejected authorization.");
        return result;
    }
    if (httpStatus == 429) {
        result.errorMessage = QStringLiteral("The report service is busy. Please retry later.");
        return result;
    }
    if (httpStatus >= 500) {
        result.errorMessage = QStringLiteral("The report service encountered an error.");
        return result;
    }
    if (httpStatus < 200 || httpStatus >= 300) {
        result.errorMessage = QStringLiteral("The report service rejected the report (HTTP %1).")
                                  .arg(httpStatus);
        return result;
    }
    if (error != QNetworkReply::NoError) {
        result.errorMessage = QStringLiteral(
            "The report service connection ended unexpectedly; creation is unconfirmed.");
        return result;
    }

    QJsonParseError parseError{};
    const QJsonDocument parsed = QJsonDocument::fromJson(response, &parseError);
    if (parseError.error != QJsonParseError::NoError || !parsed.isObject()) {
        result.errorMessage = QStringLiteral("The report service returned an invalid response.");
        return result;
    }

    const QJsonObject root = parsed.object();
    const QJsonObject issue = root.value(QStringLiteral("issue")).toObject();
    const QJsonObject record = issue.isEmpty() ? root : issue;
    const int number = record.value(QStringLiteral("number")).toInt(
        record.value(QStringLiteral("issue_number")).toInt());
    QString url = record.value(QStringLiteral("html_url")).toString().trimmed();
    if (url.isEmpty()) {
        url = record.value(QStringLiteral("issue_url")).toString().trimmed();
    }
    if (!isCanonicalIssueUrl(url, number)) {
        result.errorMessage = QStringLiteral(
            "The relay answered, but did not confirm a valid GitHub issue.");
        return result;
    }
    result.issueNumber = number;
    result.issueUrl = url;
    result.success = true;
    return result;
}

void IssueReportRelayService::submitAsync(
    const IssueReportDraft& draft, const IssueReportRuntimeContext& context,
    QNetworkAccessManager* manager, QObject* receiver, Completion completion) {
    QString reason;
    if (!isEndpointConfigured(&reason) || manager == nullptr || receiver == nullptr) {
        IssueReportSubmissionResult result;
        result.errorMessage = reason.isEmpty() ? QStringLiteral("Report service unavailable.") : reason;
        completion(result);
        return;
    }

    QNetworkRequest request(QUrl(configuredEndpoint()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Genesys-Simulator/ReportIssueRelay"));
    request.setRawHeader("Accept", "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    const QString clientKey = optionalClientKey();
    if (!clientKey.isEmpty()) {
        request.setRawHeader("X-Genesys-Client-Key", clientKey.toUtf8());
    }

    QNetworkReply* reply = manager->post(request, serializePayload(draft, context));
    auto* timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    QObject::connect(timeout, &QTimer::timeout, reply, [reply]() {
        reply->setProperty("genesysIssueTimedOut", true);
        reply->abort();
    });
    QObject::connect(reply, &QNetworkReply::finished, receiver,
                     [reply, timeout, completion = std::move(completion)]() {
        timeout->stop();
        const auto result = parseResponse(
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
            reply->readAll(), reply->error(),
            reply->property("genesysIssueTimedOut").toBool());
        reply->deleteLater();
        completion(result);
    });
    timeout->start(requestTimeoutMs());
}
