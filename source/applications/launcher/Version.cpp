#include "Version.h"

#include <algorithm>

namespace genesys::launcher {

Version Version::parse(const QString& text) {
    Version result;
    QString normalized = text.trimmed();
    if (normalized.startsWith(QLatin1Char('v'))) {
        normalized.remove(0, 1);
    }
    if (normalized.isEmpty()) {
        return result;
    }

    const QStringList parts = normalized.split(QLatin1Char('.'), Qt::KeepEmptyParts);
    result.components_.reserve(parts.size());
    for (const QString& part : parts) {
        if (part.isEmpty()) {
            result.components_.clear();
            return result;
        }
        bool ok = false;
        const quint64 value = part.toULongLong(&ok, 10);
        if (!ok || part.contains(QRegularExpression(QStringLiteral("[^0-9]")))) {
            result.components_.clear();
            return result;
        }
        result.components_.push_back(value);
    }

    result.valid_ = !result.components_.isEmpty();
    return result;
}

QString Version::toString() const {
    if (!valid_) {
        return {};
    }
    QStringList parts;
    parts.reserve(components_.size());
    for (const quint64 value : components_) {
        parts.push_back(QString::number(value));
    }
    return parts.join(QLatin1Char('.'));
}

bool operator==(const Version& lhs, const Version& rhs) noexcept {
    if (!lhs.valid_ || !rhs.valid_) {
        return lhs.valid_ == rhs.valid_ && lhs.components_ == rhs.components_;
    }
    const qsizetype count = std::max(lhs.components_.size(), rhs.components_.size());
    for (qsizetype i = 0; i < count; ++i) {
        const quint64 a = i < lhs.components_.size() ? lhs.components_.at(i) : 0;
        const quint64 b = i < rhs.components_.size() ? rhs.components_.at(i) : 0;
        if (a != b) {
            return false;
        }
    }
    return true;
}

bool operator<(const Version& lhs, const Version& rhs) noexcept {
    if (!lhs.valid_ || !rhs.valid_) {
        return lhs.valid_ < rhs.valid_;
    }
    const qsizetype count = std::max(lhs.components_.size(), rhs.components_.size());
    for (qsizetype i = 0; i < count; ++i) {
        const quint64 a = i < lhs.components_.size() ? lhs.components_.at(i) : 0;
        const quint64 b = i < rhs.components_.size() ? rhs.components_.at(i) : 0;
        if (a != b) {
            return a < b;
        }
    }
    return false;
}

} // namespace genesys::launcher
