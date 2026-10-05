// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/platform/win/DpapiContentAnalyzerSecretStore.h"

#include <Windows.h>
#include <wincrypt.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>
#include <limits>
#include <utility>

namespace remote {
namespace {

constexpr char kEnvelopeMagic[] = "RLVISION";
constexpr char kEnvelopeVersion = 1;
constexpr char kEntropy[] = "RLink.ContentAnalysis.ApiCredential.v1";
constexpr qsizetype kMaximumSecretBytes = 64 * 1024;

void SecureClear(QByteArray* bytes)
{
    if (bytes && !bytes->isEmpty()) {
        SecureZeroMemory(bytes->data(), static_cast<SIZE_T>(bytes->size()));
        bytes->clear();
    }
}

void SetError(std::string* error, const QString& message)
{
    if (!error) {
        return;
    }
    const QByteArray utf8 = message.toUtf8();
    error->assign(utf8.constData(), static_cast<std::size_t>(utf8.size()));
}

QString WindowsError(const QString& operation)
{
    return QStringLiteral("%1 failed with Windows error %2")
        .arg(operation)
        .arg(GetLastError());
}

bool IsValidCredentialId(std::string_view credentialId)
{
    return !credentialId.empty() && credentialId.size() <= 128 &&
        std::all_of(
            credentialId.begin(),
            credentialId.end(),
            [](const unsigned char ch) {
                return (ch >= 'a' && ch <= 'z') ||
                    (ch >= 'A' && ch <= 'Z') ||
                    (ch >= '0' && ch <= '9') || ch == '-' || ch == '_';
            });
}

bool ToBlob(QByteArray* bytes, DATA_BLOB* blob)
{
    if (!bytes || !blob || bytes->size() < 0 ||
        static_cast<quint64>(bytes->size()) >
            std::numeric_limits<DWORD>::max()) {
        return false;
    }
    blob->cbData = static_cast<DWORD>(bytes->size());
    blob->pbData = reinterpret_cast<BYTE*>(bytes->data());
    return true;
}

QByteArray Protect(QByteArray* plaintext, std::string* error)
{
    QByteArray entropy(kEntropy, sizeof(kEntropy) - 1);
    DATA_BLOB input{};
    DATA_BLOB entropyBlob{};
    DATA_BLOB output{};
    if (!ToBlob(plaintext, &input) || !ToBlob(&entropy, &entropyBlob)) {
        SecureClear(plaintext);
        SecureClear(&entropy);
        SetError(error, QStringLiteral("API credential is too large"));
        return {};
    }
    const BOOL succeeded = CryptProtectData(
        &input,
        L"RLink content analysis API credential",
        &entropyBlob,
        nullptr,
        nullptr,
        CRYPTPROTECT_UI_FORBIDDEN,
        &output);
    SecureClear(plaintext);
    SecureClear(&entropy);
    if (!succeeded) {
        SetError(error, WindowsError(QStringLiteral("CryptProtectData")));
        return {};
    }
    QByteArray encrypted(
        reinterpret_cast<const char*>(output.pbData),
        static_cast<qsizetype>(output.cbData));
    SecureZeroMemory(output.pbData, output.cbData);
    LocalFree(output.pbData);
    return encrypted;
}

QByteArray Unprotect(QByteArray* encrypted, std::string* error)
{
    QByteArray entropy(kEntropy, sizeof(kEntropy) - 1);
    DATA_BLOB input{};
    DATA_BLOB entropyBlob{};
    DATA_BLOB output{};
    if (!ToBlob(encrypted, &input) || !ToBlob(&entropy, &entropyBlob)) {
        SecureClear(&entropy);
        SetError(error, QStringLiteral("Encrypted API credential is invalid"));
        return {};
    }
    const BOOL succeeded = CryptUnprotectData(
        &input,
        nullptr,
        &entropyBlob,
        nullptr,
        nullptr,
        CRYPTPROTECT_UI_FORBIDDEN,
        &output);
    SecureClear(&entropy);
    if (!succeeded) {
        SetError(error, WindowsError(QStringLiteral("CryptUnprotectData")));
        return {};
    }
    QByteArray plaintext(
        reinterpret_cast<const char*>(output.pbData),
        static_cast<qsizetype>(output.cbData));
    SecureZeroMemory(output.pbData, output.cbData);
    LocalFree(output.pbData);
    return plaintext;
}

}  // namespace

DpapiContentAnalyzerSecretStore::DpapiContentAnalyzerSecretStore(
    QString directory)
    : directory_(directory.isEmpty() ? DefaultDirectory()
                                     : std::move(directory))
{
}

QString DpapiContentAnalyzerSecretStore::DefaultDirectory()
{
    return QDir(QStandardPaths::writableLocation(
                    QStandardPaths::AppLocalDataLocation))
        .filePath(QStringLiteral("content-analysis/credentials"));
}

QString DpapiContentAnalyzerSecretStore::CredentialPath(
    std::string_view credentialId) const
{
    if (!IsValidCredentialId(credentialId)) {
        return {};
    }
    return QDir(directory_).filePath(
        QString::fromLatin1(
            credentialId.data(),
            static_cast<qsizetype>(credentialId.size())) +
        QStringLiteral(".dat"));
}

bool DpapiContentAnalyzerSecretStore::LoadSecret(
    std::string_view credentialId,
    media_intelligence::SecretValue* secret,
    std::string* error)
{
    if (!secret) {
        SetError(error, QStringLiteral("Secret output is null"));
        return false;
    }
    const QString path = CredentialPath(credentialId);
    if (path.isEmpty()) {
        SetError(error, QStringLiteral("Credential ID is invalid"));
        return false;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        SetError(error, QStringLiteral("API credential is not configured"));
        return false;
    }
    if (file.size() <= 0 || file.size() > kMaximumSecretBytes * 2) {
        file.close();
        SetError(error, QStringLiteral("Encrypted API credential has an invalid size"));
        return false;
    }
    QByteArray envelope = file.readAll();
    file.close();
    const QByteArray prefix(kEnvelopeMagic, sizeof(kEnvelopeMagic) - 1);
    if (!envelope.startsWith(prefix) ||
        envelope.size() <= prefix.size() ||
        envelope.at(prefix.size()) != kEnvelopeVersion) {
        SecureClear(&envelope);
        SetError(error, QStringLiteral("Encrypted API credential format is unsupported"));
        return false;
    }
    QByteArray encrypted = envelope.mid(prefix.size() + 1);
    SecureClear(&envelope);
    QByteArray plaintext = Unprotect(&encrypted, error);
    SecureClear(&encrypted);
    if (plaintext.isEmpty() || plaintext.size() > kMaximumSecretBytes) {
        SecureClear(&plaintext);
        if (error && error->empty()) {
            SetError(error, QStringLiteral("Decrypted API credential is invalid"));
        }
        return false;
    }
    *secret = media_intelligence::SecretValue(std::string(
        plaintext.constData(),
        static_cast<std::size_t>(plaintext.size())));
    SecureClear(&plaintext);
    if (error) {
        error->clear();
    }
    return true;
}

bool DpapiContentAnalyzerSecretStore::SaveSecret(
    std::string_view credentialId,
    std::string_view secret,
    std::string* error)
{
    const QString path = CredentialPath(credentialId);
    if (path.isEmpty()) {
        SetError(error, QStringLiteral("Credential ID is invalid"));
        return false;
    }
    if (secret.empty() || secret.size() >
        static_cast<std::size_t>(kMaximumSecretBytes)) {
        SetError(error, QStringLiteral("API credential size is invalid"));
        return false;
    }

    QByteArray plaintext(
        secret.data(),
        static_cast<qsizetype>(secret.size()));
    QByteArray encrypted = Protect(&plaintext, error);
    if (encrypted.isEmpty()) {
        return false;
    }
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        SecureClear(&encrypted);
        SetError(error, QStringLiteral("Unable to create API credential directory"));
        return false;
    }
    QByteArray envelope(kEnvelopeMagic, sizeof(kEnvelopeMagic) - 1);
    envelope.append(kEnvelopeVersion);
    envelope.append(encrypted);
    SecureClear(&encrypted);

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) ||
        file.write(envelope) != envelope.size() || !file.commit()) {
        SecureClear(&envelope);
        SetError(error, QStringLiteral("Unable to atomically save API credential"));
        return false;
    }
    SecureClear(&envelope);
    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    if (error) {
        error->clear();
    }
    return true;
}

bool DpapiContentAnalyzerSecretStore::DeleteSecret(
    std::string_view credentialId,
    std::string* error)
{
    const QString path = CredentialPath(credentialId);
    if (path.isEmpty()) {
        SetError(error, QStringLiteral("Credential ID is invalid"));
        return false;
    }
    if (!QFile::exists(path) || QFile::remove(path)) {
        if (error) {
            error->clear();
        }
        return true;
    }
    SetError(error, QStringLiteral("Unable to remove API credential"));
    return false;
}

bool DpapiContentAnalyzerSecretStore::HasSecret(
    std::string_view credentialId) const
{
    const QString path = CredentialPath(credentialId);
    return !path.isEmpty() && QFileInfo::exists(path);
}

}  // namespace remote
