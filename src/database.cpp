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
#include "database.h"

// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

// LRC
#include "private/vcardutils.h"
#include "availableaccountmodel.h"

namespace lrc
{

Database::Database()
: QObject()
{

}

Database::~Database()
{

}

void
Database::addMessage(const std::string& accountId, const message::Info& message) const
{

}

void
Database::clearHistory(const std::string& accountId, const std::string& uid, bool removeContact) const
{

}

MessagesMap
Database::getAllMessages(const std::string& accountId, const std::string& uid) const
{

}

std::size_t
Database::numberOfUnreads(const std::string& accountId, const std::string& uid) const
{

}

void
Database::setMessageRead(int uid) const
{

}

void
Database::addContact(const std::string& contact, const QByteArray& payload) const
{

}

std::string
Database::getContactAttribute(const std::string& uid, const std::string& attribute) const
{

}

} // namespace lrc
