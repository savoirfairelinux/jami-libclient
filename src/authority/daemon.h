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
#pragma once

// Lrc
#include "api/account.h"
#include "api/contact.h"
#include "dbus/configurationmanager.h"

namespace lrc
{

namespace authority
{

namespace daemon
{
/**
 * Ask the daemon to add contact to the daemon.
 * @param owner
 * @param contactUri
 */
void addContact(const api::account::Info& owner, const std::string& contactUri);
/**
 * Ask the daemon to add contact to the daemon.
 * @param owner
 * @param contactInfo
 */
void addContact(const api::account::Info& owner, api::contact::Info& contactInfo);
/**
 * Ask the daemon to remove a contact and may ban it.
 * @param owner
 * @param contactInfo
 * @param banned
 */
void removeContact(const api::account::Info& owner, const std::string& contactUri, bool banned);
/**
 * Ask the daemon to add a contact from the pending list.
 * @param owner
 * @param contactUri
 */
void addContactFromPending(const api::account::Info& owner, const std::string& contactUri);
/**
 * Ask the daemon to discard a pending.
 * @param owner
 * @param contactUri
 */
void discardFromPending(const api::account::Info& owner, const std::string& contactUri);

} // namespace daemon

} // namespace authority

} // namespace lrc
