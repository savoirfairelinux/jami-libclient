/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#include "daemonproxy.h"

namespace lrc
{

namespace authority
{

namespace daemon
{

void
addContact(const api::account::Info& owner, const std::string& contactUri)
{
    DaemonProxy::instance().addContact(owner.id, contactUri);
}

void
addContact(const api::account::Info& owner, const api::contact::Info& contactInfo)
{
    DaemonProxy::instance().addContact(owner.id, contactInfo.profileInfo.uri);
}

void
removeContact(const api::account::Info& owner, const std::string& contactUri, bool banned)
{
    DaemonProxy::instance().removeContact(owner.id, contactUri, banned);
}

bool
addContactFromPending(const api::account::Info& owner, const std::string& contactUri)
{
    return DaemonProxy::instance().acceptTrustRequest(owner.id, contactUri);
}

bool
discardFromPending(const api::account::Info& owner, const std::string& contactUri)
{
    return DaemonProxy::instance().discardTrustRequest(owner.id, contactUri);
}

} // namespace daemon

} // namespace authority

} // namespace lrc
