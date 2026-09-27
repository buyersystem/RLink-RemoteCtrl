// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

namespace remote {
struct SessionDiagnosticsSnapshot;
}

namespace remote::controller::detail {

void AppendScreenFrameRateLog(const SessionDiagnosticsSnapshot &diagnostics);

} // namespace remote::controller::detail
