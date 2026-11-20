#include "UpdateVerifier.h"

#include "LauncherLogger.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

namespace genesys::launcher {

VerificationResult UpdateVerifier::verifySha256(const QString& filePath,
                                                const QString& expectedHexDigest) {
    VerificationResult result;
    static const QRegularExpression pattern(QStringLiteral("^[0-9A-Fa-f]{64}$"));
    if (!pattern.match(expectedHexDigest).hasMatch()) {
        result.error = QStringLiteral("Expected SHA-256 digest is invalid");
        return result;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.error = QStringLiteral("Unable to open downloaded bundle for SHA-256 verification");
        return result;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        result.error = QStringLiteral("Unable to read downloaded bundle for SHA-256 verification");
        return result;
    }
    const QByteArray actual = hash.result();
    const QByteArray expected = QByteArray::fromHex(expectedHexDigest.toLatin1());
    if (actual.size() != expected.size()) {
        result.error = QStringLiteral("SHA-256 mismatch");
        return result;
    }

    unsigned char difference = 0;
    for (qsizetype i = 0; i < actual.size(); ++i) {
        difference |= static_cast<unsigned char>(actual.at(i)) ^ static_cast<unsigned char>(expected.at(i));
    }
    if (difference != 0) {
        result.error = QStringLiteral("SHA-256 mismatch");
        return result;
    }

    result.ok = true;
    return result;
}

GpgvSignatureVerifier::GpgvSignatureVerifier(QString keyringPath,
                                             LauncherLogger* logger,
                                             const int timeoutMs)
    : keyringPath_(std::move(keyringPath)), logger_(logger), timeoutMs_(timeoutMs) {}

VerificationResult GpgvSignatureVerifier::verify(const QString& dataPath,
                                                 const QString& signaturePath) {
    VerificationResult result;
    if (!QFileInfo(keyringPath_).isFile()) {
        result.error = QStringLiteral("The configured GenESyS update keyring is unavailable");
        return result;
    }
    if (!QFileInfo(dataPath).isFile() || !QFileInfo(signaturePath).isFile()) {
        result.error = QStringLiteral("Bundle or signature file is unavailable");
        return result;
    }

    const QString gpgv = QStandardPaths::findExecutable(QStringLiteral("gpgv"));
    if (gpgv.isEmpty()) {
        result.error = QStringLiteral("gpgv is unavailable; signature-required policy fails closed");
        return result;
    }

    QProcess process;
    process.setProgram(gpgv);
    process.setArguments({QStringLiteral("--keyring"), keyringPath_, signaturePath, dataPath});
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start();
    if (!process.waitForStarted(3000)) {
        result.error = QStringLiteral("Unable to start gpgv for signature verification");
        return result;
    }
    if (!process.waitForFinished(timeoutMs_)) {
        process.kill();
        process.waitForFinished(3000);
        result.error = QStringLiteral("gpgv signature verification timed out");
        return result;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString diagnostic = QString::fromUtf8(process.readAllStandardError()).trimmed().left(1024);
        result.error = diagnostic.isEmpty()
            ? QStringLiteral("gpgv rejected the runtime signature")
            : QStringLiteral("gpgv rejected the runtime signature: %1").arg(diagnostic);
        if (logger_) {
            logger_->log(QStringLiteral("signature-rejected"), result.error);
        }
        return result;
    }

    result.ok = true;
    if (logger_) {
        logger_->log(QStringLiteral("signature-verified"), QStringLiteral("OpenPGP verification succeeded"));
    }
    return result;
}

} // namespace genesys::launcher
