#pragma once

#include <QByteArray>
#include <QString>

namespace genesys::launcher {

struct ExtractionResult {
    bool ok = false;
    QString error;
};

class IArchiveExtractor {
public:
    virtual ~IArchiveExtractor() = default;

    [[nodiscard]] virtual ExtractionResult extract(const QString& archivePath,
                                                   const QString& destinationRoot) = 0;
};

class TarZstdArchiveExtractor final : public IArchiveExtractor {
public:
    explicit TarZstdArchiveExtractor(int timeoutMs = 60000);

    [[nodiscard]] ExtractionResult extract(const QString& archivePath,
                                           const QString& destinationRoot) override;

    [[nodiscard]] static ExtractionResult validatePathListing(const QByteArray& listing);
    [[nodiscard]] static ExtractionResult validateVerboseListing(const QByteArray& listing);

private:
    [[nodiscard]] ExtractionResult inspect(const QString& archivePath) const;
    [[nodiscard]] static ExtractionResult validateExtractedTree(const QString& destinationRoot);

    int timeoutMs_ = 60000;
};

} // namespace genesys::launcher
