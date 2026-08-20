#include "ArchiveExtractor.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

namespace genesys::launcher {
namespace {

struct ProcessResult {
    bool ok = false;
    QByteArray out;
    QString error;
};

ProcessResult runTar(const QString& tar,
                     const QStringList& arguments,
                     const int timeoutMs) {
    ProcessResult result;
    QProcess process;
    process.setProgram(tar);
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start();
    if (!process.waitForStarted(3000)) {
        result.error = QStringLiteral("Unable to start tar");
        return result;
    }
    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(3000);
        result.error = QStringLiteral("Archive operation timed out");
        return result;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString diagnostic = QString::fromUtf8(process.readAllStandardError()).trimmed().left(2048);
        result.error = diagnostic.isEmpty()
            ? QStringLiteral("tar rejected the runtime archive")
            : QStringLiteral("tar rejected the runtime archive: %1").arg(diagnostic);
        return result;
    }
    result.ok = true;
    result.out = process.readAllStandardOutput();
    return result;
}

bool unsafePath(const QString& raw) {
    if (raw.isEmpty() || raw.contains(QLatin1Char('\\')) || raw.contains(QChar::Null)) {
        return true;
    }
    const QString path = raw.endsWith(QLatin1Char('/')) ? raw.left(raw.size() - 1) : raw;
    if (path.isEmpty() || QDir::isAbsolutePath(path) || path.startsWith(QLatin1Char('/'))) {
        return true;
    }
    const QStringList parts = path.split(QLatin1Char('/'), Qt::KeepEmptyParts);
    for (const QString& part : parts) {
        if (part.isEmpty() || part == QStringLiteral(".") || part == QStringLiteral("..")) {
            return true;
        }
    }
    return false;
}

} // namespace

TarZstdArchiveExtractor::TarZstdArchiveExtractor(const int timeoutMs)
    : timeoutMs_(timeoutMs) {}

ExtractionResult TarZstdArchiveExtractor::validatePathListing(const QByteArray& listing) {
    ExtractionResult result;
    const QList<QByteArray> lines = listing.split('\n');
    int entries = 0;
    for (QByteArray line : lines) {
        if (line.endsWith('\r')) {
            line.chop(1);
        }
        if (line.isEmpty()) {
            continue;
        }
        ++entries;
        const QString path = QString::fromUtf8(line);
        if (unsafePath(path)) {
            result.error = QStringLiteral("Archive contains an unsafe path: %1").arg(path.left(256));
            return result;
        }
    }
    if (entries == 0) {
        result.error = QStringLiteral("Archive contains no entries");
        return result;
    }
    result.ok = true;
    return result;
}

ExtractionResult TarZstdArchiveExtractor::validateVerboseListing(const QByteArray& listing) {
    ExtractionResult result;
    const QList<QByteArray> lines = listing.split('\n');
    int entries = 0;
    for (QByteArray line : lines) {
        if (line.endsWith('\r')) {
            line.chop(1);
        }
        if (line.isEmpty()) {
            continue;
        }
        ++entries;
        const char type = line.at(0);
        if (type != '-' && type != 'd') {
            result.error = QStringLiteral("Archive entry type is not allowed (only regular files and directories are accepted)");
            return result;
        }
        const int firstSpace = line.indexOf(' ');
        const QByteArray mode = firstSpace < 0 ? line : line.left(firstSpace);
        if (mode.contains('s') || mode.contains('S')) {
            result.error = QStringLiteral("Archive contains setuid/setgid permissions");
            return result;
        }
    }
    if (entries == 0) {
        result.error = QStringLiteral("Archive verbose listing contains no entries");
        return result;
    }
    result.ok = true;
    return result;
}

ExtractionResult TarZstdArchiveExtractor::inspect(const QString& archivePath) const {
    ExtractionResult result;
    if (!QFileInfo(archivePath).isFile()) {
        result.error = QStringLiteral("Runtime archive is unavailable");
        return result;
    }
    const QString tar = QStandardPaths::findExecutable(QStringLiteral("tar"));
    if (tar.isEmpty()) {
        result.error = QStringLiteral("tar is unavailable; runtime extraction cannot proceed");
        return result;
    }

    ProcessResult names = runTar(tar,
                                 {QStringLiteral("--zstd"), QStringLiteral("--list"),
                                  QStringLiteral("--quoting-style=escape"),
                                  QStringLiteral("--file"), archivePath},
                                 timeoutMs_);
    if (!names.ok) {
        result.error = names.error;
        return result;
    }
    result = validatePathListing(names.out);
    if (!result.ok) {
        return result;
    }

    ProcessResult verbose = runTar(tar,
                                   {QStringLiteral("--zstd"), QStringLiteral("--list"),
                                    QStringLiteral("--verbose"), QStringLiteral("--numeric-owner"),
                                    QStringLiteral("--quoting-style=escape"),
                                    QStringLiteral("--file"), archivePath},
                                   timeoutMs_);
    if (!verbose.ok) {
        result.error = verbose.error;
        return result;
    }
    return validateVerboseListing(verbose.out);
}

ExtractionResult TarZstdArchiveExtractor::validateExtractedTree(const QString& destinationRoot) {
    ExtractionResult result;
    const QFileInfo rootInfo(destinationRoot);
    const QString canonicalRoot = rootInfo.canonicalFilePath();
    if (canonicalRoot.isEmpty() || !rootInfo.isDir()) {
        result.error = QStringLiteral("Extracted runtime root is invalid");
        return result;
    }

    QDirIterator iterator(destinationRoot,
                          QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        const QFileInfo info = iterator.fileInfo();
        if (info.isSymLink()) {
            result.error = QStringLiteral("Extracted runtime contains a symbolic link, which is not accepted");
            return result;
        }
        if (!info.isFile() && !info.isDir()) {
            result.error = QStringLiteral("Extracted runtime contains a non-regular filesystem entry");
            return result;
        }
        const QString canonical = info.canonicalFilePath();
        if (canonical.isEmpty() ||
            !(canonical == canonicalRoot || canonical.startsWith(canonicalRoot + QLatin1Char('/')))) {
            result.error = QStringLiteral("Extracted runtime escaped the destination root");
            return result;
        }
    }

    result.ok = true;
    return result;
}

ExtractionResult TarZstdArchiveExtractor::extract(const QString& archivePath,
                                                  const QString& destinationRoot) {
    ExtractionResult inspection = inspect(archivePath);
    if (!inspection.ok) {
        return inspection;
    }

    QDir destination(destinationRoot);
    if (!destination.exists() && !QDir().mkpath(destinationRoot)) {
        return {false, QStringLiteral("Unable to create partial runtime directory")};
    }
    if (!destination.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty()) {
        return {false, QStringLiteral("Partial runtime directory must be empty before extraction")};
    }

    const QString tar = QStandardPaths::findExecutable(QStringLiteral("tar"));
    ProcessResult extracted = runTar(tar,
                                     {QStringLiteral("--zstd"), QStringLiteral("--extract"),
                                      QStringLiteral("--no-same-owner"), QStringLiteral("--no-same-permissions"),
                                      QStringLiteral("--delay-directory-restore"),
                                      QStringLiteral("--directory"), destinationRoot,
                                      QStringLiteral("--file"), archivePath},
                                     timeoutMs_);
    if (!extracted.ok) {
        return {false, extracted.error};
    }
    return validateExtractedTree(destinationRoot);
}

} // namespace genesys::launcher
