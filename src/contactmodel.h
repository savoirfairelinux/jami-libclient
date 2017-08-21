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
#pragma once

// Std
#include <memory>

// Qt
#include <qobject.h>

// Data
#include "data/contact.h"
#include "data/account.h"

namespace lrc
{

class NewAccountModel;
class ConversationModel;
class Database;

class ContactModel : public QObject {
    Q_OBJECT

    friend class NewAccountModel;
    friend class ConversationModel;


public:
    const account::Info& owner;

    ~ContactModel();

    void addContact(const std::string& uri);
    void removeContact(const std::string& uri);
    const contact::Info& getContact(const std::string& uri);
    const ContactsInfoMap& getAllContacts() const;
    void nameLookup(const std::string& uri) const;
    void addressLookup(const std::string& name) const;

Q_SIGNALS:
    void contactsChanged();

private Q_SLOTS:
    // TODO remove this from here when LRC signals are added
    void slotContactsAdded(const QString &accountID, const QString &uri, bool confirmed);
    void slotContactsRemoved(const QString &accountID, const QString &uri, bool status);

private:
    explicit ContactModel(ConversationModel& parent,
                          const Database& db,
                          const account::Info& info);

    ContactModel(const ContactModel& contactModel);
    bool fillsWithContacts();
    void sendMessage(const std::string& uri, const std::string& body) const;
    void setContactPresent(const std::string& uri, bool status);

    ContactsInfoMap contacts_;
    const Database& db_;
    ConversationModel& parent_;

};

}
