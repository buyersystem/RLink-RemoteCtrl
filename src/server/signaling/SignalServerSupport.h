// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

#include <QByteArray>
#include <QString>

#include "SlidingWindowRateLimiter.h"

namespace remote::signaling_server::detail {

inline constexpr int kProtocolVersion = 5;
inline constexpr quint64 kMaximumMessageBytes = 1024 * 1024;
inline constexpr int kMaximumRememberedMessageIds = 4096;
inline constexpr int kMaximumSessions = 4096;
inline constexpr int kMaximumRooms = 4096;
inline constexpr qsizetype kInitialStateReserve = 2048;
inline constexpr int kMaximumPermissions = 16;
inline constexpr int kProtocolMaximumRoomMembers = 5;
inline constexpr int kRoomJoinRequestTimeoutMs = 30000;
inline constexpr int kRoomControlRequestTimeoutMs = 30000;
inline constexpr int kRoomScreenShareSwitchRequestTimeoutMs = 30000;
inline constexpr qint64 kRateLimitWindowMs = 60000;
inline constexpr int kRateLimitMaximumKeys = 4096;
inline constexpr SlidingWindowRateLimitPolicy kAvailabilityIpRateLimit{
    120, kRateLimitWindowMs, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kAvailabilityUserRateLimit{
    80, kRateLimitWindowMs, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kAvailabilityDeviceRateLimit{
    60, kRateLimitWindowMs, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kRoomJoinIpRateLimit{
    20, kRateLimitWindowMs, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kRoomJoinUserRateLimit{
    12, kRateLimitWindowMs, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kRoomJoinDeviceRateLimit{
    8, kRateLimitWindowMs, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kAccountDeletionUserRateLimit{
    3, 60 * 60 * 1000, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kAssistanceIpRateLimit{
    30, kRateLimitWindowMs, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kAssistanceRequesterRateLimit{
    8, kRateLimitWindowMs, kRateLimitMaximumKeys};
inline constexpr SlidingWindowRateLimitPolicy kAssistanceTargetRateLimit{
    20, kRateLimitWindowMs, kRateLimitMaximumKeys};

bool ReadFile(const QString& path,
              QByteArray* contents,
              QString* error);
QString CreateRecoveryToken();
bool StdoutIsTerminal();
void ClearStdoutTerminal();
QString FormatBytes(qint64 bytes);
QString FormatBitRate(double bitsPerSecond);
QString FormatDuration(qint64 milliseconds);
void SetError(QString* error, const QString& message);
bool IsValidPurpose(const QString& purpose);
bool IsNineDigitPublicId(const QString& value);
QString GenerateNineDigitPublicId();

}  // namespace remote::signaling_server::detail
