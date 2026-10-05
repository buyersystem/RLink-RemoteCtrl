// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

// UI sequencing only: no session engine, real WebRTC, network or account.
#include "src/apps/controller/StreamPreferenceRequestState.h"

#include <QCoreApplication>
#include <QEvent>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QTextStream>

#include <functional>

namespace {
using remote::ScreenStreamPreferenceRequest;
using remote::controller::StreamPreferenceRequestState;
using Completion = StreamPreferenceRequestState::Completion;

int failures = 0;
void Check(bool condition, const char* message)
{
    QTextStream(condition ? stdout : stderr)
        << (condition ? "PASS " : "FAIL ") << message << '\n';
    if (!condition) ++failures;
}

ScreenStreamPreferenceRequest Preference(std::uint32_t fps)
{
    ScreenStreamPreferenceRequest preference;
    preference.framesPerSecond = fps;
    return preference;
}

void TestSequencing()
{
    StreamPreferenceRequestState state;
    state.Reset(Preference(30));
    const auto generation = state.Generation();
    const auto first = state.Begin();
    const auto second = state.Begin();
    Check(state.HasPending(), "successive menu choices remain pending until real completion");
    Check(state.Complete(generation, first, Preference(60), true) == Completion::kStale &&
          state.HasPending() && state.LastSuccessful().framesPerSecond == 60,
          "older successful send updates rollback baseline without completing the newer choice");
    Check(state.Complete(generation, second, Preference(120), false) == Completion::kFailed &&
          !state.HasPending() && state.LastSuccessful().framesPerSecond == 60,
          "latest failed send rolls back to the most recent real successful send");
    Check(state.Complete(generation, second, Preference(120), false) == Completion::kStale,
          "duplicate completion cannot repeat UI rollback or error notification");

    state.Reset(Preference(30));
    const auto newGeneration = state.Generation();
    const auto third = state.Begin();
    const auto fourth = state.Begin();
    Check(state.Complete(newGeneration, third, Preference(60), false) == Completion::kStale &&
          state.HasPending() && state.LastSuccessful().framesPerSecond == 30,
          "older failed choice cannot override the newer pending choice");
    Check(state.Complete(newGeneration, fourth, Preference(120), true) == Completion::kSucceeded &&
          !state.HasPending() && state.LastSuccessful().framesPerSecond == 120,
          "latest successful choice becomes the rollback baseline");
    Check(state.Complete(newGeneration, third, Preference(60), true) == Completion::kStale &&
          state.LastSuccessful().framesPerSecond == 120,
          "late older success cannot replace a newer successful baseline");

    state.Reset(Preference(24));
    Check(state.Complete(newGeneration, fourth, Preference(120), true) == Completion::kStale &&
          !state.HasPending() && state.LastSuccessful().framesPerSecond == 24,
          "binding/reset generation rejects old completion without changing new session defaults");
    const auto fifth = state.Begin();
    state.UpdateAcknowledged(Preference(90));
    Check(state.LastSuccessful().framesPerSecond == 24,
          "acknowledgement does not replace rollback baseline while a local choice is pending");
    Check(state.Complete(state.Generation(), fifth, Preference(60), true) == Completion::kSucceeded,
          "new binding accepts its own completion");
    state.UpdateAcknowledged(Preference(90));
    Check(state.LastSuccessful().framesPerSecond == 90,
          "idle acknowledgement still updates the real remote preference baseline");
}

void TestQueuedUiLifetime()
{
    StreamPreferenceRequestState state;
    state.Reset(Preference(30));
    const auto generation = state.Generation();
    const auto sequence = state.Begin();
    int uiUpdates = 0;
    QObject owner;
    QMetaObject::invokeMethod(&owner, [&] {
        if (state.Complete(generation, sequence, Preference(60), true) == Completion::kSucceeded) {
            ++uiUpdates;
        }
    }, Qt::QueuedConnection);
    Check(uiUpdates == 0 && state.HasPending(), "transport completion is marshaled, not applied inline");
    QCoreApplication::sendPostedEvents(&owner, QEvent::MetaCall);
    Check(uiUpdates == 1 && !state.HasPending(), "live UI owner applies marshaled completion once");

    const auto oldSequence = state.Begin();
    const auto oldGeneration = state.Generation();
    QMetaObject::invokeMethod(&owner, [&] {
        if (state.Complete(oldGeneration, oldSequence, Preference(120), true) != Completion::kStale) {
            ++uiUpdates;
        }
    }, Qt::QueuedConnection);
    state.Reset(Preference(24));
    QCoreApplication::sendPostedEvents(&owner, QEvent::MetaCall);
    Check(uiUpdates == 1 && state.LastSuccessful().framesPerSecond == 24,
          "queued old-binding UI completion is ignored after reset");

    auto* deletedOwner = new QObject;
    QPointer<QObject> guardedOwner(deletedOwner);
    QMetaObject::invokeMethod(deletedOwner, [&] { ++uiUpdates; }, Qt::QueuedConnection);
    delete deletedOwner;
    QCoreApplication::sendPostedEvents(nullptr, QEvent::MetaCall);
    Check(guardedOwner.isNull() && uiUpdates == 1,
          "destroyed UI context drops queued completion without touching freed state");
}

void TestAcknowledgementGate()
{
    StreamPreferenceRequestState state;
    state.Reset(Preference(30));
    const auto generation = state.Generation();
    const auto first = state.Begin();
    state.Complete(generation, first, Preference(60), true);
    state.MarkSentTransportSequence(10);
    Check(!state.CanApplyAcknowledged(9, 10, false),
          "successful send ignores an older FPS snapshot until its own ACK");
    const auto second = state.Begin();
    Check(!state.CanApplyAcknowledged(10, 10, false),
          "even valid earlier ACK cannot replace a newer local pending selection");
    state.Complete(generation, second, Preference(120), false);
    Check(!state.CanApplyAcknowledged(9, 11, false) &&
          state.LastSuccessful().framesPerSecond == 60,
          "new send failure retains earlier successful preference and ACK latch");
    Check(state.CanApplyAcknowledged(10, 11, false),
          "real earlier successful ACK releases latch after the newer send fails");
    const auto third = state.Begin();
    state.Complete(generation, third, Preference(120), true);
    state.MarkSentTransportSequence(12);
    Check(!state.CanApplyAcknowledged(10, 11, true),
          "older rejected ACK cannot release latest sent preference latch");
    Check(state.CanApplyAcknowledged(10, 12, true),
          "current rejected ACK still permits original actual-FPS fallback");
    state.MarkSentTransportSequence(13);
    state.Reset(Preference(24));
    Check(state.CanApplyAcknowledged(0, 0, false),
          "new binding or capture generation clears obsolete ACK latch");
}
}  // namespace

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    TestSequencing();
    TestQueuedUiLifetime();
    TestAcknowledgementGate();
    QTextStream(stdout) << "RESULT stream_preference_request_state " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
