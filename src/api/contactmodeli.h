/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>      *
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

// Qt
#include <qobject.h>

// Data
#include "api/contact.h"

namespace lrc
{

namespace api
{

class ContactModelI : public QObject {
    Q_OBJECT
public:
    virtual const api::contact::Info& getContact(const std::string& uri) const = 0;
    virtual void addContact(const std::string& uri) = 0;
    virtual void removeContact(const std::string& uri) = 0;
    virtual void nameLookup(const std::string& uri) const = 0;
    virtual void addressLookup(const std::string& name) const = 0;
};

} // namespace interface
} // namespace lrc
