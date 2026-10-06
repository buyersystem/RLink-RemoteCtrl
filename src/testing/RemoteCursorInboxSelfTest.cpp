// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>

#include "src/apps/controller/RemoteCursorInbox.h"

namespace {
int failures = 0;
int checks = 0;
void Check(bool passed, const char* description)
{
    ++checks;
    if (!passed) {
        ++failures;
        std::cerr << "FAIL: " << description << '\n';
    }
}

remote::RemoteCursorEnvelope Position(std::uint64_t sequence)
{
    remote::RemoteCursorEnvelope result;
    result.contextId = "session";
    result.senderDeviceId = "peer";
    result.screenShareGeneration = 3;
    result.sequence = sequence;
    result.position.normalizedX = static_cast<std::uint16_t>(sequence);
    result.position.displayId = 1;
    result.position.displayLayoutVersion = 2;
    result.position.shapeId = 10;
    result.position.visible = true;
    return result;
}
}

int main()
{
    using remote::RemoteCursorMessageType;
    using remote::controller::RemoteCursorInbox;
    using remote::controller::RemoteCursorMatchesContext;

    RemoteCursorInbox blockedGui;
    int wakes = 0;
    for (std::uint64_t i = 1; i <= 10'000; ++i) {
        if (blockedGui.Push({}, Position(i))) ++wakes;
    }
    Check(wakes == 1, "blocked GUI gets one wake for 10000 positions");
    auto batch = blockedGui.Take();
    Check(batch.size() == 1 && batch.front().envelope.sequence == 10'000,
          "blocked GUI receives the latest position only");
    Check(blockedGui.Push({}, Position(10'001)),
          "producer after drain claims a new wake");
    Check(!blockedGui.Push({}, Position(9'999)),
          "older arrival does not schedule a second wake");
    batch = blockedGui.Take();
    Check(batch.size() == 1 && batch.front().envelope.sequence == 10'001,
          "older arrival never overwrites latest position");

    RemoteCursorInbox barriers;
    auto shape = Position(3);
    shape.type = RemoteCursorMessageType::kShape;
    shape.shape.shapeId = 10;
    auto reset = Position(6);
    reset.type = RemoteCursorMessageType::kReset;
    (void)barriers.Push({}, Position(1));
    (void)barriers.Push({}, Position(2));
    (void)barriers.Push({}, shape);
    (void)barriers.Push({}, Position(4));
    (void)barriers.Push({}, Position(5));
    (void)barriers.Push({}, reset);
    (void)barriers.Push({}, Position(7));
    batch = barriers.Take();
    Check(batch.size() == 5, "shape/reset split replaceable position runs");
    if (batch.size() == 5) {
        Check(batch[0].envelope.sequence == 2 &&
              batch[1].envelope.type == RemoteCursorMessageType::kShape &&
              batch[2].envelope.sequence == 5 &&
              batch[3].envelope.type == RemoteCursorMessageType::kReset &&
              batch[4].envelope.sequence == 7,
              "position/shape/position/reset/position order is retained");
    }
    auto other = Position(9);
    other.screenShareGeneration = 4;
    (void)barriers.Push({}, Position(8));
    (void)barriers.Push({}, other);
    other.sequence = 10;
    other.contextId = "new-session";
    (void)barriers.Push({}, other);
    other.sequence = 11;
    other.position.displayLayoutVersion = 4;
    (void)barriers.Push({}, other);
    other.sequence = 12;
    other.position.shapeId = 11;
    (void)barriers.Push({}, other);
    other.sequence = 13;
    other.position.displayId = 2;
    (void)barriers.Push({}, other);
    other.sequence = 14;
    (void)barriers.Push("other-pair", other);
    Check(barriers.Take().size() == 7,
          "context/share/display/layout/shape/pair changes are barriers");

    RemoteCursorInbox oldBinding;
    (void)oldBinding.Push({}, Position(1));
    oldBinding.Deactivate();
    Check(oldBinding.Take().empty(), "retiring binding drops its queued points");
    Check(!oldBinding.Push({}, Position(2)),
          "in-flight old callback cannot schedule after binding retirement");

    auto packet = Position(12);
    Check(RemoteCursorMatchesContext({}, packet, {}, "peer", "session", 3),
          "current direct context is accepted");
    Check(!RemoteCursorMatchesContext({}, packet, {}, "peer", "reconnect", 3),
          "previous session context is rejected at delivery");
    Check(!RemoteCursorMatchesContext({}, packet, {}, "peer", "session", 4),
          "previous share generation is rejected at delivery");
    Check(!RemoteCursorMatchesContext("pair", packet, {}, "peer", "session", 3),
          "wrong pair cannot enter direct binding");
    Check(!RemoteCursorMatchesContext({}, packet, {}, "other", "session", 3),
          "wrong sender cannot enter binding");
    Check(RemoteCursorMatchesContext({}, packet, {}, "peer", "session", 0, true),
          "direct bootstrap retains engine unknown-generation behavior");
    Check(!RemoteCursorMatchesContext("pair", packet, "pair", "peer", "session", 0),
          "room without current share generation is rejected");
    packet.screenShareGeneration = 0;
    Check(!RemoteCursorMatchesContext({}, packet, {}, "peer", "session", 0, true),
          "zero-generation packet is always rejected");
    packet = Position(13);
    remote::controller::RemoteCursorContext contextCache{
        {}, "peer", "session", 3, false};
    Check(contextCache.Matches({}, packet), "cached current context is accepted");
    contextCache.generation = 4;
    Check(!contextCache.Matches({}, packet),
          "cache refresh rejects previously queued share generation");
    contextCache.generation = 3;
    contextCache.contextId = "reconnected-session";
    Check(!contextCache.Matches({}, packet),
          "cache refresh rejects previously queued transport context");
    contextCache = {};
    Check(!contextCache.Matches({}, packet),
          "cleared binding cache rejects all old deliveries");

    // Exercise the same claim/drain handoff as the Qt adapter with a producer
    // publishing while a GUI-like consumer handles earlier batches.
    RemoteCursorInbox concurrent;
    std::mutex wakeMutex;
    std::condition_variable wakeCondition;
    int pendingWakes = 0;
    bool done = false;
    std::uint64_t latest = 0;
    bool ordered = true;
    std::thread producer([&] {
        for (std::uint64_t i = 1; i <= 40'000; ++i) {
            if (concurrent.Push({}, Position(i))) {
                {
                    std::lock_guard lock(wakeMutex);
                    ++pendingWakes;
                }
                wakeCondition.notify_one();
            }
            if ((i % 37) == 0) std::this_thread::yield();
        }
        {
            std::lock_guard lock(wakeMutex);
            done = true;
        }
        wakeCondition.notify_one();
    });
    while (true) {
        {
            std::unique_lock lock(wakeMutex);
            wakeCondition.wait(lock, [&] { return pendingWakes > 0 || done; });
            if (pendingWakes == 0 && done) break;
            --pendingWakes;
        }
        for (const auto& message : concurrent.Take()) {
            ordered = ordered && message.envelope.sequence > latest;
            latest = message.envelope.sequence;
        }
        std::this_thread::yield();
    }
    producer.join();
    Check(ordered, "concurrent producer/drain never delivers stale order");
    Check(latest == 40'000, "concurrent wake handoff does not lose final update");
    Check(concurrent.Take().empty(), "concurrent handoff leaves no unwoken points");
    std::cout << "RemoteCursorInboxSelfTest: " << checks << " checks, "
              << failures << " failures\n";
    return failures ? 1 : 0;
}
