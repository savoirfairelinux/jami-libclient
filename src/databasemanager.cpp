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
#include "databasemanager.h"

DatabaseManager::DatabaseManager(QObject* parent)
: QObject(parent)
{

}

DatabaseManager::~DatabaseManager()
{

}

void
DatabaseManager::addMessage(const std::string& account, const std::string& uid, const std::string& body, const long timestamp, const bool isOutgoing)
{

}

void
DatabaseManager::removeHistory(const std::string& account, const std::string& uid)
{

}

Messages
DatabaseManager::getMessages(const std::string& account, const std::string& uid) const
{
    return Messages();
}

unsigned int
DatabaseManager::numberOfUnreads(const std::string& account, const std::string& uid) const
{
    return 0;
}

void
DatabaseManager::setMessageRead(int uid)
{

}

void
DatabaseManager::addContact(const std::string& contact, const QByteArray& payload)
{

}

std::string
DatabaseManager::getUri(const std::string& uid) const
{
    return "";
}

std::string
DatabaseManager::getAlias(const std::string& uid) const
{
    return "";
}

std::string
DatabaseManager::getAvatar(const std::string& uid) const
{
    return "";
}
