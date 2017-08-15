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
#include "contactitem.h"

// Lrc
#include "smartlistmodel.h"
#include "database.h"
#include "availableaccountmodel.h"

// Qt
#include <qstring.h>
#include <QDateTime>

// Debug
#include <qdebug.h>

// Ring daemon
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"

ContactItem::ContactItem(ContactMethod* cm)
: SmartListItem()
{
    this->contact.uri = cm->uri().toStdString();
    this->contact.avatar = DataBase::instance().getAvatar(QString(this->contact.uri.c_str()));
    this->contact.id = cm->bestId().toStdString();
    this->contact.registeredName = cm->registeredName().toStdString();
    this->contact.displayName = cm->bestName().toStdString();
    this->contact.isPresent = false;
    this->contact.unreadMessages = 0;
}

ContactItem::~ContactItem()
{
}

void
ContactItem::setTitle(const std::string title)
{
    if (not _title)
        _title = std::unique_ptr<std::string>(new std::string());

    _title->assign(title);
}

const std::string
ContactItem::getTitle() const
{
    return _title->data();
}

void
ContactItem::activate()
{
    emit SmartListModel::instance().showConversationView(this);
}

const std::string
ContactItem::getAlias()
{
    return this->contact.displayName;
}

const std::string
ContactItem::getAvatar()
{
    return this->contact.avatar;
}

void
ContactItem::sendMessage(std::string message)
{
    // il faudera traiter les cas messages durant appels et par dht.

    QMap<QString, QString> payloads;
    payloads["text/plain"] = message.c_str();

    auto account = AvailableAccountModel::instance().currentDefaultAccount();

    auto id = ConfigurationManager::instance().sendTextMessage(account->id(), contact.uri.c_str(), payloads);

    DataBase::instance().addMessage(
        contact.uri.c_str(),
        account->id(),
        message.c_str(),
        QString::number(QDateTime::currentMSecsSinceEpoch() / 1000),
        true
    );
}

void
ContactItem::placeCall()
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();

    if (not account) {
        qDebug() << "placeCall, invalid pointer";
        return;
    }

    auto uri = "ring:" + contact.uri;

    CallManager::instance().placeCall(account->id(), uri.c_str());
}

const std::string
ContactItem::getLastInteraction()
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();

    if (not account) {
        qDebug() << "placeCall, invalid pointer";
        return std::string();
    }

    auto messages = DataBase::instance().getMessages(contact.uri.c_str(), account->id());

    if (messages.size() == 0)
        return std::string();

    return messages.back().body;
}

const std::string
ContactItem::getUri()
{
    return contact.uri;
}

void
ContactItem::setCallId(const unsigned int callId)
{
    callId_ = callId;
}

#include <contactitem.moc>
