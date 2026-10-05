// SPDX-License-Identifier: GPL-3.0-only
#include "src/apps/remote/InProcessSessionEngineInternal.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/ScreenShareControlProtocol.h"

#include <iostream>
#include <stdexcept>
#include <vector>

namespace remote::testing {
class InProcessSessionEngineTestAccess {
public:
    static void Run()
    {
        app::InProcessSessionEngine engine;
        auto& snapshot = engine.snapshot_;
        snapshot.localDeviceId = "viewer";
        snapshot.sessionId = "direct";
        snapshot.peerDeviceId = "sharer";
        snapshot.remoteControlRole = RemoteControlRole::kController;
        snapshot.direct.remoteScreenShareGeneration = 4;

        const auto directAck = [&](std::uint64_t sequence, bool accepted,
                                   std::uint64_t generation = 4) {
            auto applied = Ack("direct", sequence, generation, accepted);
            std::vector<std::uint8_t> bytes;
            Check(EncodeScreenStreamPreferenceAppliedV2(applied, &bytes), "encode direct ACK");
            Check(engine.DispatchDirectScreenData(std::string(kControlReliableChannel), bytes),
                "production direct ACK dispatcher consumed payload");
        };
        snapshot.direct.screenPreferencePending = true;
        snapshot.direct.screenPreferenceSequence = 5;
        directAck(5, true);
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 5,
            "direct successful ACK commits accepted sequence");
        Check(!snapshot.direct.screenPreferencePending, "direct success clears pending");
        snapshot.direct.screenPreferencePending = true;
        snapshot.direct.screenPreferenceSequence = 6;
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 5,
            "direct pending request retains accepted contract");
        directAck(6, false);
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 5,
            "direct rejected ACK retains accepted contract");
        Check(!snapshot.direct.screenPreferencePending, "direct rejection clears pending");

        ScreenStreamPreferenceRequest invalid;
        invalid.framesPerSecond = 0;
        Check(!engine.SetDirectScreenStreamPreference(invalid).accepted, "direct invalid request fails");
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 5,
            "direct failed validation retains accepted contract");
        ScreenStreamPreferenceRequest valid;
        valid.framesPerSecond = 60;
        Check(!engine.SetDirectScreenStreamPreference(valid).accepted,
            "direct request without active controller fails");
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 5,
            "direct unavailable request retains accepted contract");
        snapshot.direct.screenPreferencePending = true;
        snapshot.direct.screenPreferenceSequence = 7;
        directAck(6, true);
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 5 && snapshot.direct.screenPreferencePending,
            "direct obsolete ACK cannot commit or clear newer pending request");
        directAck(7, true, 3);
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 5,
            "direct old generation ACK cannot commit");
        directAck(7, true);
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 7, "direct recovery commits new accepted request");
        // A display switch preserves the sender's already applied preference.
        snapshot.direct.remoteScreenShareGeneration = 8;
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 7,
            "direct display generation preserves applied preference identity");
        engine.ResetSessionStateLocked();
        Check(snapshot.direct.screenPreferenceAcceptedSequence == 0,
            "direct session reset clears accepted preference identity");

        constexpr auto pairId = "room-pair";
        auto pair = std::make_shared<app::InProcessSessionEngine::RoomPairRuntime>();
        pair->pairId = pairId;
        pair->roomId = "room";
        pair->peerDeviceId = "sharer";
        engine.roomPairs_.emplace(pairId, pair);
        snapshot.localDeviceId = "viewer";
        snapshot.room.roomId = "room";
        snapshot.room.membership = RoomMembershipState::kActive;
        snapshot.room.screenShareState = RoomScreenShareState::kActive;
        snapshot.room.screenSharerDeviceId = "sharer";
        snapshot.room.screenShareEpoch = 10;
        snapshot.room.members.push_back({.deviceId = "viewer", .online = true});
        snapshot.room.members.push_back({.deviceId = "sharer", .online = true});
        snapshot.roomActivity.peerConnections.push_back({.pairId = pairId, .peerDeviceId = "sharer"});
        const auto roomAck = [&](std::uint64_t sequence, std::uint64_t generation, bool accepted) {
            auto applied = Ack("room", sequence, generation, accepted);
            std::vector<std::uint8_t> bytes;
            Check(EncodeScreenStreamPreferenceAppliedV2(applied, &bytes), "encode room ACK");
            Check(engine.DispatchRoomPairScreenData(pairId, bytes),
                "production room ACK dispatcher consumed payload");
        };
        auto& peer = snapshot.roomActivity.peerConnections.front();
        peer.screenPreferencePending = true;
        peer.screenPreferenceSequence = 20;
        peer.screenPreferenceGeneration = 10;
        roomAck(20, 10, true);
        Check(peer.screenPreferenceAcceptedSequence == 20 && peer.screenPreferenceAcceptedGeneration == 10,
            "room success commits generation-bound accepted preference");
        peer.screenPreferencePending = true;
        peer.screenPreferenceSequence = 21;
        Check(peer.screenPreferenceAcceptedSequence == 20, "room pending request retains accepted contract");
        roomAck(21, 10, false);
        Check(peer.screenPreferenceAcceptedSequence == 20 && peer.screenPreferenceAcceptedGeneration == 10,
            "room rejected ACK retains accepted contract");
        Check(!engine.SetRoomScreenStreamPreference(pairId, invalid).accepted, "room invalid request fails");
        Check(peer.screenPreferenceAcceptedSequence == 20, "room failed validation retains accepted contract");
        Check(!engine.SetRoomScreenStreamPreference(pairId, valid).accepted,
            "room unavailable controller request fails");
        Check(peer.screenPreferenceAcceptedSequence == 20, "room unavailable request retains accepted contract");

        auto nextRoom = snapshot.room;
        nextRoom.screenShareEpoch = 11;
        engine.OnRoomState(nextRoom);
        Check(peer.screenPreferenceAcceptedSequence == 0 && peer.screenPreferenceAcceptedGeneration == 0,
            "production room epoch reset clears accepted preference contract");
        peer.screenPreferencePending = true;
        peer.screenPreferenceSequence = 22;
        peer.screenPreferenceGeneration = 11;
        roomAck(22, 10, true);
        Check(peer.screenPreferenceAcceptedSequence == 0 && peer.screenPreferencePending,
            "old room epoch ACK cannot commit or clear pending request");
        roomAck(22, 11, true);
        Check(peer.screenPreferenceAcceptedSequence == 22 && peer.screenPreferenceAcceptedGeneration == 11,
            "room new epoch successful ACK restores accepted contract");
    }

    static unsigned Checks() { return checks_; }
private:
    static ScreenStreamPreferenceApplied Ack(const char* room, std::uint64_t sequence,
        std::uint64_t generation, bool accepted)
    {
        ScreenStreamPreferenceApplied result;
        result.roomId = room;
        result.senderDeviceId = "sharer";
        result.requestSequence = sequence;
        result.screenShareGeneration = generation;
        result.accepted = accepted;
        result.width = 1920;
        result.height = 1080;
        result.framesPerSecond = 60;
        result.maxBitrateBps = 20'000'000;
        result.scaleBackend = ScreenScaleBackend::kWebRtc;
        if (!accepted) result.error = "test rejection";
        return result;
    }
    static void Check(bool condition, const char* description)
    {
        ++checks_;
        if (!condition) throw std::runtime_error(description);
    }
    inline static unsigned checks_ = 0;
};
}  // namespace remote::testing

int main()
{
    try {
        remote::testing::InProcessSessionEngineTestAccess::Run();
        std::cout << "PASS " << remote::testing::InProcessSessionEngineTestAccess::Checks()
                  << " preference feedback contract checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
