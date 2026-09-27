// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DataChannelManager.h"

#include <algorithm>
#include <utility>

#include "rtc_base/copy_on_write_buffer.h"

namespace remote {
namespace {

DataChannelState ToPublicState(
    webrtc::DataChannelInterface::DataState state)
{
    switch (state) {
    case webrtc::DataChannelInterface::kConnecting:
        return DataChannelState::kConnecting;
    case webrtc::DataChannelInterface::kOpen:
        return DataChannelState::kOpen;
    case webrtc::DataChannelInterface::kClosing:
        return DataChannelState::kClosing;
    case webrtc::DataChannelInterface::kClosed:
        return DataChannelState::kClosed;
    }
    return DataChannelState::kClosed;
}

}  // namespace

class DataChannelManager::Binding final
    : public webrtc::DataChannelObserver {
public:
    Binding(DataChannelManager* owner,
            webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)
        : owner_(owner), channel_(std::move(channel))
    {
        channel_->RegisterObserver(this);
    }

    ~Binding() override { channel_->UnregisterObserver(); }

    webrtc::scoped_refptr<webrtc::DataChannelInterface> Channel() const
    {
        return channel_;
    }

    void Close() { channel_->Close(); }

    void OnStateChange() override
    {
        owner_->PublishState(channel_);
    }

    void OnMessage(const webrtc::DataBuffer& buffer) override
    {
        owner_->PublishMessage(channel_->label(), buffer);
    }

private:
    DataChannelManager* owner_;
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel_;
};

DataChannelManager::DataChannelManager(StateCallback stateCallback,
                                       MessageCallback messageCallback)
    : stateCallback_(std::move(stateCallback)),
      messageCallback_(std::move(messageCallback))
{}

DataChannelManager::~DataChannelManager()
{
    Close();
}

std::optional<DataChannelCreateError> DataChannelManager::Create(
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
    const std::vector<DataChannelSpec>& channels)
{
    for (const auto& spec : channels) {
        if (spec.label.empty()) {
            return DataChannelCreateError{
                "invalid_channel_label",
                "A DataChannel label cannot be empty."};
        }
        if (spec.maxRetransmits && spec.maxPacketLifeTimeMs) {
            return DataChannelCreateError{
                "invalid_channel_reliability",
                "maxRetransmits and maxPacketLifeTimeMs are mutually exclusive."};
        }
        if (Contains(spec.label)) {
            return DataChannelCreateError{
                "duplicate_channel_label",
                "A DataChannel with this label already exists."};
        }

        webrtc::DataChannelInit nativeConfig;
        nativeConfig.ordered = spec.ordered;
        nativeConfig.maxRetransmits = spec.maxRetransmits;
        nativeConfig.maxRetransmitTime = spec.maxPacketLifeTimeMs;
        nativeConfig.protocol = spec.protocol;
        if (spec.priority) {
            switch (*spec.priority) {
            case DataChannelSpec::Priority::kLow:
                nativeConfig.priority = webrtc::PriorityValue(
                    webrtc::Priority::kLow);
                break;
            case DataChannelSpec::Priority::kMedium:
                nativeConfig.priority = webrtc::PriorityValue(
                    webrtc::Priority::kMedium);
                break;
            case DataChannelSpec::Priority::kHigh:
                nativeConfig.priority = webrtc::PriorityValue(
                    webrtc::Priority::kHigh);
                break;
            }
        }
        auto channelOrError =
            peer->CreateDataChannelOrError(spec.label, &nativeConfig);
        if (!channelOrError.ok()) {
            return DataChannelCreateError{
                "data_channel_create_failed",
                channelOrError.error().message()};
        }
        if (!Attach(channelOrError.MoveValue())) {
            return DataChannelCreateError{
                "duplicate_channel_label",
                "A DataChannel with this label already exists."};
        }
    }
    return std::nullopt;
}

bool DataChannelManager::Attach(
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    if (!channel) {
        return false;
    }
    const std::string label = channel->label();
    auto binding = std::make_unique<Binding>(this, channel);
    bool inserted = false;
    {
        std::lock_guard lock(mutex_);
        if (!channels_.contains(label)) {
            channels_.emplace(label, std::move(binding));
            inserted = true;
        }
    }
    if (!inserted) {
        channel->Close();
        return false;
    }
    PublishState(std::move(channel));
    return true;
}

SendResult DataChannelManager::Send(
    const std::string& channelName,
    std::span<const std::uint8_t> data,
    bool binary) const
{
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel;
    {
        std::lock_guard lock(mutex_);
        const auto found = channels_.find(channelName);
        if (found == channels_.end()) {
            return SendResult::kChannelNotFound;
        }
        channel = found->second->Channel();
    }
    if (channel->state() != webrtc::DataChannelInterface::kOpen) {
        return SendResult::kChannelNotOpen;
    }

    const webrtc::CopyOnWriteBuffer payload(data.data(), data.size());
    return channel->Send(webrtc::DataBuffer(payload, binary))
               ? SendResult::kSent
               : SendResult::kSendFailed;
}

std::optional<std::uint64_t> DataChannelManager::BufferedAmount(
    const std::string& channelName) const
{
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel;
    {
        std::lock_guard lock(mutex_);
        const auto found = channels_.find(channelName);
        if (found == channels_.end()) {
            return std::nullopt;
        }
        channel = found->second->Channel();
    }
    if (!channel ||
        channel->state() != webrtc::DataChannelInterface::kOpen) {
        return std::nullopt;
    }
    return channel->buffered_amount();
}

void DataChannelManager::Close()
{
    std::unordered_map<std::string, std::unique_ptr<Binding>> channels;
    {
        std::lock_guard lock(mutex_);
        channels = std::move(channels_);
    }
    for (auto& [label, binding] : channels) {
        (void)label;
        binding->Close();
    }
}

bool DataChannelManager::Contains(const std::string& label) const
{
    std::lock_guard lock(mutex_);
    return channels_.contains(label);
}

void DataChannelManager::PublishState(
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    if (!channel || !stateCallback_) {
        return;
    }
    DataChannelInfo info;
    info.label = channel->label();
    info.state = ToPublicState(channel->state());
    info.ordered = channel->ordered();
    info.maxRetransmits = channel->maxRetransmitsOpt();
    info.maxPacketLifeTimeMs = channel->maxPacketLifeTime();
    info.protocol = channel->protocol();
    stateCallback_(info);
}

void DataChannelManager::PublishMessage(
    const std::string& label,
    const webrtc::DataBuffer& buffer)
{
    if (!messageCallback_) {
        return;
    }
    std::vector<std::uint8_t> payload(buffer.data.size());
    if (!payload.empty()) {
        std::copy_n(buffer.data.cdata(), payload.size(), payload.data());
    }
    messageCallback_(label, payload, buffer.binary);
}

}  // namespace remote
