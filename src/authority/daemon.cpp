/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
// Lrc
#include "daemon.h"

namespace lrc
{

namespace authority
{

namespace daemon
{

void
addContact(const api::account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().addContact(QString(owner.id.c_str()),
    QString(contactUri.c_str()));
}

void
addContact(const api::account::Info& owner, api::contact::Info& contactInfo)
{
    ConfigurationManager::instance().addContact(QString(owner.id.c_str()),
    QString(contactInfo.profileInfo.uri.c_str()));
}

void
removeContact(const api::account::Info& owner, const std::string& contactUri, bool banned)
{
    ConfigurationManager::instance().removeContact(QString(owner.id.c_str()),
    QString(contactUri.c_str()), banned);
}

void
addContactFromPending(const api::account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().acceptTrustRequest(QString(owner.id.c_str()),
    QString(contactUri.c_str()));
}

void
discardFromPending(const api::account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().discardTrustRequest(
        QString(owner.id.c_str()),
        QString(contactUri.c_str())
    );
}

} // namespace daemon

} // namespace authority

} // namespace lrc
