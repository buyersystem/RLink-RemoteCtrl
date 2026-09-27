// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SignalServerSupport.h"

#include <algorithm>
#include <array>
#include <cstdio>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <QFile>
#include <QRandomGenerator>
#include <QTextStream>

namespace remote::signaling_server::detail {

void SetError(QString* error, const QString& message)
{
    if (error) {
        *error = message;
    }
}

bool ReadFile(const QString& path,
              QByteArray* contents,
              QString* error)
{
    QFile file(path);
    if (!contents || !file.open(QIODevice::ReadOnly)) {
        SetError(error, QStringLiteral("Cannot open %1: %2")
                            .arg(path, file.errorString()));
        return false;
    }
    *contents = file.readAll();
    return true;
}

QString CreateRecoveryToken()
{
    std::array<quint32, 8> randomWords{};
    QRandomGenerator::system()->fillRange(
        randomWords.data(), randomWords.size());
    const QByteArray bytes(
        reinterpret_cast<const char*>(randomWords.data()),
        static_cast<qsizetype>(sizeof(randomWords)));
    return QString::fromLatin1(bytes.toBase64(
        QByteArray::Base64UrlEncoding |
        QByteArray::OmitTrailingEquals));
}

bool StdoutIsTerminal()
{
#ifdef _WIN32
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(fileno(stdout)) != 0;
#endif
}

void ClearStdoutTerminal()
{
#ifdef _WIN32
    const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (output == INVALID_HANDLE_VALUE ||
        !GetConsoleScreenBufferInfo(output, &info)) {
        return;
    }
    const DWORD cells = static_cast<DWORD>(info.dwSize.X) *
                        static_cast<DWORD>(info.dwSize.Y);
    const COORD origin{0, 0};
    DWORD written = 0;
    FillConsoleOutputCharacterW(output, L' ', cells, origin, &written);
    FillConsoleOutputAttribute(output, info.wAttributes, cells, origin,
                               &written);
    SetConsoleCursorPosition(output, origin);
#else
    QTextStream(stdout) << "\x1b[2J\x1b[H";
#endif
}

QString FormatBytes(qint64 bytes)
{
    static constexpr std::array<const char*, 4> units{
        "B", "KiB", "MiB", "GiB"};
    double value = static_cast<double>(std::max<qint64>(0, bytes));
    std::size_t unit = 0;
    while (value >= 1024.0 && unit + 1 < units.size()) {
        value /= 1024.0;
        ++unit;
    }
    const int precision = unit == 0 ? 0 : 2;
    return QStringLiteral("%1 %2")
        .arg(QString::number(value, 'f', precision),
             QString::fromLatin1(units[unit]));
}

QString FormatBitRate(double bitsPerSecond)
{
    static constexpr std::array<const char*, 4> units{
        "bps", "Kbps", "Mbps", "Gbps"};
    double value = std::max(0.0, bitsPerSecond);
    std::size_t unit = 0;
    while (value >= 1000.0 && unit + 1 < units.size()) {
        value /= 1000.0;
        ++unit;
    }
    const int precision = unit == 0 ? 0 : 2;
    return QStringLiteral("%1 %2")
        .arg(QString::number(value, 'f', precision),
             QString::fromLatin1(units[unit]));
}

QString FormatDuration(qint64 milliseconds)
{
    const qint64 totalSeconds = std::max<qint64>(0, milliseconds / 1000);
    const qint64 days = totalSeconds / 86400;
    const qint64 hours = (totalSeconds % 86400) / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;
    if (days > 0) {
        return QStringLiteral("%1天 %2小时 %3分")
            .arg(days).arg(hours).arg(minutes);
    }
    if (hours > 0) {
        return QStringLiteral("%1小时 %2分 %3秒")
            .arg(hours).arg(minutes).arg(seconds);
    }
    return QStringLiteral("%1分 %2秒").arg(minutes).arg(seconds);
}

bool IsValidPurpose(const QString& purpose)
{
    return purpose == QStringLiteral("remote_control") ||
           purpose == QStringLiteral("camera_only");
}

bool IsNineDigitPublicId(const QString& value)
{
    return value.size() == 9 && value.front() != QChar('0') &&
           std::all_of(value.cbegin(), value.cend(),
                       [](QChar character) { return character.isDigit(); });
}

QString GenerateNineDigitPublicId()
{
    constexpr quint32 kFirstPublicId = 100000000;
    constexpr quint32 kPublicIdCount = 900000000;
    return QString::number(
        kFirstPublicId +
        QRandomGenerator::system()->bounded(kPublicIdCount));
}

}  // namespace remote::signaling_server::detail
