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
#include "contactinfo.h"
#include "callinfo.h"
#include "account.h" // old
#include "databasemanager.h"

namespace lrc
{

class ContactModel : public QObject {
    Q_OBJECT
    public:
    explicit ContactModel(QObject* parent = nullptr);
    ~ContactModel();

    const contact::Info& addContact(const std::string& uri);
    void removeContact(const std::string& uri);
    void sendMessage(const std::string& uri, const std::string& body) const;
    std::shared_ptr<contact::Info> getContact(const std::string& uri);
    const ContactsInfoMap& getContacts() const;
    bool isAContact(const std::string& uri) const;
    void nameLookup(const std::string& uri) const;
    void addressLookup(const std::string& name) const;


    private:
    ContactsInfoMap ringContacts_;
    ContactsInfoMap sipContacts_;
    bool fillsWithContacts();


    ContactsInfoMap contacts_;
    const std::shared_ptr<DatabaseManager> dbm_;
    const Account* account_;

};

}
