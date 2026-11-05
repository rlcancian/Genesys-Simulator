#include "../dialogs/DialogReportIssue.h"
#include "../services/IssueReportBrowserService.h"
#include "../services/IssueReportRelayService.h"

#include <QApplication>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QUrlQuery>
#include <algorithm>
#include <cstring>

namespace {
constexpr auto kIssueUrl =
    "https://github.com/rlcancian/Genesys-Simulator/issues/42";
const QByteArray kCreated = R"({"issue":{"number":42,"html_url":"https://github.com/rlcancian/Genesys-Simulator/issues/42"}})";

class FakeReply final : public QNetworkReply {
public:
    FakeReply(const QNetworkRequest& request, Operation operation,
              QByteArray response, int status, int delayMs,
              QNetworkReply::NetworkError failure, QObject* parent)
        : QNetworkReply(parent), _response(std::move(response)) {
        setRequest(request);
        setUrl(request.url());
        setOperation(operation);
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        if (delayMs >= 0) {
            QTimer::singleShot(delayMs, this, [this, status, failure]() {
                if (isFinished()) return;
                if (status > 0) setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);
                if (failure != QNetworkReply::NoError) setError(failure, QStringLiteral("mock error"));
                setFinished(true);
                emit readyRead();
                emit finished();
            });
        }
    }

    void abort() override {
        if (isFinished()) return;
        setError(QNetworkReply::OperationCanceledError, QStringLiteral("aborted"));
        setFinished(true);
        emit finished();
    }

    qint64 bytesAvailable() const override {
        return _response.size() - _offset + QNetworkReply::bytesAvailable();
    }

protected:
    qint64 readData(char* buffer, qint64 maxSize) override {
        if (_offset >= _response.size()) return -1;
        const qint64 count = std::min<qint64>(maxSize, _response.size() - _offset);
        std::memcpy(buffer, _response.constData() + _offset, static_cast<size_t>(count));
        _offset += count;
        return count;
    }

private:
    QByteArray _response;
    qint64 _offset = 0;
};

class FakeNetworkManager final : public QNetworkAccessManager {
public:
    QByteArray response = kCreated;
    QByteArray payload;
    int status = 201;
    int delayMs = 0; // -1 never completes until timed out
    int postCount = 0;
    QNetworkReply::NetworkError failure = QNetworkReply::NoError;
    QNetworkRequest lastRequest;

protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest& request,
                                 QIODevice* outgoing) override {
        ++postCount;
        lastRequest = request;
        if (outgoing != nullptr) payload = outgoing->readAll();
        return new FakeReply(request, op, response, status, delayMs, failure, this);
    }
};

IssueReportDraft sample() {
    IssueReportDraft d;
    d.categoryKey = QStringLiteral("bug");
    d.categoryLabel = QStringLiteral("Bug");
    d.title = QStringLiteral("Unexpected crash");
    d.summary = QStringLiteral("The editor closes unexpectedly.");
    return d;
}

void closeMessageBox() {
    for (QWidget* w : QApplication::topLevelWidgets()) {
        if (auto* box = qobject_cast<QMessageBox*>(w)) {
            box->accept();
        }
    }
}

QLineEdit* title(DialogReportIssue& dialog) {
    return dialog.findChild<QLineEdit*>(QStringLiteral("issueTitle"));
}
QPlainTextEdit* description(DialogReportIssue& dialog) {
    return dialog.findChild<QPlainTextEdit*>(QStringLiteral("issueDescription"));
}
QPushButton* submit(DialogReportIssue& dialog) {
    return dialog.findChild<QPushButton*>(QStringLiteral("issueSubmit"));
}
} // namespace

class IssueReportingTests : public QObject {
    Q_OBJECT
private slots:
    void init() {
        qunsetenv("GENESYS_REPORT_ISSUE_RELAY_URL");
        qunsetenv("GENESYS_REPORT_ISSUE_TIMEOUT_MS");
        qunsetenv("GENESYS_REPORT_ISSUE_CLIENT_KEY");
    }

    void formValidationAndType() {
        DialogReportIssue dialog;
        QVERIFY(title(dialog));
        QVERIFY(description(dialog));
        QVERIFY(submit(dialog));
        QVERIFY(!submit(dialog)->isEnabled());
        QCOMPARE(dialog.draft().categoryKey, QStringLiteral("bug"));
        title(dialog)->setText(QStringLiteral("   "));
        description(dialog)->setPlainText(QStringLiteral("Text"));
        QVERIFY(!submit(dialog)->isEnabled());
        title(dialog)->setText(QStringLiteral("  Title  "));
        QVERIFY(submit(dialog)->isEnabled());
        description(dialog)->setPlainText(QStringLiteral("   "));
        QVERIFY(!submit(dialog)->isEnabled());
        description(dialog)->setPlainText(QStringLiteral("  Details\nmore  "));
        QVERIFY(submit(dialog)->isEnabled());
        QCOMPARE(dialog.draft().title, QStringLiteral("Title"));
        QCOMPARE(dialog.draft().summary, QStringLiteral("Details\nmore"));
        auto* combo = dialog.findChild<QComboBox*>(QStringLiteral("issueType"));
        QCOMPARE(combo->count(), 3);
        combo->setCurrentIndex(1);
        QCOMPARE(dialog.draft().categoryKey, QStringLiteral("enhancement"));
        combo->setCurrentIndex(2);
        QCOMPARE(dialog.draft().categoryKey, QStringLiteral("question"));
        const auto report = dialog.draft();
        QVERIFY(!report.includeLogTail);
        QVERIFY(!report.includeModelText);
        QVERIFY(!report.includeScreenshot);
        QVERIFY(report.contactEmail.isEmpty());
        QVERIFY(report.reproductionSteps.isEmpty());
    }

    void cancelDoesNotSubmit() {
        FakeNetworkManager manager;
        bool opened = false;
        DialogReportIssue dialog(nullptr, &manager, [&opened](const QUrl&) {
            opened = true;
            return true;
        });
        title(dialog)->setText(QStringLiteral("Title"));
        description(dialog)->setPlainText(QStringLiteral("Description"));
        QTimer::singleShot(0, &dialog, &QDialog::reject);
        QCOMPARE(dialog.exec(), int(QDialog::Rejected));
        QCOMPARE(manager.postCount, 0);
        QVERIFY(!opened);
    }

    void browserUrlEncodesUnicodeAndReservedCharacters() {
        IssueReportDraft d = sample();
        d.title = QString::fromUtf8("São José / 漢字 ? & # % + \" \\");
        d.summary = QString::fromUtf8("Primeira linha\nSegunda 😊 ?&#+%\"/\\");
        const auto result = IssueReportBrowserService::build(d);
        QCOMPARE(result.prefill, IssueReportBrowserService::Prefill::Complete);
        QCOMPARE(result.url.scheme(), QStringLiteral("https"));
        QCOMPARE(result.url.host(), QStringLiteral("github.com"));
        QCOMPARE(result.url.path(), QStringLiteral("/rlcancian/Genesys-Simulator/issues/new"));
        const QUrlQuery query(result.url);
        QCOMPARE(query.queryItemValue(QStringLiteral("title"), QUrl::FullyDecoded), d.title);
        QCOMPARE(query.queryItemValue(QStringLiteral("body"), QUrl::FullyDecoded),
                 IssueReportBrowserService::body(d));
        QCOMPARE(query.queryItems().size(), 2);
        QVERIFY(result.url.toEncoded().contains("title="));
        QVERIFY(result.url.toEncoded().contains("%"));
        QVERIFY(!result.url.toEncoded().contains("token="));
    }

    void oversizedBrowserUrlFallsBackWithoutTruncation() {
        IssueReportDraft d = sample();
        d.summary = QString(5000, QChar('x'));
        const auto partial = IssueReportBrowserService::build(d);
        QCOMPARE(partial.prefill, IssueReportBrowserService::Prefill::BodyRequiresPaste);
        QVERIFY(partial.url.toEncoded().size() <= 1800);
        QVERIFY(!QUrlQuery(partial.url).hasQueryItem(QStringLiteral("body")));
        QCOMPARE(IssueReportBrowserService::body(d).size(), d.summary.size() + 14);
        d.title = QString(5000, QChar('z'));
        const auto empty = IssueReportBrowserService::build(d);
        QCOMPARE(empty.prefill, IssueReportBrowserService::Prefill::AllRequiresPaste);
        QVERIFY(empty.url.query().isEmpty());
    }

    void browserFailureKeepsDraft() {
        int browserAttempts = 0;
        DialogReportIssue dialog(nullptr, nullptr,
                                 [&browserAttempts](const QUrl&) {
                                     ++browserAttempts;
                                     return false;
                                 });
        title(dialog)->setText(QStringLiteral("Keep title"));
        description(dialog)->setPlainText(QStringLiteral("Keep description"));
        QTimer::singleShot(0, &closeMessageBox);
        submit(dialog)->click();
        QCOMPARE(browserAttempts, 1);
        QCOMPARE(dialog.result(), int(QDialog::Rejected));
        QCOMPARE(dialog.draft().title, QStringLiteral("Keep title"));
        QCOMPARE(dialog.draft().summary, QStringLiteral("Keep description"));
    }

    void browserHandoffIsNotIssueCreation() {
        QUrl opened;
        DialogReportIssue dialog(nullptr, nullptr, [&opened](const QUrl& url) {
            opened = url;
            return true;
        });
        title(dialog)->setText(QStringLiteral("Title"));
        description(dialog)->setPlainText(QStringLiteral("Description"));
        QTimer::singleShot(0, &closeMessageBox);
        submit(dialog)->click();
        QVERIFY(opened.isValid());
        QCOMPARE(dialog.result(), int(QDialog::Accepted)); // Handoff only
        QVERIFY(QUrlQuery(opened).hasQueryItem(QStringLiteral("body")));
    }

    void relayResponseValidation() {
        using S = IssueReportRelayService;
        auto success = S::parseResponse(201, kCreated);
        QVERIFY(success.success);
        QCOMPARE(success.issueNumber, 42);
        QCOMPARE(success.issueUrl, QString::fromLatin1(kIssueUrl));

        QVERIFY(!S::parseResponse(200, QByteArray("{}")).success);
        QVERIFY(!S::parseResponse(200, QByteArray("not json")).success);
        QVERIFY(!S::parseResponse(200, QByteArray(R"({"issue":{"number":42,"html_url":"https://evil.example/issues/42"}})")).success);
        QVERIFY(!S::parseResponse(200, QByteArray(R"({"issue":{"number":43,"html_url":"https://github.com/rlcancian/Genesys-Simulator/issues/42"}})")).success);
        QVERIFY(!S::parseResponse(200, QByteArray(R"({"issue":{"number":42,"html_url":"https://github.com/rlcancian/Genesys-Simulator/issues/42?steal=yes"}})")).success);
        QVERIFY(!S::parseResponse(200, QByteArray(R"({"issue":{"number":42,"html_url":"http://github.com/rlcancian/Genesys-Simulator/issues/42"}})")).success);
        for (const int status : {400, 401, 403, 429, 500, 503, 302}) {
            QVERIFY2(!S::parseResponse(status, kCreated).success, "HTTP failure reported as success");
        }
        QVERIFY(!S::parseResponse(0, QByteArray(), QNetworkReply::ConnectionRefusedError).success);
        QVERIFY(!S::parseResponse(0, QByteArray(), QNetworkReply::OperationCanceledError, true).success);
    }

    void endpointValidation() {
        QString reason;
        QVERIFY(!IssueReportRelayService::isEndpointConfigured(&reason));
        QVERIFY(!reason.isEmpty());
        for (const QByteArray bad : {
                QByteArray("http://relay.example.test/api"),
                QByteArray("https://relay.example.test/path#fragment"),
                QByteArray("https://user:password@relay.example.test/api"),
                QByteArray("https://")}) {
            qputenv("GENESYS_REPORT_ISSUE_RELAY_URL", bad);
            QVERIFY(!IssueReportRelayService::isEndpointConfigured());
        }
        qputenv("GENESYS_REPORT_ISSUE_RELAY_URL",
                "https://relay.example.test/api/report");
        QVERIFY(IssueReportRelayService::isEndpointConfigured());
    }

    void relayPayloadExcludesDiagnostics() {
        auto d = sample();
        d.includeLogTail = true;
        IssueReportRuntimeContext context;
        context.applicationVersion = QStringLiteral("1.0");
        context.qtVersion = QStringLiteral("6.8");
        context.currentModelName = QStringLiteral("/private/model-name");
        context.currentModelSimulanguage = QStringLiteral("SECRET_MODEL");
        context.consoleTail = QStringLiteral("SECRET_CONSOLE");
        context.processLogTail = QStringLiteral("SECRET_LOG");
        context.screenshotPngBase64 = QByteArray("SECRET_IMAGE");

        const QByteArray payload = IssueReportRelayService::serializePayload(d, context);
        const auto parsed = QJsonDocument::fromJson(payload).object();
        QCOMPARE(parsed.value(QStringLiteral("schema_version")).toInt(), 1);
        QCOMPARE(parsed.value(QStringLiteral("report")).toObject()
                     .value(QStringLiteral("summary")).toString(), d.summary);
        QVERIFY(parsed.value(QStringLiteral("diagnostics")).toObject().isEmpty());
        for (const QByteArray forbidden : {QByteArray("SECRET_MODEL"), QByteArray("SECRET_CONSOLE"),
                                           QByteArray("SECRET_LOG"), QByteArray("SECRET_IMAGE"),
                                           QByteArray("/private/model-name")}) {
            QVERIFY(!payload.contains(forbidden));
        }
    }

    void relayMockTransportAndDuplicateGuard() {
        qputenv("GENESYS_REPORT_ISSUE_RELAY_URL",
                "https://relay.example.test/api/report");
        qputenv("GENESYS_REPORT_ISSUE_TIMEOUT_MS", "80");
        FakeNetworkManager manager;
        manager.delayMs = -1;

        DialogReportIssue dialog(nullptr, &manager);
        dialog.setRelayAvailable(true);
        title(dialog)->setText(QStringLiteral("Test Title"));
        description(dialog)->setPlainText(QStringLiteral("Test Body"));
        submit(dialog)->click();
        QVERIFY(!submit(dialog)->isEnabled());
        QCOMPARE(manager.postCount, 1);
        submit(dialog)->click();
        QCOMPARE(manager.postCount, 1);
        QCOMPARE(manager.lastRequest.url().scheme(), QStringLiteral("https"));
        QCOMPARE(manager.lastRequest.attribute(QNetworkRequest::RedirectPolicyAttribute).toInt(),
                 int(QNetworkRequest::ManualRedirectPolicy));
        QVERIFY(!manager.payload.contains("SECRET"));
        // Let the simulated timeout complete and dismiss the warning.
        QTimer::singleShot(110, &closeMessageBox);
        QTRY_VERIFY_WITH_TIMEOUT(submit(dialog)->isEnabled(), 2000);
        QCOMPARE(dialog.draft().summary, QStringLiteral("Test Body"));
    }

    void relayMockSuccessfulResponse() {
        qputenv("GENESYS_REPORT_ISSUE_RELAY_URL",
                "https://relay.example.test/api/report");
        FakeNetworkManager manager;
        bool finished = false;
        IssueReportSubmissionResult result;
        IssueReportRelayService::submitAsync(
            sample(), {}, &manager, this,
            [&](const auto& value) { result = value; finished = true; });
        QTRY_VERIFY_WITH_TIMEOUT(finished, 1000);
        QCOMPARE(manager.postCount, 1);
        QCOMPARE(manager.lastRequest.header(QNetworkRequest::ContentTypeHeader).toString(),
                 QStringLiteral("application/json"));
        QVERIFY(result.success);
        QCOMPARE(result.issueNumber, 42);
        QCOMPARE(result.issueUrl, QString::fromLatin1(kIssueUrl));
    }

    void relayMockBadResponse() {
        qputenv("GENESYS_REPORT_ISSUE_RELAY_URL",
                "https://relay.example.test/api/report");
        FakeNetworkManager manager;
        manager.status = 200;
        manager.response = QByteArray("{}");
        bool finished = false;
        IssueReportSubmissionResult result;
        IssueReportRelayService::submitAsync(
            sample(), {}, &manager, this,
            [&](const auto& value) { result = value; finished = true; });
        QTRY_VERIFY_WITH_TIMEOUT(finished, 1000);
        QVERIFY(!result.success);
        QVERIFY(!result.errorMessage.isEmpty());
    }
};

QTEST_MAIN(IssueReportingTests)
#include "test_issue_reporting.moc"
