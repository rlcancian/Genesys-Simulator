#include "UpdateDialog.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QProgressDialog>

namespace genesys::launcher {

UpdateDialog::UpdateDialog(QWidget* parent)
    : parent_(parent) {}

UpdateDialog::~UpdateDialog() = default;

bool UpdateDialog::confirmUpdate(const Version& currentVersion,
                                 const Version& candidateVersion,
                                 const QString& releaseNotes) {
    QString text = QStringLiteral("A GenESyS runtime update is available.\n\nCurrent: %1\nAvailable: %2")
        .arg(currentVersion.isValid() ? currentVersion.toString() : QStringLiteral("system/base"),
             candidateVersion.toString());
    if (!releaseNotes.trimmed().isEmpty()) {
        text += QStringLiteral("\n\n%1").arg(releaseNotes.trimmed().left(2048));
    }
    text += QStringLiteral("\n\nThe installed system version remains available as fallback.");

    QMessageBox box(QMessageBox::Information,
                    QStringLiteral("GenESyS Update"),
                    text,
                    QMessageBox::NoButton,
                    parent_);
    QPushButton* update = box.addButton(QStringLiteral("Update"), QMessageBox::AcceptRole);
    box.addButton(QStringLiteral("Not now"), QMessageBox::RejectRole);
    box.setDefaultButton(update);
    box.exec();
    return box.clickedButton() == update;
}

void UpdateDialog::beginDownload(const qint64 totalBytes) {
    progress_ = std::make_unique<QProgressDialog>(
        QStringLiteral("Downloading and validating the GenESyS runtime…"),
        QStringLiteral("Cancel"),
        0,
        100,
        parent_);
    progress_->setWindowTitle(QStringLiteral("GenESyS Update"));
    progress_->setWindowModality(Qt::ApplicationModal);
    progress_->setMinimumDuration(0);
    progress_->setAutoClose(false);
    progress_->setAutoReset(false);
    progress_->setProperty("genesysTotalBytes", totalBytes);
    progress_->setValue(0);
    progress_->show();
    QCoreApplication::processEvents();
}

bool UpdateDialog::updateDownloadProgress(const qint64 receivedBytes,
                                          const qint64 totalBytes) {
    if (!progress_) {
        return true;
    }
    const qint64 denominator = totalBytes > 0
        ? totalBytes
        : progress_->property("genesysTotalBytes").toLongLong();
    if (denominator > 0) {
        const int percent = static_cast<int>(qBound<qint64>(0, (receivedBytes * 100) / denominator, 100));
        progress_->setValue(percent);
    }
    QCoreApplication::processEvents();
    return !progress_->wasCanceled();
}

void UpdateDialog::finishDownload() {
    if (progress_) {
        progress_->setValue(100);
        progress_->close();
        progress_.reset();
    }
}

void UpdateDialog::showError(const QString& message) {
    QMessageBox::warning(parent_, QStringLiteral("GenESyS Update"), message.left(2048));
}

void UpdateDialog::showInfo(const QString& message) {
    QMessageBox::information(parent_, QStringLiteral("GenESyS Update"), message.left(2048));
}

} // namespace genesys::launcher
