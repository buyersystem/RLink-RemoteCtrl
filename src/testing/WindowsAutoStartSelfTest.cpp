// SPDX-License-Identifier: GPL-3.0-only
// Test the production XML and COM registration without enabling real autostart.
#include "src/apps/controller/WindowsAutoStart.cpp"
#include <QTextStream>
#include <QUuid>
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include <QThread>
#include <QTemporaryDir>

namespace remote::controller::detail {
extern const char kWindowsAutoStartValueName[] = "RemoteC";
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    using namespace remote::controller::detail;
    QFile report;
    const int reportOption = app.arguments().indexOf(QStringLiteral("--report"));
    if (reportOption >= 0 && reportOption + 1 < app.arguments().size()) {
        report.setFileName(app.arguments()[reportOption + 1]);
        if (!report.open(QIODevice::WriteOnly | QIODevice::Text)) return 6;
    }
    QFile console;
    if (!report.isOpen() && !console.open(stdout, QIODevice::WriteOnly)) return 6;
    QTextStream out(report.isOpen() ? &report : &console);
    const QString sid = CurrentUserSid();
    const QString xml = TaskXml(sid);
    QXmlStreamReader reader(xml);
    QStringList users, commands;
    bool highest = false, interactive = false, battery = false, noLimit = false;
    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isStartElement()) continue;
        const QString tag = reader.name().toString();
        if (tag == QStringLiteral("UserId")) users << reader.readElementText();
        else if (tag == QStringLiteral("Command")) commands << reader.readElementText();
        else if (tag == QStringLiteral("RunLevel")) highest = reader.readElementText() == QStringLiteral("HighestAvailable");
        else if (tag == QStringLiteral("LogonType")) interactive = reader.readElementText() == QStringLiteral("InteractiveToken");
        else if (tag == QStringLiteral("DisallowStartIfOnBatteries")) battery = reader.readElementText() == QStringLiteral("false");
        else if (tag == QStringLiteral("ExecutionTimeLimit")) noLimit = reader.readElementText() == QStringLiteral("PT0S");
    }
    const bool passed = !sid.isEmpty() && !reader.hasError() && users.size() == 2 &&
        users[0] == sid && users[1] == sid && commands.size() == 1 &&
        commands[0] == QDir::toNativeSeparators(app.applicationFilePath()) &&
        highest && interactive && battery && noLimit;
    out << "AUTOSTART_XML=" << (passed ? "PASS" : "FAIL") << Qt::endl;
    if (!passed) return 1;
    QTemporaryDir preferenceDirectory;
    if (!preferenceDirectory.isValid()) return 8;
    QSettings preferences(preferenceDirectory.filePath(QStringLiteral("autostart.ini")), QSettings::IniFormat);
    int configureCalls = 0;
    bool requestedEnabled = false;
    QString preferenceError;
    const auto configure = [&](bool enabled, QString*) {
        ++configureCalls;
        requestedEnabled = enabled;
        return true;
    };
    const bool first = ApplyAutoStartDefault(preferences, configure, &preferenceError) &&
        configureCalls == 1 && requestedEnabled && preferences.value(QString::fromLatin1(kAutoStartChoice)).toBool();
    const bool repeated = ApplyAutoStartDefault(preferences, configure, &preferenceError) && configureCalls == 1;
    SaveAutoStartChoice(preferences, false, &preferenceError);
    const bool offPreserved = ApplyAutoStartDefault(preferences, configure, &preferenceError) &&
        configureCalls == 1 && !preferences.value(QString::fromLatin1(kAutoStartChoice)).toBool();
    preferences.remove(QString::fromLatin1(kAutoStartChoice));
    const bool failure = !ApplyAutoStartDefault(preferences, [](bool, QString* error) {
        *error = QStringLiteral("expected failure"); return false;
    }, &preferenceError) && !preferences.contains(QString::fromLatin1(kAutoStartChoice));
    const bool retry = ApplyAutoStartDefault(preferences, configure, &preferenceError) && configureCalls == 2;
    const bool defaults = first && repeated && offPreserved && failure && retry;
    out << "AUTOSTART_DEFAULT_ON_AND_EXPLICIT_OFF=" << (defaults ? "PASS" : "FAIL") << Qt::endl;
    if (!defaults) return 8;
    QObject receiver;
    QEventLoop loop;
    bool delivered = false, correctThread = false;
    QueryWindowsAutoStartAsync(&receiver, [&](bool) {
        delivered = true;
        correctThread = QThread::currentThread() == app.thread();
        loop.quit();
    });
    const bool asynchronous = !delivered;
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();
    const bool asyncPassed = asynchronous && delivered && correctThread;
    out << "AUTOSTART_QUERY_ASYNC=" << (asyncPassed ? "PASS" : "FAIL") << Qt::endl;
    if (!asyncPassed) return 7;
    if (!app.arguments().contains(QStringLiteral("--task-scheduler-integration"))) return 0;

    TaskConnection connection;
    HRESULT hr = connection.Open();
    if (FAILED(hr)) { QString error; Failed(&error, QStringLiteral("Connect"), hr); out << error << Qt::endl; return 2; }
    const QString name = QStringLiteral("RLink.AutoStart.SelfTest.%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    VARIANT empty; VariantInit(&empty);
    ComPtr<IRegisteredTask> task;
    // TASK_DISABLE prevents this isolated task from running even on logon.
    hr = connection.folder->RegisterTask(Bstr(name), Bstr(xml), TASK_CREATE | TASK_DISABLE,
        empty, empty, TASK_LOGON_INTERACTIVE_TOKEN, empty, &task);
    if (FAILED(hr)) { QString error; Failed(&error, QStringLiteral("Register"), hr); out << error << Qt::endl; return 3; }
    const bool owned = OwnedTask(task.Get(), sid);
    const bool disabled = !ActiveTask(task.Get());
    const bool validDefinition = ActiveTask(task.Get(), false);
    ComPtr<IRegisteredTask> reread;
    const bool readable = SUCCEEDED(connection.folder->GetTask(Bstr(name), &reread)) && OwnedTask(reread.Get(), sid);
    const HRESULT cleanup = connection.folder->DeleteTask(Bstr(name), 0);
    if (FAILED(cleanup)) { out << "CLEANUP_FAILED=" << name << Qt::endl; return 4; }
    const bool removed = MissingTask(connection.folder->GetTask(Bstr(name), &reread));
    const bool integration = owned && disabled && validDefinition && readable && removed;
    out << "owned=" << owned << " disabled=" << disabled << " definition=" << validDefinition
        << " readable=" << readable << " removed=" << removed << Qt::endl;
    if (!owned) {
        BSTR normalized = nullptr;
        if (SUCCEEDED(task->get_Xml(&normalized))) {
            out << QString::fromWCharArray(normalized) << Qt::endl;
            SysFreeString(normalized);
        }
    }
    out << "AUTOSTART_COM_REGISTER_READ_DELETE=" << (integration ? "PASS" : "FAIL") << Qt::endl;
    return integration ? 0 : 5;
}
