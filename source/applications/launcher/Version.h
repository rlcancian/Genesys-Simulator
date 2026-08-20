#pragma once

#include <QString>
#include <QVector>

namespace genesys::launcher {

class Version {
public:
    Version() = default;

    [[nodiscard]] static Version parse(const QString& text);

    [[nodiscard]] bool isValid() const noexcept { return valid_; }
    [[nodiscard]] QString toString() const;
    [[nodiscard]] const QVector<quint64>& components() const noexcept { return components_; }

    friend bool operator==(const Version& lhs, const Version& rhs) noexcept;
    friend bool operator!=(const Version& lhs, const Version& rhs) noexcept { return !(lhs == rhs); }
    friend bool operator<(const Version& lhs, const Version& rhs) noexcept;
    friend bool operator>(const Version& lhs, const Version& rhs) noexcept { return rhs < lhs; }
    friend bool operator<=(const Version& lhs, const Version& rhs) noexcept { return !(rhs < lhs); }
    friend bool operator>=(const Version& lhs, const Version& rhs) noexcept { return !(lhs < rhs); }

private:
    QVector<quint64> components_;
    bool valid_ = false;
};

} // namespace genesys::launcher
