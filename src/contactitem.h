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
#include "smartlistitem.h"
#include "contact.h"
#include "contactmethod.h"

class LIB_EXPORT ContactItem : public SmartListItem {
    public:
    explicit ContactItem(ContactMethod* contact);
    ~ContactItem();

    void setTitle(const std::string) override;
    const std::string getTitle() const override;
    const std::string getAlias() const override;
    const std::string getAvatar() const override;
    const std::string getLastInteraction() const override;
    const std::string getUri();
    const bool isPresent() const override;
    virtual void activate() override;

    // message communication
    void sendMessage(std::string message); // manage only dht message for now

    // video communication
    void placeCall();
    void setCallId(const unsigned int);

    public Q_SLOTS:
    void slotPresenceChanged(bool);

private:
    Contact contact;
    unsigned int callId_ = 0;
};
