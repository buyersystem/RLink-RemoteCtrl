// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DirectConnectPage.h"

#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "src/apps/controller/FramelessWindow.h"
#include "src/apps/controller/ui/RemoteCTheme.h"

namespace remote::controller {

DirectConnectPage::DirectConnectPage(QWidget* parent)
    : QScrollArea(parent)
{
    setObjectName(QStringLiteral("localDevicePage"));
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    EnableSmoothWheelScrolling(this);

    auto* content = new QWidget(this);
    contentLayout_ = new QVBoxLayout(content);
    contentLayout_->setContentsMargins(32, 28, 32, 30);
    contentLayout_->setSpacing(20);

    auto* header = new QVBoxLayout();
    header->setSpacing(3);
    auto* title = new QLabel(QStringLiteral("远程协助"), content);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitle = new QLabel(
        QStringLiteral("查看本机身份、信令状态和可用的媒体能力。"),
        content);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    subtitle->setWordWrap(true);
    header->addWidget(title);
    header->addWidget(subtitle);
    contentLayout_->addLayout(header);
    setWidget(content);

    BuildLoginPrompt();
    BuildSignedInWorkspace();
    contentLayout_->addStretch(1);

    QFile pageStyle(QStringLiteral(":/ui/theme/local-device.qss"));
    if (pageStyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(QString::fromUtf8(pageStyle.readAll()));
    }
}

void DirectConnectPage::BuildLoginPrompt()
{
    loginPrompt_ = new QFrame(this);
    loginPrompt_->setObjectName(QStringLiteral("localDeviceLoginHero"));
    auto* loginPromptLayout = new QVBoxLayout(loginPrompt_);
    loginPromptLayout->setContentsMargins(30, 28, 30, 24);
    loginPromptLayout->setSpacing(18);

    auto* loginHeroRow = new QHBoxLayout();
    loginHeroRow->setSpacing(30);
    auto* loginHeroCopy = new QVBoxLayout();
    loginHeroCopy->setSpacing(8);
    loginHeroCopy->setAlignment(Qt::AlignVCenter);
    auto* heroEyebrow =
        new QLabel(QStringLiteral("安全设备接入"), loginPrompt_);
    heroEyebrow->setObjectName(QStringLiteral("localDeviceHeroEyebrow"));
    loginHeroCopy->addWidget(heroEyebrow);
    auto* loginTitle = new QLabel(
        QStringLiteral("登录后，让这台电脑成为你的 RLink 设备"), loginPrompt_);
    loginTitle->setObjectName(QStringLiteral("localDeviceHeroTitle"));
    loginTitle->setWordWrap(true);
    loginHeroCopy->addWidget(loginTitle);
    loginStatus_ = new QLabel(
        QStringLiteral(
            "设备身份、同账号设备、协作房间和远程协助会在登录后自动启用。"),
        loginPrompt_);
    loginStatus_->setObjectName(QStringLiteral("localDeviceHeroBody"));
    loginStatus_->setWordWrap(true);
    loginHeroCopy->addWidget(loginStatus_);
    loginHeroCopy->addSpacing(8);
    loginButton_ = new QPushButton(QStringLiteral("立即登录  →"), loginPrompt_);
    loginButton_->setObjectName(QStringLiteral("deviceLoginButton"));
    loginButton_->setCursor(Qt::PointingHandCursor);
    loginButton_->setMinimumWidth(178);
    loginButton_->setMaximumWidth(224);
    loginButton_->setFixedHeight(46);
    loginHeroCopy->addWidget(loginButton_, 0, Qt::AlignLeft);
    auto* secureLoginNote = new QLabel(
        QStringLiteral("身份验证和会话控制使用加密连接。"), loginPrompt_);
    secureLoginNote->setObjectName(QStringLiteral("localDeviceSecureNote"));
    loginHeroCopy->addWidget(secureLoginNote);
    loginHeroRow->addLayout(loginHeroCopy, 1);

    auto* loginHeroArtwork = new QLabel(loginPrompt_);
    loginHeroArtwork->setAlignment(Qt::AlignCenter);
    loginHeroArtwork->setFixedSize(330, 180);
    loginHeroArtwork->setPixmap(
        QIcon(QStringLiteral(
                  ":/ui/illustrations/devices/local-device-hero.svg"))
            .pixmap(320, 170));
    loginHeroRow->addWidget(loginHeroArtwork, 0, Qt::AlignVCenter);
    loginPromptLayout->addLayout(loginHeroRow);

    auto* featureStrip = new QFrame(loginPrompt_);
    featureStrip->setObjectName(QStringLiteral("localDeviceFeatureStrip"));
    auto* featureLayout = new QHBoxLayout(featureStrip);
    featureLayout->setContentsMargins(14, 12, 14, 12);
    featureLayout->setSpacing(8);
    const auto addLoginFeature =
        [featureStrip, featureLayout](const QString& resource,
                                     const QString& title,
                                     const QString& detail) {
            auto* feature = new QWidget(featureStrip);
            auto* row = new QHBoxLayout(feature);
            row->setContentsMargins(5, 0, 5, 0);
            row->setSpacing(10);
            auto* iconLabel = new QLabel(feature);
            iconLabel->setObjectName(QStringLiteral("localDeviceFeatureIcon"));
            iconLabel->setProperty("localFeatureIconResource", resource);
            iconLabel->setAlignment(Qt::AlignCenter);
            iconLabel->setFixedSize(40, 40);
            const bool dark = ui::RemoteCTheme::IsDark(
                ui::RemoteCTheme::LoadPreference());
            ui::RemoteCTheme::SetPixmap(
                iconLabel, resource, QSize(20, 20),
                resource.contains(QStringLiteral("shield-check"))
                    ? ui::ThemeIconTone::kSuccess
                    : (dark ? ui::ThemeIconTone::kOnDark
                            : ui::ThemeIconTone::kPrimary));
            auto* labels = new QVBoxLayout();
            labels->setSpacing(1);
            auto* titleLabel = new QLabel(title, feature);
            titleLabel->setObjectName(
                QStringLiteral("localDeviceFeatureTitle"));
            auto* detailLabel = new QLabel(detail, feature);
            detailLabel->setObjectName(
                QStringLiteral("localDeviceFeatureDetail"));
            labels->addWidget(titleLabel);
            labels->addWidget(detailLabel);
            row->addWidget(iconLabel);
            row->addLayout(labels, 1);
            featureLayout->addWidget(feature, 1);
        };
    addLoginFeature(
        QStringLiteral(":/ui/icons/lucide/base/shield-check.svg"),
        QStringLiteral("安全"), QStringLiteral("加密保护每一次远程连接"));
    addLoginFeature(
        QStringLiteral(":/ui/icons/lucide/base/monitor-check.svg"),
        QStringLiteral("4K 120FPS"),
        QStringLiteral("超高清高帧率远程桌面"));
    addLoginFeature(
        QStringLiteral(":/ui/icons/lucide/base/star-check.svg"),
        QStringLiteral("免费"), QStringLiteral("核心远程功能免费使用"));
    loginPromptLayout->addWidget(featureStrip);
    contentLayout_->addWidget(loginPrompt_);

    connect(loginButton_, &QPushButton::clicked,
            this, &DirectConnectPage::LoginRequested);
}

QString DirectConnectPage::DeviceId() const
{
    return deviceIdEdit_ ? deviceIdEdit_->text().trimmed() : QString{};
}

QString DirectConnectPage::VerificationCode() const
{
    return verificationCodeEdit_
        ? verificationCodeEdit_->text().trimmed()
        : QString{};
}

void DirectConnectPage::SetActionEnabled(bool enabled)
{
    if (connectButton_) {
        connectButton_->setEnabled(enabled);
    }
}

void DirectConnectPage::SetActionState(bool enabled, const QString& text)
{
    if (!connectButton_) {
        return;
    }
    connectButton_->setEnabled(enabled);
    connectButton_->setText(text);
}

void DirectConnectPage::PrepareCredentials(const QString& deviceId)
{
    if (!deviceIdEdit_ || !verificationCodeEdit_) {
        return;
    }
    deviceIdEdit_->setText(deviceId);
    verificationCodeEdit_->clear();
    verificationCodeEdit_->setFocus(Qt::OtherFocusReason);
}

void DirectConnectPage::ShowSignedOut(const QString& message)
{
    loginStatus_->setText(
        message.isEmpty()
            ? QStringLiteral("登录后即可注册本机设备、创建或加入协作房间。")
            : message);
    loginButton_->setText(QStringLiteral("立即登录   →"));
    loginButton_->setEnabled(true);
    SetAuthenticated(false);
}

void DirectConnectPage::ShowLoginBusy(const QString& message)
{
    loginStatus_->setText(message);
    loginButton_->setText(QStringLiteral("登录进行中"));
    loginButton_->setEnabled(false);
    SetAuthenticated(false);
}

void DirectConnectPage::SetAuthenticated(bool authenticated)
{
    loginPrompt_->setVisible(!authenticated);
    signedInWorkspace_->setVisible(authenticated);
}

void DirectConnectPage::SetAccountLabel(const QString& label)
{
    accountLabel_->setText(label.isEmpty() ? QStringLiteral("未登录") : label);
    accountLabel_->setToolTip(accountLabel_->text());
}

void DirectConnectPage::SetLocalCredentials(
    const QString& deviceId,
    const QString& verificationCode)
{
    const bool validDeviceId = deviceId.size() == 9;
    const bool validCode = verificationCode.size() == 6;
    localDeviceId_->setText(validDeviceId ? deviceId : QStringLiteral("未注册"));
    localVerificationCode_->setText(
        validCode ? verificationCode : QStringLiteral("------"));
    copyDeviceIdButton_->setEnabled(validDeviceId);
    copyVerificationCodeButton_->setEnabled(validCode);
    shareLocalCredentialsButton_->setEnabled(validDeviceId && validCode);
}

void DirectConnectPage::SetSignalStatus(
    const QString& text,
    const QString& styleSheet)
{
    signalStatus_->setText(text);
    if (signalStatus_->styleSheet() != styleSheet) {
        signalStatus_->setStyleSheet(styleSheet);
    }
}

void DirectConnectPage::SetRuntimeStatus(
    const QString& text,
    const QString& color)
{
    runtimeStatus_->setText(text);
    runtimeStatus_->setStyleSheet(QStringLiteral("color:%1;").arg(color));
}

void DirectConnectPage::SetDecoderStatus(
    const QString& text,
    const QString& color)
{
    decoderStatus_->setText(text);
    decoderStatus_->setStyleSheet(QStringLiteral("color:%1;").arg(color));
}

void DirectConnectPage::SetAssistHint(
    const QString& text,
    AssistHintTone tone)
{
    assistHint_->setText(text);
    assistHint_->setProperty("error", tone == AssistHintTone::kError);
    switch (tone) {
    case AssistHintTone::kWarning:
        assistHint_->setStyleSheet(QStringLiteral("color:#a66b12;"));
        break;
    case AssistHintTone::kSuccess:
        assistHint_->setStyleSheet(QStringLiteral("color:#15945f;"));
        break;
    case AssistHintTone::kError:
        assistHint_->setStyleSheet(QStringLiteral("color:#d14343;"));
        break;
    case AssistHintTone::kDefault:
        assistHint_->setStyleSheet({});
        break;
    }
}

bool DirectConnectPage::HasAssistError() const
{
    return assistHint_->property("error").toBool();
}

}  // namespace remote::controller
