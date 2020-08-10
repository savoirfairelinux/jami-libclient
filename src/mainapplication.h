/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

#pragma once

#include <QFile>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>

#include <memory>

class MainApplication : public QApplication
{
    Q_OBJECT

public:
    explicit MainApplication(int &argc, char **argv);
    ~MainApplication();

    bool applicationSetup();
    void exitApp();

    static void applicationInitialization();
    static QString getDebugFilePath();
    static char **parseInputArgument(int &argc, char *argv[], char *argToParse);

protected:
    void consoleDebug();
    void vsConsoleDebug();
    void fileDebug(QFile *debugFile);

private:
    void loadTranslations();
    void initLrc();
    void processInputArgument(bool &startMinimized);
    void setApplicationFont();
    void qmlInitialization();

    std::unique_ptr<QFile> debugFile_;
    QQmlApplicationEngine *engine_;
};