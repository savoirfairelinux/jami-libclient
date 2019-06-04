/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "daemon_connector.h"

// daemon
#include <account_const.h>

// Models and database
#include "api/account.h"
#include "api/lrc.h"
#include "api/newaccountmodel.h"
#include "api/datatransfermodel.h"
#include "api/behaviorcontroller.h"

// Lrc
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"
#include "dbus/videomanager.h"
#include "namedirectory.h"
#include "waitforsignalhelper.h"

// Qt
#include <QEventLoop>

typedef std::string AccountId;
typedef std::string Alias;

class DaemonPimpl {
    public:
    DaemonPimpl();
    ~DaemonPimpl();

    QEventLoop eventLoop_;
    std::atomic_bool stopLoop_;
    std::map<Alias, AccountId> accountsCreated_;
};

DaemonPimpl::DaemonPimpl() {}
DaemonPimpl::~DaemonPimpl() {
    for (const auto& [_, id] : accountsCreated_) {
        qWarning() << "Remove account " << id.c_str();
        ConfigurationManager::instance().removeAccount(id.c_str());
    }
}


Daemon::Daemon()
: pimpl_{std::make_unique<DaemonPimpl>()}
{}

Daemon::~Daemon()
{}

void
Daemon::addAccount(const std::string& alias, bool isSip)
{
    QString accountId;
    bool accountReady = false;
    auto i = 0;
    auto waitSignalHelper = std::make_unique<WaitForSignalHelper>([&]() {
            MapStringString details = ConfigurationManager::instance().getAccountTemplate("RING");
            using namespace DRing::Account;
            details[ConfProperties::TYPE] = isSip ? "SIP" : "RING";
            details[ConfProperties::DISPLAYNAME] = alias.c_str();
            details[ConfProperties::ALIAS] = alias.c_str();
            details[ConfProperties::UPNP_ENABLED] = "true";
            details[ConfProperties::ARCHIVE_PASSWORD] = "";
            details[ConfProperties::ARCHIVE_PIN] = "";
            details[ConfProperties::ARCHIVE_PATH] = "";
            accountId = ConfigurationManager::instance().addAccount(details);
        });
    do {
        if (i != 0) {
            waitSignalHelper = std::make_unique<WaitForSignalHelper>([&]() {});  // Just wait for more registrationStateChanged
        }
        waitSignalHelper->setSignal("registrationStateChanged", ConfigurationManager::instance(),
                SIGNAL(registrationStateChanged(const QString&, const QString&, int, const QString&)));
        auto accountSignals = waitSignalHelper->wait(1000);

        MapStringString volatileDetails = ConfigurationManager::instance().getVolatileAccountDetails(accountId);
        auto daemonStatus = volatileDetails[DRing::Account::ConfProperties::Registration::STATUS];
        accountReady = (daemonStatus == "REGISTERED");
        ++i;

        qDebug() << "Account (" << accountId << ") creation - status: " << daemonStatus;

    } while (!accountReady && i < 7 /* secs */);

    pimpl_->accountsCreated_.emplace(std::pair<Alias, AccountId>(alias, accountId.toStdString()));
}

std::string
Daemon::getAccountId(const std::string& alias) const
{
    std::string result;
    try {
        result = pimpl_->accountsCreated_[alias];
    } catch (...) {
        // ignore
    }
    return result;
}

void
Daemon::addNewDevice(const std::string& accountId, const std::string& id, const std::string& name)
{

}

std::vector<std::string>
Daemon::getAccountList() const
{
    QStringList accounts = ConfigurationManager::instance().getAccountList();
    std::vector<std::string> result;
    for (const auto& account : accounts) {
        result.emplace_back(account.toStdString());
    }
    return result;
}