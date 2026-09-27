// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SignalServer.Internal.h"

namespace remote::signaling_server {

SignalServer::SignalServer(SignalServerConfig config)
    : impl_(std::make_unique<Impl>(std::move(config)))
{}

SignalServer::~SignalServer()
{
    impl_->Stop();
}

bool SignalServer::Start(QString* error)
{
    return impl_->Start(error);
}

void SignalServer::Stop()
{
    impl_->Stop();
}

bool SignalServer::IsListening() const
{
    return impl_->IsListening();
}

quint16 SignalServer::ServerPort() const
{
    return impl_->ServerPort();
}

quint16 SignalServer::WebhookPort() const
{
    return impl_->WebhookPort();
}

}  // namespace remote::signaling_server
