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
#pragma once

// Qt
#include <qobject.h>

// Data
#include "contactinfo.h"
#include "callinfo.h"

// Std
#include <memory>

typedef std::map<std::string, std::shared_ptr<CallInfo>> ContactsInfo;

class ContactModel : public QObject {
    Q_OBJECT
    public:
    explicit ContactModel(QObject* parent = nullptr);
    ~ContactModel();

    const ContactInfo& addContact(const std::string& uri);
    void removeContact(const std::string& uri);
    void sendMessage(const std::string& uri, const std::string& body) const;
    const ContactInfo& getContact(const std::string& uri/*TODO: add type*/) const;
    const ContactsInfo& getContacts() const;
    void nameLookup(const std::string& uri) const;
    void addressLookup(const std::string& name) const;


    private:
    ContactsInfo ringContacts_;
    ContactsInfo sipContacts_;

};
