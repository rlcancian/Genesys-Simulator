#include "PlatformInfo.h"

#include <QFile>
#include <QRegularExpression>
#include <QSysInfo>
#include <QTextStream>

namespace genesys::launcher {
namespace {

QString unquote(QString value) {
    value = value.trimmed();
    if (value.size() >= 2 && ((value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))) ||
                              (value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\''))))) {
        value = value.mid(1, value.size() - 2);
    }
    return value;
}

QString normalizeArchitecture(QString architecture) {
    architecture = architecture.trimmed().toLower();
    if (architecture == QStringLiteral("amd64") || architecture == QStringLiteral("x86_64")) {
        return QStringLiteral("x86_64");
    }
    if (architecture == QStringLiteral("aarch64") || architecture == QStringLiteral("arm64")) {
        return QStringLiteral("aarch64");
    }
    return architecture;
}

bool safeKeyPart(const QString& value) {
    return !value.isEmpty() && !value.contains(QRegularExpression(QStringLiteral("[^a-z0-9._-]")));
}

} // namespace

PlatformInfo PlatformInfo::detect(const QString& osReleasePath, const QString& architectureOverride) {
    PlatformInfo result;
    QFile file(osReleasePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return result;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.startsWith(QStringLiteral("ID="))) {
            result.osId = unquote(line.mid(3)).toLower();
        } else if (line.startsWith(QStringLiteral("VERSION_ID="))) {
            result.osVersion = unquote(line.mid(11)).toLower();
        }
    }

    result.architecture = normalizeArchitecture(architectureOverride.isEmpty()
        ? QSysInfo::currentCpuArchitecture()
        : architectureOverride);

    if (!safeKeyPart(result.osId) || !safeKeyPart(result.osVersion) || !safeKeyPart(result.architecture)) {
        return result;
    }

    result.manifestKey = QStringLiteral("%1-%2-%3").arg(result.osId, result.osVersion, result.architecture);
    result.valid = true;
    return result;
}

} // namespace genesys::launcher
