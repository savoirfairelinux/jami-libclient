/****************************************************************************
 *   Copyright (C) 2015 by Savoir-faire Linux                               *
 *   Author : Stepan Salenikovich <stepan.salenikovich@savoirfairelinux.com>*
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
#include "delegatemanager.h"

#include "accountlistcolordelegate.h"
#include "dbuserrordelegate.h"
#include "itemmodelstateserializationdelegate.h"
#include "contactmethodselectordelegate.h"
#include "pixmapmanipulationdelegate.h"
#include "presenceserializationdelegate.h"
#include "profilepersisterdelegate.h"
#include "shortcutdelegate.h"

std::shared_ptr<DelegateManager>
getDelegateManager()
{
    static auto manager = std::make_shared<DelegateManager>();
    return manager;
}

AccountListColorDelegate*
DelegateManager::getAccountListColorDelegate()
{
    if (!m_accountListColorDelegate)
        m_accountListColorDelegate.reset(new AccountListColorDelegate);
    return m_accountListColorDelegate.get();
}

void
DelegateManager::setAccountListColorDelegate(AccountListColorDelegate* delegate)
{
    m_accountListColorDelegate.reset(delegate);
}

/**
 * The LRC implementation of this delegate is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ContactMethodSelectorDelegate*
DelegateManager::getContactMethodSelectorDelegate()
{
    return m_contactMethodSelectorDelegate.get();
}

void
DelegateManager::setContactMethodSelectorDelegate(ContactMethodSelectorDelegate* delegate)
{
    m_contactMethodSelectorDelegate.reset(delegate);
}

/**
 * The LRC implementation of this class is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ItemModelStateSerializationDelegate*
DelegateManager::getItemModelStateSerializationDelegate()
{
    return m_itemModelStateSerializationDelegate.get();
}

void
DelegateManager::setItemModelStateSerializationDelegate(ItemModelStateSerializationDelegate* delegate)
{
    m_itemModelStateSerializationDelegate.reset(delegate);
}

PixmapManipulationDelegate*
DelegateManager::getPixmapManipulationDelegate()
{
    if (!m_pixmapManipulationDelegate)
        m_pixmapManipulationDelegate.reset(new PixmapManipulationDelegate);
    return m_pixmapManipulationDelegate.get();
}

void
DelegateManager::setPixmapManipulationDelegate(PixmapManipulationDelegate* delegate)
{
    m_pixmapManipulationDelegate.reset(delegate);
}

PresenceSerializationDelegate*
DelegateManager::getPresenceSerializationDelegate()
{
    if (!m_presenceSerializationDelegate)
        m_presenceSerializationDelegate.reset(new PresenceSerializationDelegate);
    return m_presenceSerializationDelegate.get();
}

void
DelegateManager::setPresenceSerializationDelegate(PresenceSerializationDelegate* delegate)
{
    m_presenceSerializationDelegate.reset(delegate);

    /* run the load method to preserve the previous behaviour of the delegate */
    m_presenceSerializationDelegate->load();
}

/**
 * The LRC implementation of this delegate is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ProfilePersisterDelegate*
DelegateManager::getProfilePersisterDelegate()
{
    return m_profilePersisterDelegate.get();
}

void
DelegateManager::setProfilePersisterDelegate(ProfilePersisterDelegate* delegate)
{
    m_profilePersisterDelegate.reset(delegate);
}

ShortcutDelegate*
DelegateManager::getShortcutDelegate()
{
    if (!m_shortcutDelegate)
        m_shortcutDelegate.reset(new ShortcutDelegate);
    return m_shortcutDelegate.get();
}

void
DelegateManager::setShortcutDelegate(ShortcutDelegate* delegate)
{
    m_shortcutDelegate.reset(delegate);
}
