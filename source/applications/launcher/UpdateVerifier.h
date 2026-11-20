#pragma once

#include <QString>

namespace genesys::launcher {

class LauncherLogger;

struct VerificationResult {
    bool ok = false;
    QString error;
};

class ISignatureVerifier {
public:
    virtual ~ISignatureVerifier() = default;

    [[nodiscard]] virtual VerificationResult verify(const QString& dataPath,
                                                    const QString& signaturePath) = 0;
};

class GpgvSignatureVerifier final : public ISignatureVerifier {
public:
    explicit GpgvSignatureVerifier(QString keyringPath,
                                   LauncherLogger* logger = nullptr,
                                   int timeoutMs = 15000);

    [[nodiscard]] VerificationResult verify(const QString& dataPath,
                                            const QString& signaturePath) override;

private:
    QString keyringPath_;
    LauncherLogger* logger_ = nullptr;
    int timeoutMs_ = 15000;
};

class UpdateVerifier {
public:
    [[nodiscard]] static VerificationResult verifySha256(const QString& filePath,
                                                         const QString& expectedHexDigest);
};

} // namespace genesys::launcher
