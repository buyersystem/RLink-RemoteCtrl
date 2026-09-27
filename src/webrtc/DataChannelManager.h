// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "api/data_channel_interface.h"
#include "api/peer_connection_interface.h"
#include "api/scoped_refptr.h"
#include "src/webrtc/IWebRtcSession.h"

namespace remote {

struct DataChannelCreateError {
    std::string code;
    std::string message;
};

class DataChannelManager final {
public:
    using StateCallback = std::function<void(const DataChannelInfo&)>;
    using MessageCallback = std::function<void(
        const std::string&, std::span<const std::uint8_t>, bool)>;

    DataChannelManager(StateCallback stateCallback,
                       MessageCallback messageCallback);
    ~DataChannelManager();

    DataChannelManager(const DataChannelManager&) = delete;
    DataChannelManager& operator=(const DataChannelManager&) = delete;

    std::optional<DataChannelCreateError> Create(
        webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
        const std::vector<DataChannelSpec>& channels);
    bool Attach(
        webrtc::scoped_refptr<webrtc::DataChannelInterface> channel);
    SendResult Send(const std::string& channelName,
                    std::span<const std::uint8_t> data,
                    bool binary) const;
    std::optional<std::uint64_t> BufferedAmount(
        const std::string& channelName) const;
    void Close();

private:
    class Binding;

    bool Contains(const std::string& label) const;
    void PublishState(
        webrtc::scoped_refptr<webrtc::DataChannelInterface> channel);
    void PublishMessage(const std::string& label,
                        const webrtc::DataBuffer& buffer);

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<Binding>> channels_;
    StateCallback stateCallback_;
    MessageCallback messageCallback_;
};

}  // namespace remote
