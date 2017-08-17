/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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
// Parent
#include "newconversationitem.h"

// Lrc
#include "accountmodel.h"
#include "smartlistmodel.h"
#include "database.h"
#include "availableaccountmodel.h"
#include "phonedirectorymodel.h"
#include "uri.h"

// Qt
#include <qstring.h>

// Debug
#include <qdebug.h>

// Ring daemon
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"

NewConversationItem::NewConversationItem()
: ContactItem(), alias_("Searching...")
{
}

NewConversationItem::~NewConversationItem()
{
}

void
NewConversationItem::setTitle(const std::string title)
{
    if (not _title)
        _title = std::unique_ptr<std::string>(new std::string());

    _title->assign(title);
}

const std::string
NewConversationItem::getTitle() const
{
    return _title->data();
}

void
NewConversationItem::activate()
{
    if (contact_.id.length() > 0) {
        // If a contact is linked, we can show a new conversation
        emit SmartListModel::instance().newConversationItemActivated(this);
    }
}

const bool
NewConversationItem::isPresent() const
{
    return false;
}

void
NewConversationItem::search(const std::string& query)
{
    // Update alias
    Contact emptyContact;
    contact_ = emptyContact;
    alias_ = "Searching..." + query;
    emit changed();
    // Query NS
    auto uri = URI(QString(query.c_str()));
    Account* account = nullptr;
    if (uri.schemeType() != URI::SchemeType::NONE) {
        account = AvailableAccountModel::instance().currentDefaultAccount(uri.schemeType());
    } else {
        account = AvailableAccountModel::instance().currentDefaultAccount();
    }
    if (!account) return;
    connect(&NameDirectory::instance(), &NameDirectory::registeredNameFound, this, &NewConversationItem::registeredNameFound);
    if (account->protocol() == Account::Protocol::RING &&
        uri.protocolHint() != URI::ProtocolHint::RING)
    {
        account->lookupName(QString(query.c_str()));
    } else {
        /* no lookup, simply use the URI as is */
        auto cm = PhoneDirectoryModel::instance().getNumber(uri, account);
        alias_ = cm->bestName().toStdString();
        setMinimumContact(cm->uri().toStdString());
    }
}

void
NewConversationItem::registeredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name)
{
    Q_UNUSED(account)
    Q_UNUSED(address)
    if (status == NameDirectory::LookupStatus::SUCCESS) {
        alias_ = name.toStdString();
        setMinimumContact(address.toStdString());
    }
    disconnect(&NameDirectory::instance(), &NameDirectory::registeredNameFound, this, &NewConversationItem::registeredNameFound);
}

void
NewConversationItem::setMinimumContact(const std::string& address)
{
    Contact newContact;
    newContact.uri = address;
    newContact.id = address;
    newContact.registeredName = alias_;
    newContact.displayName = alias_;
    newContact.avatar = "";
    newContact.isPresent = false;
    newContact.unreadMessages = 0;
    contact_ = newContact;
    emit contactFound(address);
}


void
NewConversationItem::addContact()
{
    if (contact_.id.length() == 0) return;
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    ConfigurationManager::instance().addContact(account->id(), QString(contact_.id.c_str()));
}


void
NewConversationItem::sendInvitation()
{
    addContact();
    connect(&AccountModel::instance(), &AccountModel::daemonContactAdded, this, &NewConversationItem::slotContactAdded);
}

void
NewConversationItem::slotContactAdded(const std::string& uid)
{
    emit contactAdded(uid);
}

void
NewConversationItem::slotContactAddedAndCall(const std::string& uid)
{
    emit contactAddedAndCall(uid);
}

void
NewConversationItem::slotContactAddedAndSend(const std::string& uid)
{
    emit contactAddedAndSend(uid, awaitingMessage_);
}

void
NewConversationItem::sendMessage(std::string message)
{
    addContact();
    awaitingMessage_ = message;
    connect(&AccountModel::instance(), &AccountModel::daemonContactAdded, this, &NewConversationItem::slotContactAddedAndSend);
}

void
NewConversationItem::placeCall()
{
    addContact();
    connect(&AccountModel::instance(), &AccountModel::daemonContactAdded, this, &NewConversationItem::slotContactAddedAndCall);
}



#include <newconversationitem.moc>
