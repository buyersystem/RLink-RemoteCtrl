#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace remote::app {

inline bool ShouldShowClipboardFailure(std::string_view code,
                                      bool localIsController)
{
    return code != "clipboard_file_limit_exceeded" || localIsController;
}

inline bool ClipboardFilesExceedLimit(std::uint64_t accumulated,
                                     std::uint64_t next,
                                     std::uint64_t limit)
{
    return next > limit || accumulated > limit - next;
}

inline std::string ClipboardFileLimitMessage(std::uint64_t limit,
                                            bool receiving)
{
    // Keep the existing binary-megabyte limit; MB is the UI shorthand.
    return "文件总大小超过" + std::string(receiving ? "接收端" : "发送端") +
        "上限（" + std::to_string(limit / (1024 * 1024)) +
        " MB），本次粘贴已取消。\n大文件可以用文件传输功能哦。";
}

}  // namespace remote::app
