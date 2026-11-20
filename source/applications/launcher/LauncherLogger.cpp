#include "LauncherLogger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QTextStream>

#ifndef GENESYS_LAUNCHER_VERSION
#define GENESYS_LAUNCHER_VERSION "0.0.0"
#endif

namespace genesys::launcher {

LauncherLogger::LauncherLogger(QString path, const qint64 maxBytes)
    : path_(std::move(path)), maxBytes_(maxBytes) {}

QString LauncherLogger::sanitize(QString value) {
    value.replace(QLatin1Char('\n'), QLatin1Char(' '));
    value.replace(QLatin1Char('\r'), QLatin1Char(' '));
    return value.left(4096);
}

void LauncherLogger::rotateIfNeeded() {
    if (path_.isEmpty()) {
        return;
    }
    const QFileInfo info(path_);
    if (!info.exists() || info.size() < maxBytes_) {
        return;
    }
    const QString rotated = path_ + QStringLiteral(".1");
    QFile::remove(rotated);
    QFile::rename(path_, rotated);
}

void LauncherLogger::log(const QString& event, const QString& detail) {
    if (path_.isEmpty()) {
        return;
    }

    QMutexLocker locker(&mutex_);
    const QFileInfo info(path_);
    QDir().mkpath(info.absolutePath());
    rotateIfNeeded();

    QFile file(path_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)
           << " launcher=" << GENESYS_LAUNCHER_VERSION
           << " event=" << sanitize(event);
    if (!detail.isEmpty()) {
        stream << " detail=" << sanitize(detail);
    }
    stream << '\n';
}

} // namespace genesys::launcher
