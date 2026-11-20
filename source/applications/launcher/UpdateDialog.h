#pragma once

#include "Version.h"

#include <QString>

#include <memory>

class QProgressDialog;
class QWidget;

namespace genesys::launcher {

class IUpdateUi {
public:
    virtual ~IUpdateUi() = default;

    [[nodiscard]] virtual bool confirmUpdate(const Version& currentVersion,
                                             const Version& candidateVersion,
                                             const QString& releaseNotes) = 0;
    virtual void beginDownload(qint64 totalBytes) = 0;
    [[nodiscard]] virtual bool updateDownloadProgress(qint64 receivedBytes,
                                                      qint64 totalBytes) = 0;
    virtual void finishDownload() = 0;
    virtual void showError(const QString& message) = 0;
    virtual void showInfo(const QString& message) = 0;
};

class UpdateDialog final : public IUpdateUi {
public:
    explicit UpdateDialog(QWidget* parent = nullptr);
    ~UpdateDialog() override;

    [[nodiscard]] bool confirmUpdate(const Version& currentVersion,
                                     const Version& candidateVersion,
                                     const QString& releaseNotes) override;
    void beginDownload(qint64 totalBytes) override;
    [[nodiscard]] bool updateDownloadProgress(qint64 receivedBytes,
                                              qint64 totalBytes) override;
    void finishDownload() override;
    void showError(const QString& message) override;
    void showInfo(const QString& message) override;

private:
    QWidget* parent_ = nullptr;
    std::unique_ptr<QProgressDialog> progress_;
};

} // namespace genesys::launcher
