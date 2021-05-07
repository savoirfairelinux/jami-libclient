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
#include "namedirectory.h"
#include "qrimageprovider.h"
#include "tintedbuttonimageprovider.h"
#include "avatarimageprovider.h"

#include "accountadapter.h"
#include "avadapter.h"
#include "calladapter.h"
#include "contactadapter.h"
#include "pluginadapter.h"
#include "messagesadapter.h"
#include "settingsadapter.h"
#include "utilsadapter.h"
#include "conversationsadapter.h"

#include <atomic>

#include <QScopedPointer>
#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QQmlContext>

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

#if defined _MSC_VER && !COMPILE_ONLY
        gnutls_global_init();
#endif

        std::atomic_bool isMigrating(false);
        lrcInstance_.reset(
            new LRCInstance(nullptr, nullptr, "", connectivityMonitor_.get(), muteDring_));
        lrcInstance_->subscribeToDebugReceived();

        auto downloadPath = settingsManager_->getValue(Settings::Key::DownloadPath);
        lrcInstance_->dataTransferModel().downloadDirectory = downloadPath.toString() + "/";
    }

    void qmlEngineRegistration(QQmlEngine* engine)
    {
        // setup the adapters (their lifetimes are that of MainApplication)
        auto callAdapter = new CallAdapter(systemTray_.get(), lrcInstance_.data(), this);
        auto messagesAdapter = new MessagesAdapter(settingsManager_.get(),
                                                   lrcInstance_.data(),
                                                   this);
        auto conversationsAdapter = new ConversationsAdapter(systemTray_.get(),
                                                             lrcInstance_.data(),
                                                             this);
        auto avAdapter = new AvAdapter(lrcInstance_.data(), this);
        auto contactAdapter = new ContactAdapter(lrcInstance_.data(), this);
        auto accountAdapter = new AccountAdapter(settingsManager_.get(), lrcInstance_.data(), this);
        auto utilsAdapter = new UtilsAdapter(systemTray_.get(), lrcInstance_.data(), this);
        auto settingsAdapter = new SettingsAdapter(settingsManager_.get(),
                                                   lrcInstance_.data(),
                                                   this);
        auto pluginAdapter = new PluginAdapter(lrcInstance_.data(), this);

        // qml adapter registration
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, callAdapter, "CallAdapter");
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, messagesAdapter, "MessagesAdapter");
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, conversationsAdapter, "ConversationsAdapter");
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, avAdapter, "AvAdapter");
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, contactAdapter, "ContactAdapter");
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, accountAdapter, "AccountAdapter");
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, utilsAdapter, "UtilsAdapter");
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, settingsAdapter, "SettingsAdapter");
        QML_REGISTERSINGLETONTYPE_POBJECT(NS_ADAPTERS, pluginAdapter, "PluginAdapter");

        // TODO: remove these
        QML_REGISTERSINGLETONTYPE_CUSTOM(NS_MODELS, AVModel, &lrcInstance_->avModel())
        QML_REGISTERSINGLETONTYPE_CUSTOM(NS_MODELS, PluginModel, &lrcInstance_->pluginModel())
        QML_REGISTERSINGLETONTYPE_CUSTOM(NS_HELPERS, UpdateManager, lrcInstance_->getUpdateManager())

        // register other types that don't require injection(e.g. uncreatables, c++/qml singletons)
        Utils::registerTypes();

        engine->addImageProvider(QLatin1String("qrImage"), new QrImageProvider(lrcInstance_.get()));
        engine->addImageProvider(QLatin1String("tintedPixmap"),
                                 new TintedButtonImageProvider(lrcInstance_.get()));
        engine->addImageProvider(QLatin1String("avatarImage"),
                                 new AvatarImageProvider(lrcInstance_.get()));

        engine->rootContext()->setContextProperty("ScreenInfo", &screenInfo_);
        engine->rootContext()->setContextProperty("LRCInstance", lrcInstance_.get());

        engine->setObjectOwnership(&lrcInstance_->avModel(), QQmlEngine::CppOwnership);
        engine->setObjectOwnership(&lrcInstance_->pluginModel(), QQmlEngine::CppOwnership);
        engine->setObjectOwnership(lrcInstance_->getUpdateManager(), QQmlEngine::CppOwnership);
        engine->setObjectOwnership(lrcInstance_.get(), QQmlEngine::CppOwnership);
        engine->setObjectOwnership(&NameDirectory::instance(), QQmlEngine::CppOwnership);
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
        engine->addImportPath("qrc:/tests/qml");
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
    ScreenInfo screenInfo_;

    bool muteDring_ {false};
};

int
main(int argc, char** argv)
{
    bool muteDring {false};

    // Remove "-mutedring" from argv, as quick_test_main_with_setup() will
    // fail if given an invalid command-line argument.
    auto end = std::remove_if(argv + 1, argv + argc, [](char* argv) {
        return (strcmp(argv, "-mutedring") == 0);
    });

    if (end != argv + argc) {
        muteDring = true;

        // Adjust the argument count.
        argc = std::distance(argv, end);
    }

    QStandardPaths::setTestModeEnabled(true);

    QTEST_SET_MAIN_SOURCE_PATH
    Setup setup(muteDring);
    return quick_test_main_with_setup(argc, argv, "qml_test", nullptr, &setup);
}

#include "main.moc"
