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

// Std
#include <string>

// Lrc
#include "contactitem.h"
#include "contact.h"
#include "namedirectory.h"

class LIB_EXPORT NewConversationItem : public ContactItem {
    Q_OBJECT
    public:
    explicit NewConversationItem();
    ~NewConversationItem();

    const std::string getAlias() const override { return alias_; };
    virtual void activate() override;
    bool isPresent() const override;

    void search(const std::string& query);

    void sendInvitation();
    void sendMessage(std::string message); // manage only dht message for now
    void placeCall();

Q_SIGNALS:
    void contactFound(const std::string& uid);
    // Used by smartlistmodel to transform the NewConversationItem to a Conversation item
    void contactAdded(const std::string& uid);
    void contactAddedAndCall(const std::string& uid);
    void contactAddedAndSend(const std::string& uid, std::string message);

public Q_SLOTS:
    void registeredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name);
    void slotContactAdded(const std::string& uid);
    void slotContactAddedAndCall(const std::string& uid);
    void slotContactAddedAndSend(const std::string& uid);

private:
    void addContact();
    void setMinimumContact(const std::string& address);
    std::string alias_;
    std::string awaitingMessage_;
};
