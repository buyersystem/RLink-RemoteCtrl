// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)
#pragma once

#include "DiagnosticsCardsWidget.h"
#include "src/core/SessionDiagnostics.h"
#include <vector>

namespace remote::controller::detail {
DiagnosticsSection MakeContentPolicySection(const QString& key,
    const QString& peerName, const RtpStreamStatsSnapshot& stream);
QVector<DiagnosticsSection> MakeContentPolicySections(const QString& peerKey,
    const QString& peerName, const std::vector<RtpStreamStatsSnapshot>& streams);
QString ContentPolicyCopyText(const QVector<DiagnosticsSection>& sections);
} // namespace remote::controller::detail
