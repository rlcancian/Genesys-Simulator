#pragma once

#include <QByteArray>
#include <QIODevice>
#include <QString>
#include <QUrl>

#include <functional>

namespace genesys::launcher {

struct NetworkRequestOptions {
    qint64 maximumBytes = 1024 * 1024;
    int timeoutMs = 8000;
    int maximumRedirects = 3;
    bool httpsOnly = true;
};

struct NetworkResponse {
    bool ok = false;
    bool cancelled = false;
    int httpStatus = 0;
    qint64 bytesReceived = 0;
    QUrl finalUrl;
    QString error;
};

using ProgressCallback = std::function<void(qint64, qint64)>;
using CancellationCallback = std::function<bool()>;

class INetworkTransport {
public:
    virtual ~INetworkTransport() = default;

    virtual NetworkResponse download(const QUrl& url,
                                     QIODevice& destination,
                                     const NetworkRequestOptions& options,
                                     ProgressCallback progress = {},
                                     CancellationCallback cancelled = {}) = 0;

    [[nodiscard]] virtual std::pair<NetworkResponse, QByteArray> get(
        const QUrl& url,
        const NetworkRequestOptions& options,
        ProgressCallback progress = {},
        CancellationCallback cancelled = {});
};

class QtNetworkTransport final : public INetworkTransport {
public:
    NetworkResponse download(const QUrl& url,
                             QIODevice& destination,
                             const NetworkRequestOptions& options,
                             ProgressCallback progress = {},
                             CancellationCallback cancelled = {}) override;
};

} // namespace genesys::launcher
