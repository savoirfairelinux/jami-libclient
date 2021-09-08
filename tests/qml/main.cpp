/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Albert Babí Oller <albert.babi@savoirfairelinux.com>
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
#include "qmlregister.h"
#include "appsettingsmanager.h"
#include "connectivitymonitor.h"
#include "systemtray.h"
#include "previewengine.h"

#include <atomic>

#include <QScopedPointer>
#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QtWebEngineCore>
#include <QtWebEngineQuick>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#if defined _MSC_VER && !COMPILE_ONLY
#include <gnutls/gnutls.h>
#endif

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup(bool muteDring = false)
        : muteDring_(muteDring)
    {}

    void init()
    {
        connectivityMonitor_.reset(new ConnectivityMonitor(this));
        settingsManager_.reset(new AppSettingsManager(this));
        systemTray_.reset(new SystemTray(settingsManager_.get(), this));

        QFontDatabase::addApplicationFont(":/images/FontAwesome.otf");

#if defined _MSC_VER && !COMPILE_ONLY
        gnutls_global_init();
#endif

        lrcInstance_.reset(
            new LRCInstance(nullptr, nullptr, "", connectivityMonitor_.get(), muteDring_));
        lrcInstance_->subscribeToDebugReceived();

        auto downloadPath = settingsManager_->getValue(Settings::Key::DownloadPath);
        lrcInstance_->accountModel().downloadDirectory = downloadPath.toString() + "/";
    }

    void qmlEngineRegistration(QQmlEngine* engine)
    {
        // Expose custom types to the QML engine.
        Utils::registerTypes(engine,
                             systemTray_.get(),
                             lrcInstance_.get(),
                             settingsManager_.get(),
                             previewEngine_.get(),
                             &screenInfo_,
                             this);
    }

public Q_SLOTS:

    /*!
     * Called once before qmlEngineAvailable.
     */
    void applicationAvailable()
    {
        init();
    }

    /*!
     * Called when the QML engine is available. Any import paths, plugin paths,
     * and extra file selectors will have been set on the engine by this point.
     * This function is called once for each QML test file, so any arguments are
     * unique to that test. For example, this means that each QML test file will
     * have its own QML engine.
     *
     * This function can be used to register QML types and add import paths,
     * amongst other things.
     */
    void qmlEngineAvailable(QQmlEngine* engine)
    {
        qmlEngineRegistration(engine);
    }

    /*!
     * Called once right after the all test execution has finished. Use this
     * function to clean up before everything is destroyed.
     */
    void cleanupTestCase() {}

private:
    QScopedPointer<LRCInstance> lrcInstance_;

    QScopedPointer<ConnectivityMonitor> connectivityMonitor_;
    QScopedPointer<AppSettingsManager> settingsManager_;
    QScopedPointer<SystemTray> systemTray_;
    QScopedPointer<PreviewEngine> previewEngine_;
    ScreenInfo screenInfo_;

    bool muteDring_ {false};
};

int
main(int argc, char** argv)
{
    QDir tempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));

    auto jamiDataDir = tempDir.absolutePath() + "\\jami_test\\jami";
    auto jamiConfigDir = tempDir.absolutePath() + "\\jami_test\\.config";
    auto jamiCacheDir = tempDir.absolutePath() + "\\jami_test\\.cache";

    bool envSet = qputenv("JAMI_DATA_HOME", jamiDataDir.toLocal8Bit());
    envSet &= qputenv("JAMI_CONFIG_HOME", jamiConfigDir.toLocal8Bit());
    envSet &= qputenv("JAMI_CACHE_HOME", jamiCacheDir.toLocal8Bit());
    if (!envSet)
        return 1;

    bool muteDring {false};

    // Remove "-mutejamid" from argv, as quick_test_main_with_setup() will
    // fail if given an invalid command-line argument.
    auto end = std::remove_if(argv + 1, argv + argc, [](char* argv) {
        return (strcmp(argv, "-mutejamid") == 0);
    });

    if (end != argv + argc) {
        muteDring = true;

        // Adjust the argument count.
        argc = std::distance(argv, end);
    }

    QtWebEngineQuick::initialize();

    QTEST_SET_MAIN_SOURCE_PATH
    Setup setup(muteDring);
    return quick_test_main_with_setup(argc, argv, "qml_test", nullptr, &setup);
}

#include "main.moc"
