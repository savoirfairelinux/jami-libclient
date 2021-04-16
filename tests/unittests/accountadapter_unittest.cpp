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

#include "globaltestenvironment.h"

#include "mainapplication.h"
#include "qmlregister.h"
#include "appsettingsmanager.h"
#include "connectivitymonitor.h"
#include "systemtray.h"

#include "accountadapter.h"

#include <gtest/gtest.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#if defined _MSC_VER && !COMPILE_ONLY
#include <gnutls/gnutls.h>
#endif

/*!
 * Test fixture for AccountAdapter testing
 */
class AccountAdapterFixture : public ::testing::Test
{
public:
    // Prepare unit test context. Called at
    // prior each unit test execution
    void SetUp() override
    {
        connectivityMonitor_.reset(new ConnectivityMonitor(nullptr));
        settingsManager_.reset(new AppSettingsManager(nullptr));
        systemTray_.reset(new SystemTray(settingsManager_.get(), nullptr));

#if defined _MSC_VER && !COMPILE_ONLY
        gnutls_global_init();
#endif

        std::atomic_bool isMigrating(false);
        lrcInstance_.reset(
            new LRCInstance(nullptr, nullptr, "", connectivityMonitor_.get(), muteDring));
        lrcInstance_->subscribeToDebugReceived();

        // setup the adapters (their lifetimes are that of MainApplication)
        accountAdapter_.reset(
            new AccountAdapter(settingsManager_.get(), lrcInstance_.data(), nullptr));
    }

    // Close unit test context. Called
    // after each unit test ending
    void TearDown() override {}

    QScopedPointer<AccountAdapter> accountAdapter_;

    QScopedPointer<LRCInstance> lrcInstance_;
    QScopedPointer<ConnectivityMonitor> connectivityMonitor_;
    QScopedPointer<AppSettingsManager> settingsManager_;
    QScopedPointer<SystemTray> systemTray_;
};

/*!
 * WHEN  There is no account initially.
 * THEN  Account list should be empty.
 */
TEST_F(AccountAdapterFixture, InitialAccountListCheck)
{
    auto accountListSize = lrcInstance_->accountModel().getAccountList().size();

    ASSERT_EQ(accountListSize, 0);
}