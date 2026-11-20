#pragma once

#include <QMutex>
#include <QString>

namespace genesys::launcher {

class LauncherLogger {
public:
    explicit LauncherLogger(QString path, qint64 maxBytes = 1024 * 1024);

    void log(const QString& event, const QString& detail = {});

private:
    void rotateIfNeeded();
    static QString sanitize(QString value);

    QString path_;
    qint64 maxBytes_;
    QMutex mutex_;
};

} // namespace genesys::launcher
