#include "LauncherConfig.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QHash>

namespace genesys::launcher {
namespace {

using IniMap = QHash<QString, QString>;

IniMap readUpdatesSection(const QString& path, QStringList& warnings) {
    IniMap values;
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return values;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        warnings.push_back(QStringLiteral("Cannot read configuration: %1").arg(path));
        return values;
    }

    QTextStream stream(&file);
    bool inUpdates = false;
    qsizetype lineNo = 0;
    while (!stream.atEnd()) {
        ++lineNo;
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) || line.startsWith(QLatin1Char(';'))) {
            continue;
        }
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            inUpdates = line.mid(1, line.size() - 2).trimmed() == QStringLiteral("updates");
            continue;
        }
        if (!inUpdates) {
            continue;
        }
        const qsizetype separator = line.indexOf(QLatin1Char('='));
        if (separator <= 0) {
            warnings.push_back(QStringLiteral("Invalid configuration line %1 in %2").arg(lineNo).arg(path));
            continue;
        }
        const QString key = line.left(separator).trimmed().toLower();
        const QString value = line.mid(separator + 1).trimmed();
        values.insert(key, value);
    }
    return values;
}

bool parseBool(const QString& value, bool* ok) {
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("true") || normalized == QStringLiteral("yes") || normalized == QStringLiteral("1") || normalized == QStringLiteral("on")) {
        *ok = true;
        return true;
    }
    if (normalized == QStringLiteral("false") || normalized == QStringLiteral("no") || normalized == QStringLiteral("0") || normalized == QStringLiteral("off")) {
        *ok = true;
        return false;
    }
    *ok = false;
    return false;
}

void applyValues(LauncherConfig& config, const IniMap& values, bool systemPolicy, const QString& sourceName) {
    auto applyBool = [&](const QString& key, bool& target, bool safeSystemValue) {
        if (!values.contains(key)) {
            return;
        }
        bool ok = false;
        const bool parsed = parseBool(values.value(key), &ok);
        if (!ok) {
            config.warnings.push_back(QStringLiteral("Invalid boolean '%1' for %2 in %3").arg(values.value(key), key, sourceName));
            if (systemPolicy) {
                target = safeSystemValue;
            }
            return;
        }
        target = parsed;
    };

    applyBool(QStringLiteral("enabled"), config.updatesEnabled, false);
    applyBool(QStringLiteral("allow_user_runtime"), config.allowUserRuntime, false);
    applyBool(QStringLiteral("require_signature"), config.requireSignature, true);
    applyBool(QStringLiteral("check_on_gui_startup"), config.checkOnGuiStartup, false);
    applyBool(QStringLiteral("fallback_to_system"), config.fallbackToSystem, true);

    if (values.contains(QStringLiteral("channel"))) {
        const QString channel = values.value(QStringLiteral("channel")).trimmed();
        if (!channel.isEmpty() && channel.size() <= 64 && !channel.contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")))) {
            config.channel = channel;
        } else {
            config.warnings.push_back(QStringLiteral("Invalid update channel in %1").arg(sourceName));
            if (systemPolicy) {
                config.channel = QStringLiteral("stable");
            }
        }
    }

    if (values.contains(QStringLiteral("minimum_check_interval_hours"))) {
        bool ok = false;
        const int value = values.value(QStringLiteral("minimum_check_interval_hours")).toInt(&ok);
        if (ok && value >= 0 && value <= 24 * 30) {
            config.minimumCheckIntervalHours = value;
        } else {
            config.warnings.push_back(QStringLiteral("Invalid minimum_check_interval_hours in %1").arg(sourceName));
            if (systemPolicy) {
                config.minimumCheckIntervalHours = 12;
            }
        }
    }

    if (values.contains(QStringLiteral("max_user_versions"))) {
        bool ok = false;
        const int value = values.value(QStringLiteral("max_user_versions")).toInt(&ok);
        if (ok && value >= 1 && value <= 20) {
            config.maxUserVersions = value;
        } else {
            config.warnings.push_back(QStringLiteral("Invalid max_user_versions in %1").arg(sourceName));
            if (systemPolicy) {
                config.maxUserVersions = 2;
            }
        }
    }

    if (values.contains(QStringLiteral("manifest_url"))) {
        const QUrl url(values.value(QStringLiteral("manifest_url")), QUrl::StrictMode);
        if (url.isValid() && url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0 && !url.host().isEmpty()) {
            config.manifestUrl = url;
        } else {
            config.warnings.push_back(QStringLiteral("Invalid or non-HTTPS manifest_url in %1").arg(sourceName));
            if (systemPolicy) {
                config.manifestUrl = {};
                config.updatesEnabled = false;
            }
        }
    }
}

} // namespace

LauncherConfig LauncherConfig::load(const QString& systemConfigPath, const QString& userConfigPath) {
    LauncherConfig config;

    QStringList userWarnings;
    const IniMap userValues = readUpdatesSection(userConfigPath, userWarnings);
    config.warnings.append(userWarnings);
    if (!userValues.isEmpty()) {
        config.hasExplicitPolicy = true;
        applyValues(config, userValues, false, userConfigPath);
    }

    QStringList systemWarnings;
    const IniMap systemValues = readUpdatesSection(systemConfigPath, systemWarnings);
    config.warnings.append(systemWarnings);
    if (!systemValues.isEmpty()) {
        config.hasExplicitPolicy = true;
        applyValues(config, systemValues, true, systemConfigPath);
    }

    if (!config.hasExplicitPolicy || config.manifestUrl.isEmpty()) {
        config.updatesEnabled = false;
    }
    return config;
}

} // namespace genesys::launcher
