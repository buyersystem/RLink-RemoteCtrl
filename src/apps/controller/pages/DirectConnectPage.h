// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QScrollArea>
#include <QString>

class QVBoxLayout;
class QLineEdit;
class QPushButton;
class QFrame;
class QLabel;

namespace remote::controller {

class DirectConnectPage final : public QScrollArea {
    Q_OBJECT

public:
    enum class AssistHintTone {
        kDefault,
        kWarning,
        kSuccess,
        kError,
    };

    explicit DirectConnectPage(QWidget* parent = nullptr);

    QString DeviceId() const;
    QString VerificationCode() const;
    void SetActionEnabled(bool enabled);
    void SetActionState(bool enabled, const QString& text);
    void PrepareCredentials(const QString& deviceId);
    void ShowSignedOut(const QString& message);
    void ShowLoginBusy(const QString& message);
    void SetAuthenticated(bool authenticated);
    void SetAccountLabel(const QString& label);
    void SetLocalCredentials(const QString& deviceId,
                             const QString& verificationCode);
    void SetSignalStatus(const QString& text, const QString& styleSheet);
    void SetRuntimeStatus(const QString& text, const QString& color);
    void SetDecoderStatus(const QString& text, const QString& color);
    void SetAssistHint(const QString& text, AssistHintTone tone);
    bool HasAssistError() const;

signals:
    void LoginRequested();
    void ClipboardTextRequested(const QString& text,
                                const QString& successMessage);
    void CredentialsChanged();
    void ActionRequested(const QString& deviceId,
                         const QString& verificationCode);

private:
    void BuildLoginPrompt();
    void BuildSignedInWorkspace();
    void ShowCopiedFeedback(QPushButton* button, const QString& resetText);

    QVBoxLayout* contentLayout_ = nullptr;
    QFrame* loginPrompt_ = nullptr;
    QLabel* loginStatus_ = nullptr;
    QPushButton* loginButton_ = nullptr;
    QFrame* signedInWorkspace_ = nullptr;
    QLabel* accountLabel_ = nullptr;
    QLabel* localDeviceId_ = nullptr;
    QLabel* localVerificationCode_ = nullptr;
    QPushButton* copyDeviceIdButton_ = nullptr;
    QPushButton* copyVerificationCodeButton_ = nullptr;
    QPushButton* shareLocalCredentialsButton_ = nullptr;
    QLabel* signalStatus_ = nullptr;
    QLabel* runtimeStatus_ = nullptr;
    QLabel* decoderStatus_ = nullptr;
    QLabel* assistHint_ = nullptr;
    QLineEdit* deviceIdEdit_ = nullptr;
    QLineEdit* verificationCodeEdit_ = nullptr;
    QPushButton* connectButton_ = nullptr;
};

}  // namespace remote::controller
