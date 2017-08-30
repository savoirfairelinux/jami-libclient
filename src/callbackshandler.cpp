/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#include "callbackshandler.h"

// Models and database
#include "api/lrc.h"
#include "api/newaccountmodel.h"

// Lrc
#include "account.h"
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"
#include "namedirectory.h"

namespace lrc
{

using namespace api;

CallbacksHandler::CallbacksHandler(const Lrc& parent)
: QObject()
, parent(parent)
{
    // Get signals from daemon
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::incomingAccountMessage,
            this,
            &CallbacksHandler::slotNewAccountMessage);

    connect(&PresenceManager::instance(),
            &PresenceManagerInterface::newBuddyNotification,
            this,
            &CallbacksHandler::slotNewBuddySubscription);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::contactAdded,
            this,
            &CallbacksHandler::slotContactAdded);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::contactRemoved,
            this,
            &CallbacksHandler::slotContactRemoved);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::incomingTrustRequest,
            this,
            &CallbacksHandler::slotIncomingContactRequest);

    connect(&NameDirectory::instance(),
            &NameDirectory::registeredNameFound,
            this,
            &CallbacksHandler::slotRegisteredNameFound);
    connect(&CallManager::instance(),
            &CallManagerInterface::incomingCall,
            this,
            &CallbacksHandler::slotIncomingCall);

    connect(&CallManager::instance(),
            &CallManagerInterface::callStateChanged,
            this,
            &CallbacksHandler::slotCallStateChanged);
}

CallbacksHandler::~CallbacksHandler()
{

}

void
CallbacksHandler::slotNewAccountMessage(const QString& accountId,
                                        const QString& from,
                                        const QMap<QString,QString>& payloads)
{
    std::map<std::string,std::string> stdPayloads;

    for (auto item : payloads.keys())
        stdPayloads[item.toStdString()] = payloads.value(item).toStdString();

    emit NewAccountMessage(accountId.toStdString(), from.toStdString(), stdPayloads);
}

void
CallbacksHandler::slotNewBuddySubscription(const QString& accountId,
                                           const QString& uri,
                                           bool status,
                                           const QString& message)
{
    emit NewBuddySubscription(uri.toStdString());
}

void
CallbacksHandler::slotContactAdded(const QString& accountId,
                                   const QString& contactUri,
                                   bool confirmed)
{
    emit contactAdded(accountId.toStdString(), contactUri.toStdString(), confirmed);
}

void
CallbacksHandler::slotContactRemoved(const QString& accountId,
                                     const QString& contactUri,
                                     bool banned)
{
    emit contactRemoved(accountId.toStdString(), contactUri.toStdString(), banned);
}

void
CallbacksHandler::slotIncomingContactRequest(const QString& accountId,
                                             const QString& ringID,
                                             const QByteArray& payload,
                                             time_t time)
{
    Q_UNUSED(time)
    emit incomingContactRequest(accountId.toStdString(), ringID.toStdString(), payload.toStdString());
}

void
CallbacksHandler::slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status,
                                          const QString& address, const QString& name)
{
    if (status == NameDirectory::LookupStatus::SUCCESS) {
        emit registeredNameFound(account->id().toStdString(), address.toStdString(), name.toStdString());
    }
}

void
CallbacksHandler::slotIncomingCall(const QString &accountID, const QString &callID, const QString &fromQString)
{
    auto from = fromQString.toStdString();

    // during a call we receiving something like :
    // "gargouille <6f42876966f3eb12c5ad33c33398e0fb22c6cea4@ring.dht>"
    // we trim to get only the ringid
    from.erase(0, from.find('<')+1);
    from.erase(from.find('@'));

    emit incomingCall(accountID.toStdString(), callID.toStdString(), from);
}

void
CallbacksHandler::slotCallStateChanged(const QString& callId, const QString& state, int code)
{
    emit callStateChanged(callId.toStdString(), state.toStdString(), code);
}

} // namespace lrc
