/*!
 * Copyright (C) 2015-2020 by Savoir-faire Linux
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

int
main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.utf8");

    QList<char*> qtWebEngineChromiumFlags;

#ifdef Q_OS_LINUX
    setenv("QT_QPA_PLATFORMTHEME", "gtk3", true);
    setenv("QML_DISABLE_DISK_CACHE", "1", true);
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

    QtWebEngineQuick::initialize();

    QApplication::setApplicationName("Jami");
    QApplication::setOrganizationDomain("jami.net");
    QApplication::setQuitOnLastWindowClosed(false);
    QCoreApplication::setApplicationVersion(QString(VERSION_STRING));
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

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
