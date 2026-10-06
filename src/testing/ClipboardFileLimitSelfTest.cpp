#include "src/apps/remote/ClipboardFileLimit.h"

#include <iostream>
#include <limits>

int main()
{
    using namespace remote::app;
    constexpr std::uint64_t mib = 1024 * 1024;
    bool passed = true;
    const auto check = [&](bool value, const char* name) {
        std::cout << name << '=' << (value ? "PASS" : "FAIL") << '\n';
        passed &= value;
    };
    check(!ClipboardFilesExceedLimit(0, 100 * mib, 100 * mib), "EXACT_LIMIT_ALLOWED");
    check(ClipboardFilesExceedLimit(0, 100 * mib + 1, 100 * mib), "SINGLE_FILE_OVER_LIMIT");
    check(ClipboardFilesExceedLimit(60 * mib, 41 * mib, 100 * mib), "AGGREGATE_FILES_OVER_LIMIT");
    check(!ClipboardFilesExceedLimit(60 * mib, 40 * mib, 100 * mib), "AGGREGATE_EXACT_LIMIT_ALLOWED");
    check(ClipboardFilesExceedLimit(0, 30 * mib, 25 * mib) &&
          !ClipboardFilesExceedLimit(0, 30 * mib, 100 * mib), "DIFFERENT_PEER_LIMITS");
    const auto maximum = (std::numeric_limits<std::uint64_t>::max)();
    check(ClipboardFilesExceedLimit(maximum, 1, maximum), "OVERFLOW_REJECTED");
    check(!ClipboardFilesExceedLimit(0, 0, 0), "EMPTY_FILE_ALLOWED");
    check(ShouldShowClipboardFailure("clipboard_file_limit_exceeded", true),
          "CONTROLLER_SHOWS_LIMIT_FAILURE_IN_BOTH_DIRECTIONS");
    check(!ShouldShowClipboardFailure("clipboard_file_limit_exceeded", false),
          "CONTROLLED_ENDPOINT_SUPPRESSES_DUPLICATE_LIMIT_FAILURE");
    check(ShouldShowClipboardFailure("clipboard_apply_failed", false) &&
          ShouldShowClipboardFailure("clipboard_apply_failed", true),
          "OTHER_FAILURE_PROMPTS_UNCHANGED");
    const auto sender = ClipboardFileLimitMessage(100 * mib, false);
    const auto receiver = ClipboardFileLimitMessage(25 * mib, true);
    check(sender == "文件总大小超过发送端上限（100 MB），本次粘贴已取消。\n大文件可以用文件传输功能哦。",
          "SENDER_CHINESE_LIMIT_MESSAGE");
    check(receiver == "文件总大小超过接收端上限（25 MB），本次粘贴已取消。\n大文件可以用文件传输功能哦。",
          "RECEIVER_CHINESE_LIMIT_MESSAGE");
    check(sender.find("字节") == std::string::npos &&
          receiver.find("字节") == std::string::npos,
          "NO_BYTE_COUNTS_OR_LONG_ADVICE");
    return passed ? 0 : 1;
}
