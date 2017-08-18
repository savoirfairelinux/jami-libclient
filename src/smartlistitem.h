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
#include <memory>
#include <string>

// Qt
#include <qobject.h> // for signals

// Lrc
#include "typedefs.h"
#include "globals.h"
#include "message.h"


class LIB_EXPORT SmartListItem : public QObject {
    Q_OBJECT

    public:
    virtual ~SmartListItem();

    virtual void setUID(const std::string& newUID);
    virtual const std::string getUID() const;

    // Informations about the item
    virtual const std::string getTitle() const = 0;
    virtual const std::string getAvatar() const = 0;
    virtual const std::string getLastInteraction() const = 0;
    virtual long int getLastInteractionTimeStamp() const = 0;
    virtual bool isPresent() const = 0;

    // Actions related
    virtual void activate() = 0;
    virtual Messages getHistory() const = 0;
    virtual void removeHistory() = 0;
    virtual void placeCall() = 0;

    Q_SIGNALS:
    void changed(SmartListItem*);
    void lastInteractionChanged(SmartListItem*);
    void newMessage(Message);

    protected:
    SmartListItem();

    std::shared_ptr<std::string> _uid;

};
