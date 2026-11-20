#include "NetworkTransport.h"

#include <QBuffer>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace genesys::launcher {

std::pair<NetworkResponse, QByteArray> INetworkTransport::get(
    const QUrl& url,
    const NetworkRequestOptions& options,
    ProgressCallback progress,
    CancellationCallback cancelled) {
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    NetworkResponse response = download(url, buffer, options, std::move(progress), std::move(cancelled));
    buffer.close();
    return {std::move(response), std::move(data)};
}

NetworkResponse QtNetworkTransport::download(const QUrl& url,
                                             QIODevice& destination,
                                             const NetworkRequestOptions& options,
                                             ProgressCallback progress,
                                             CancellationCallback cancelled) {
    NetworkResponse result;
    if (!url.isValid() || url.host().isEmpty() ||
        (options.httpsOnly && url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0)) {
        result.error = QStringLiteral("Network request URL is invalid or violates the HTTPS-only policy");
        return result;
    }
    if (!destination.isWritable()) {
        result.error = QStringLiteral("Network destination is not writable");
        return result;
    }
    if (options.maximumBytes <= 0 || options.timeoutMs <= 0 || options.maximumRedirects < 0) {
        result.error = QStringLiteral("Invalid bounded network request options");
        return result;
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setMaximumRedirectsAllowed(options.maximumRedirects);
    request.setTransferTimeout(options.timeoutMs);

    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QTimer cancelPoll;
    cancelPoll.setInterval(50);

    bool exceededLimit = false;
    bool destinationFailed = false;
    bool timedOut = false;

    QObject::connect(reply, &QNetworkReply::readyRead, [&]() {
        const QByteArray chunk = reply->readAll();
        if (chunk.isEmpty()) {
            return;
        }
        if (result.bytesReceived > options.maximumBytes - chunk.size()) {
            exceededLimit = true;
            reply->abort();
            return;
        }
        if (destination.write(chunk) != chunk.size()) {
            destinationFailed = true;
            reply->abort();
            return;
        }
        result.bytesReceived += chunk.size();
    });
    QObject::connect(reply, &QNetworkReply::downloadProgress, [&](const qint64 received, const qint64 total) {
        if (total > options.maximumBytes) {
            exceededLimit = true;
            reply->abort();
            return;
        }
        if (progress) {
            progress(received, total);
        }
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, [&]() {
        timedOut = true;
        reply->abort();
    });
    QObject::connect(&cancelPoll, &QTimer::timeout, [&]() {
        if (cancelled && cancelled()) {
            result.cancelled = true;
            reply->abort();
        }
    });

    timeout.start(options.timeoutMs);
    if (cancelled) {
        cancelPoll.start();
    }
    loop.exec();

    timeout.stop();
    cancelPoll.stop();
    const QByteArray tail = reply->readAll();
    if (!tail.isEmpty() && !exceededLimit && !destinationFailed) {
        if (result.bytesReceived > options.maximumBytes - tail.size()) {
            exceededLimit = true;
        } else if (destination.write(tail) != tail.size()) {
            destinationFailed = true;
        } else {
            result.bytesReceived += tail.size();
        }
    }

    result.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    result.finalUrl = reply->url();

    if (result.cancelled) {
        result.error = QStringLiteral("Network request cancelled");
    } else if (timedOut) {
        result.error = QStringLiteral("Network request timed out");
    } else if (exceededLimit) {
        result.error = QStringLiteral("Network response exceeded configured size limit");
    } else if (destinationFailed) {
        result.error = QStringLiteral("Failed to write network response to destination");
    } else if (reply->error() != QNetworkReply::NoError) {
        result.error = reply->errorString();
    } else if (result.httpStatus < 200 || result.httpStatus >= 300) {
        result.error = QStringLiteral("Unexpected HTTP status %1").arg(result.httpStatus);
    } else if (options.httpsOnly && result.finalUrl.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0) {
        result.error = QStringLiteral("Redirect left HTTPS transport");
    } else {
        result.ok = true;
    }

    reply->deleteLater();
    return result;
}

} // namespace genesys::launcher
