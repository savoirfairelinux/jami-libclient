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


class LIB_EXPORT SmartListItem : public QObject {
    Q_OBJECT

    public:
    virtual ~SmartListItem();

    virtual void setTitle(const std::string) {};
    virtual const std::string getTitle() const { return _title->data(); };
    virtual void activate() {};
    virtual const std::string getAlias() const { return ""; };
    virtual const std::string getAvatar() const { return ""; };
    virtual const std::string getLastInteraction() const { return ""; };
    virtual const bool getPresence() const = 0;

    protected:
    SmartListItem();

    std::shared_ptr<std::string> _title;

};
