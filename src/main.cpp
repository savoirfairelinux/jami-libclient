/*!
 * Copyright (C) 2015-2022 Savoir-faire Linux Inc.
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mainapplication.h"
#include "runguard.h"
#include "version.h"

#include <QCryptographicHash>
#include <QApplication>
#include <QtWebEngineCore>
#include <QtWebEngineQuick>

#include <clocale>

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/syscall.h>
#else
#include <thread>
#endif

#ifndef ENABLE_TESTS

static char**
parseInputArgument(int& argc, char* argv[], QList<char*> argsToParse)
{
    /*
     * Forcefully append argsToParse.
     */
    int oldArgc = argc;
    argc += argsToParse.size();
    char** newArgv = new char*[argc];
    for (int i = 0; i < oldArgc; i++) {
        newArgv[i] = argv[i];
    }

    for (int i = oldArgc; i < argc; i++) {
        newArgv[i] = argsToParse.at(i - oldArgc);
    }
    return newArgv;
}

// Qt WebEngine Chromium Flags
static char noSandbox[] {"--no-sandbox"};
static char disableWebSecurity[] {"--disable-web-security"};
static char singleProcess[] {"--single-process"};

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
static char TIMESTAMP_FORMAT[] {"hh:mm:ss.zzz"};

static QString
messageLevel(QtMsgType type)
{
    if (type == QtDebugMsg)
        return QString("D");
    if (type == QtInfoMsg)
        return QString("I");
    if (type == QtWarningMsg)
        return QString("W");
    if (type == QtCriticalMsg)
        return QString("E");
    if (type == QtFatalMsg)
        return QString("F");
    return QString("Unknown message type");
}
#endif

#ifdef Q_OS_LINUX
static long
getCurrentTid()
{
    return syscall(__NR_gettid) & 0xffff;
}
#elif defined(Q_OS_WIN)
static thread::id
getCurrentTid()
{
    return std::this_thread::get_id();
}
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
static QString
getFileBaseName(const QMessageLogContext& context)
{
    QFileInfo info(context.file);
    return info.baseName() + "." + info.suffix();
}

static void
messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QTextStream strm(stdout);
    strm << "[";
    strm << QDateTime::currentDateTime().toString(TIMESTAMP_FORMAT);
    strm << "|";
    strm << getCurrentTid();
    strm << "|";
    strm << messageLevel(type);
    strm << "|";
    strm << getFileBaseName(context) << ":" << context.line;
    strm << "] ";
    strm << msg;
    strm << Qt::endl;
}
#endif

int
main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.utf8");

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    // Set the default logging handler. Do not override if the
    // user defines its own format.
    if (getenv("QT_MESSAGE_PATTERN") == nullptr) {
        qInstallMessageHandler(messageHandler);
    }
#endif

    QList<char*> qtWebEngineChromiumFlags;

#ifdef Q_OS_LINUX
    setenv("QT_QPA_PLATFORMTHEME", "gtk3", true);
    setenv("QML_DISABLE_DISK_CACHE", "1", true);

    /*
     * Some GNU/Linux distros, like Zorin OS, set QT_STYLE_OVERRIDE
     * to force a particular Qt style.  This has been fine with Qt5
     * even when using our own Qt package which may not have that
     * style available.  However, with Qt6, attempting to override
     * to a nonexistent style seems to result in the main window
     * simply not showing.  So here we unset this variable, also
     * because we currently hard-code the Material style anyway.
     * https://bugreports.qt.io/browse/QTBUG-99889
     */
    unsetenv("QT_STYLE_OVERRIDE");
#ifdef __GLIBC__
    // Current glibc is causing some bugs with font loading
    // See https://bugreports.qt.io/browse/QTBUG-92969
    // As I prefer to not use custom patched Qt, just wait for a
    // new version with this bug fixed
    if (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 33))
        qtWebEngineChromiumFlags << noSandbox;
#endif
#endif
    qtWebEngineChromiumFlags << disableWebSecurity;
    qtWebEngineChromiumFlags << singleProcess;

    QApplication::setApplicationName("Jami");
    QApplication::setOrganizationDomain("jami.net");
    QApplication::setQuitOnLastWindowClosed(false);
    QCoreApplication::setApplicationVersion(QString(VERSION_STRING));
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

#if defined(Q_OS_MACOS)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::MetalRhi);
#elif defined(Q_OS_WIN)
    QQuickWindow::setGraphicsApi(QSGRendererInterface::VulkanRhi);
#endif

    auto newArgv = parseInputArgument(argc, argv, qtWebEngineChromiumFlags);

    MainApplication app(argc, newArgv);

    /*
     * Runguard to make sure that only one instance runs at a time.
     * Note: needs to be after the creation of the application
     */
    QCryptographicHash appData(QCryptographicHash::Sha256);
    appData.addData(QApplication::applicationName().toUtf8());
    appData.addData(QApplication::organizationDomain().toUtf8());
    RunGuard guard(appData.result(), &app);
    if (!guard.tryToRun()) {
        return 0;
    }

    if (!app.init()) {
        guard.release();
        return 0;
    }

    /*
     * Exec the application.
     */
    auto ret = app.exec();

    guard.release();
    return ret;
}
#endif
