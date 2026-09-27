// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DirectConnectPage.h"

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <utility>

#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"

namespace remote::controller {

void DirectConnectPage::BuildSignedInWorkspace()
{
    signedInWorkspace_ = new QFrame(this);
    signedInWorkspace_->setObjectName(QStringLiteral("localLoggedInWorkspace"));
    signedInWorkspace_->setAttribute(Qt::WA_StyledBackground, true);
    auto* workspaceLayout = new QVBoxLayout(signedInWorkspace_);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);
    workspaceLayout->setSpacing(12);

    auto* mainColumns = new QHBoxLayout();
    mainColumns->setSpacing(22);

    auto* identityPanel = new QFrame(signedInWorkspace_);
    identityPanel->setObjectName(QStringLiteral("localIdentityPanel"));
    identityPanel->setAttribute(Qt::WA_StyledBackground, true);
    identityPanel->setMinimumHeight(330);
    auto* identityLayout = new QVBoxLayout(identityPanel);
    identityLayout->setContentsMargins(24, 22, 24, 22);
    identityLayout->setSpacing(12);

    auto* identityHeader = new QHBoxLayout();
    auto* identityIcon = new QLabel(identityPanel);
    identityIcon->setObjectName(QStringLiteral("localPanelIcon"));
    identityIcon->setAlignment(Qt::AlignCenter);
    identityIcon->setFixedSize(40, 40);
    ui::RemoteCTheme::SetPixmap(
        identityIcon, QStringLiteral(":/ui/icons/light/monitor.svg"),
        QSize(21, 21), ui::ThemeIconTone::kPrimary);
    identityHeader->addWidget(identityIcon);
    auto* identityHeaderText = new QVBoxLayout();
    identityHeaderText->setSpacing(1);
    auto* identityTitle = new QLabel(
        QStringLiteral("本机身份"), identityPanel);
    identityTitle->setObjectName(QStringLiteral("localPanelTitle"));
    identityHeaderText->addWidget(identityTitle);
    identityHeader->addLayout(identityHeaderText, 1);

    shareLocalCredentialsButton_ = new QPushButton(
        QStringLiteral("分享"), identityPanel);
    shareLocalCredentialsButton_->setObjectName(QStringLiteral("softButton"));
    shareLocalCredentialsButton_->setIconSize(QSize(15, 15));
    shareLocalCredentialsButton_->setToolTip(
        QStringLiteral("复制设备 ID 和验证码，可直接发送给对方"));
    ui::RemoteCTheme::SetIcon(
        shareLocalCredentialsButton_,
        QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    shareLocalCredentialsButton_->setCursor(Qt::PointingHandCursor);
    shareLocalCredentialsButton_->setEnabled(false);
    identityHeader->addWidget(
        shareLocalCredentialsButton_, 0, Qt::AlignVCenter);
    identityLayout->addLayout(identityHeader);

    const auto makeCredentialRow =
        [identityPanel](const QString& caption) {
            auto* row = new QFrame(identityPanel);
            row->setObjectName(QStringLiteral("localCredentialRow"));
            row->setAttribute(Qt::WA_StyledBackground, true);
            auto* layout = new QHBoxLayout(row);
            layout->setContentsMargins(14, 10, 10, 10);
            layout->setSpacing(10);
            auto* textLayout = new QVBoxLayout();
            textLayout->setSpacing(2);
            auto* captionLabel = new QLabel(caption, row);
            captionLabel->setObjectName(
                QStringLiteral("localCredentialCaption"));
            textLayout->addWidget(captionLabel);
            layout->addLayout(textLayout, 1);
            return std::pair<QFrame*, QVBoxLayout*>(row, textLayout);
        };

    auto [deviceIdBox, deviceIdText] = makeCredentialRow(
        QStringLiteral("本机设备 ID（9 位）"));
    localDeviceId_ = new QLabel(QStringLiteral("未注册"), deviceIdBox);
    localDeviceId_->setObjectName(QStringLiteral("localCredentialValue"));
    localDeviceId_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    deviceIdText->addWidget(localDeviceId_);
    copyDeviceIdButton_ = new QPushButton(
        QStringLiteral("复制"), deviceIdBox);
    copyDeviceIdButton_->setObjectName(QStringLiteral("softButton"));
    copyDeviceIdButton_->setIconSize(QSize(15, 15));
    ui::RemoteCTheme::SetIcon(
        copyDeviceIdButton_, QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        copyDeviceIdButton_,
        QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
        QSize(15, 15), QColor(QStringLiteral("#2563EB")),
        QColor(QStringLiteral("#12B76A")));
    copyDeviceIdButton_->setCursor(Qt::PointingHandCursor);
    copyDeviceIdButton_->setEnabled(false);
    deviceIdBox->layout()->addWidget(copyDeviceIdButton_);
    identityLayout->addWidget(deviceIdBox);

    auto [verificationBox, verificationText] = makeCredentialRow(
        QStringLiteral("一次性验证码（6 位）"));
    localVerificationCode_ = new QLabel(
        QStringLiteral("------"), verificationBox);
    localVerificationCode_->setObjectName(
        QStringLiteral("localCredentialValue"));
    localVerificationCode_->setProperty("verification", true);
    localVerificationCode_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    verificationText->addWidget(localVerificationCode_);
    copyVerificationCodeButton_ = new QPushButton(
        QStringLiteral("复制"), verificationBox);
    copyVerificationCodeButton_->setObjectName(QStringLiteral("softButton"));
    copyVerificationCodeButton_->setIconSize(QSize(15, 15));
    ui::RemoteCTheme::SetIcon(
        copyVerificationCodeButton_,
        QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        copyVerificationCodeButton_,
        QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
        QSize(15, 15), QColor(QStringLiteral("#2563EB")),
        QColor(QStringLiteral("#12B76A")));
    copyVerificationCodeButton_->setCursor(Qt::PointingHandCursor);
    copyVerificationCodeButton_->setEnabled(false);
    verificationBox->layout()->addWidget(copyVerificationCodeButton_);
    identityLayout->addWidget(verificationBox);

    auto [accountBox, accountText] = makeCredentialRow(
        QStringLiteral("当前账户"));
    accountLabel_ = new QLabel(QStringLiteral("未登录"), accountBox);
    accountLabel_->setObjectName(QStringLiteral("localAccountValue"));
    accountLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    accountText->addWidget(accountLabel_);
    identityLayout->addWidget(accountBox);
    mainColumns->addWidget(identityPanel, 1);

    auto* directAssistCard = new QFrame(signedInWorkspace_);
    directAssistCard->setObjectName(QStringLiteral("localAssistPanel"));
    directAssistCard->setAttribute(Qt::WA_StyledBackground, true);
    directAssistCard->setMinimumHeight(330);
    auto* directAssistLayout = new QVBoxLayout(directAssistCard);
    directAssistLayout->setContentsMargins(24, 22, 24, 22);
    directAssistLayout->setSpacing(12);

    auto* assistHeader = new QHBoxLayout();
    auto* assistIcon = new QLabel(directAssistCard);
    assistIcon->setObjectName(QStringLiteral("localPanelIcon"));
    assistIcon->setAlignment(Qt::AlignCenter);
    assistIcon->setFixedSize(40, 40);
    ui::RemoteCTheme::SetPixmap(
        assistIcon, QStringLiteral(":/ui/icons/light/control.svg"),
        QSize(21, 21), ui::ThemeIconTone::kPrimary);
    assistHeader->addWidget(assistIcon);
    auto* assistHeaderText = new QVBoxLayout();
    assistHeaderText->setSpacing(1);
    auto* directAssistTitle = new QLabel(
        QStringLiteral("远程协助"), directAssistCard);
    directAssistTitle->setObjectName(QStringLiteral("localPanelTitle"));
    auto* directAssistSubtitle = new QLabel(
        QStringLiteral("输入对方设备 ID 和验证码，快速建立远程控制。"),
        directAssistCard);
    directAssistSubtitle->setObjectName(QStringLiteral("localPanelSubtitle"));
    directAssistSubtitle->setWordWrap(true);
    assistHeaderText->addWidget(directAssistTitle);
    assistHeaderText->addWidget(directAssistSubtitle);
    assistHeader->addLayout(assistHeaderText, 1);
    directAssistLayout->addLayout(assistHeader);
    directAssistLayout->addSpacing(2);

    deviceIdEdit_ = new QLineEdit(directAssistCard);
    deviceIdEdit_->setObjectName(QStringLiteral("assistDeviceIdInput"));
    deviceIdEdit_->setPlaceholderText(QStringLiteral("对方设备 ID（9 位）"));
    deviceIdEdit_->setMaxLength(9);
    deviceIdEdit_->setClearButtonEnabled(true);
    deviceIdEdit_->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[1-9][0-9]{0,8}")),
        deviceIdEdit_));
    deviceIdEdit_->setContextMenuPolicy(Qt::NoContextMenu);
    deviceIdEdit_->setFixedHeight(48);
    auto* assistDeviceAction = deviceIdEdit_->addAction(
        QIcon(), QLineEdit::LeadingPosition);
    ui::RemoteCTheme::SetIcon(
        assistDeviceAction, QStringLiteral(":/ui/icons/light/monitor.svg"),
        ui::ThemeIconTone::kNeutral);
    directAssistLayout->addWidget(deviceIdEdit_);

    verificationCodeEdit_ = new QLineEdit(directAssistCard);
    verificationCodeEdit_->setObjectName(
        QStringLiteral("assistVerificationInput"));
    verificationCodeEdit_->setPlaceholderText(
        QStringLiteral("对方验证码（6 位）"));
    verificationCodeEdit_->setMaxLength(6);
    verificationCodeEdit_->setClearButtonEnabled(true);
    verificationCodeEdit_->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[0-9]{0,6}")),
        verificationCodeEdit_));
    verificationCodeEdit_->setContextMenuPolicy(Qt::NoContextMenu);
    verificationCodeEdit_->setFixedHeight(48);
    auto* assistCodeAction = verificationCodeEdit_->addAction(
        QIcon(), QLineEdit::LeadingPosition);
    ui::RemoteCTheme::SetIcon(
        assistCodeAction, QStringLiteral(":/ui/icons/status/success.svg"),
        ui::ThemeIconTone::kSuccess);
    directAssistLayout->addWidget(verificationCodeEdit_);

    connectButton_ = new QPushButton(
        QStringLiteral("发起连接"), directAssistCard);
    connectButton_->setObjectName(QStringLiteral("primaryButton"));
    connectButton_->setIconSize(QSize(19, 19));
    ui::RemoteCTheme::SetIcon(
        connectButton_, QStringLiteral(":/ui/icons/actions/control.svg"),
        ui::ThemeIconTone::kOnDark);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        connectButton_,
        QStringLiteral(":/ui/icons/lucide/base/mouse-pointer-2.svg"),
        QStringLiteral(":/ui/icons/lucide/base/screen-share.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(19, 19), QColor(QStringLiteral("#FFFFFF")),
        QColor(QStringLiteral("#FFFFFF")));
    connectButton_->setCursor(Qt::PointingHandCursor);
    connectButton_->setFixedHeight(48);
    connectButton_->setEnabled(false);
    directAssistLayout->addWidget(connectButton_);

    assistHint_ = new QLabel(
        QStringLiteral("验证成功后自动打开远程桌面；每次成功协助后验证码自动刷新。"),
        directAssistCard);
    assistHint_->setObjectName(QStringLiteral("localPanelSubtitle"));
    assistHint_->setWordWrap(true);
    assistHint_->hide();
    directAssistLayout->addWidget(assistHint_);
    directAssistLayout->addStretch(1);
    mainColumns->addWidget(directAssistCard, 1);
    workspaceLayout->addLayout(mainColumns);

    signalStatus_ = new QLabel(signedInWorkspace_);
    runtimeStatus_ = new QLabel(signedInWorkspace_);
    decoderStatus_ = new QLabel(signedInWorkspace_);
    signalStatus_->hide();
    runtimeStatus_->hide();
    decoderStatus_->hide();

    contentLayout_->addWidget(signedInWorkspace_);

    connect(deviceIdEdit_, &QLineEdit::textChanged,
            this, &DirectConnectPage::CredentialsChanged);
    connect(verificationCodeEdit_, &QLineEdit::textChanged,
            this, &DirectConnectPage::CredentialsChanged);
    connect(deviceIdEdit_, &QLineEdit::returnPressed,
            connectButton_, &QPushButton::click);
    connect(verificationCodeEdit_, &QLineEdit::returnPressed,
            connectButton_, &QPushButton::click);
    connect(connectButton_, &QPushButton::clicked, this, [this] {
        emit ActionRequested(DeviceId(), VerificationCode());
    });
    connect(copyDeviceIdButton_, &QPushButton::clicked, this, [this] {
        const QString deviceId = localDeviceId_->text().trimmed();
        if (deviceId.size() != 9) return;
        ShowCopiedFeedback(copyDeviceIdButton_, QStringLiteral("复制"));
        emit ClipboardTextRequested(deviceId, QStringLiteral("设备 ID 已复制"));
    });
    connect(copyVerificationCodeButton_, &QPushButton::clicked, this, [this] {
        const QString code = localVerificationCode_->text().trimmed();
        if (code.size() != 6) return;
        emit ClipboardTextRequested(code, QStringLiteral("验证码已复制"));
    });
    connect(shareLocalCredentialsButton_, &QPushButton::clicked, this, [this] {
        const QString deviceId = localDeviceId_->text().trimmed();
        const QString code = localVerificationCode_->text().trimmed();
        if (deviceId.size() != 9 || code.size() != 6) return;
        const QString shareText = QStringLiteral(
            "RLink 远程协助\n"
            "设备 ID：%1\n"
            "验证码：%2\n\n"
            "请在验证码有效期内使用。")
            .arg(deviceId, code);
        ShowCopiedFeedback(
            shareLocalCredentialsButton_, QStringLiteral("分享"));
        emit ClipboardTextRequested(
            shareText, QStringLiteral("分享信息已复制，可直接发送给对方"));
    });
}

void DirectConnectPage::ShowCopiedFeedback(
    QPushButton* button,
    const QString& resetText)
{
    button->setText(QStringLiteral("已复制"));
    QTimer::singleShot(1300, button, [button, resetText] {
        button->setText(resetText);
    });
}

}  // namespace remote::controller
