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
#include <QtWebEngine>

#include <clocale>

static char**
parseInputArgument(int& argc, char* argv[], char* argToParse)
{
    /*
     * Forcefully append argToParse.
     */
    int oldArgc = argc;
    argc = argc + 1 + 1;
    char** newArgv = new char*[argc];
    for (int i = 0; i < oldArgc; i++) {
        newArgv[i] = argv[i];
    }
    newArgv[oldArgc] = argToParse;
    newArgv[oldArgc + 1] = nullptr;
    return newArgv;
}

int
main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.utf8");
#ifdef Q_OS_LINUX
    setenv("QT_QPA_PLATFORMTHEME", "gtk3", true);
#endif
#ifdef Q_OS_WIN
    QApplication::setApplicationName("Ring");
#else
    QApplication::setApplicationName("Jami");
#endif
    QApplication::setOrganizationDomain("jami.net");
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setQuitOnLastWindowClosed(false);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QCoreApplication::setApplicationVersion(QString(VERSION_STRING));
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
    QtWebEngine::initialize();

    char ARG_DISABLE_WEB_SECURITY[] = "--disable-web-security";
    auto newArgv = parseInputArgument(argc, argv, ARG_DISABLE_WEB_SECURITY);
    MainApplication app(argc, newArgv);

    /*
     * Runguard to make sure that only one instance runs at a time.
     * Note: needs to be after the creation of the application
     */
    QCryptographicHash appData(QCryptographicHash::Sha256);
    appData.addData(QApplication::applicationName().toUtf8());
    appData.addData(QApplication::organizationDomain().toUtf8());
    RunGuard guard(appData.result());
    if (!guard.tryToRun()) {
        return 0;
    }

    app.init();

    /*
     * Exec the application.
     */
    auto ret = app.exec();

    guard.release();
    return ret;
}
