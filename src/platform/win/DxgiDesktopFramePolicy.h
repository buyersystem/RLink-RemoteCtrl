#pragma once

#include <cstdint>

namespace remote {

// Pointer-only updates can return an unreliable desktop resource. Only read
// that resource when DXGI reports an actual desktop image update.
constexpr bool HasDxgiDesktopImageUpdate(std::int64_t lastPresentTime,
                                        std::uint32_t accumulatedFrames)
{
    return lastPresentTime != 0 && accumulatedFrames != 0;
}

}  // namespace remote
