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
#include "contactmethodselectordelegate.h"
#include "itemmodelstateserializationdelegate.h"
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

class DelegateManagerPrivate
{
public:
    std::unique_ptr<AccountListColorDelegate>            m_accountListColorDelegate;
    std::unique_ptr<ContactMethodSelectorDelegate>       m_contactMethodSelectorDelegate;
    std::unique_ptr<ItemModelStateSerializationDelegate> m_itemModelStateSerializationDelegate;
    std::unique_ptr<PixmapManipulationDelegate>          m_pixmapManipulationDelegate;
    std::unique_ptr<PresenceSerializationDelegate>       m_presenceSerializationDelegate;
    std::unique_ptr<ProfilePersisterDelegate>            m_profilePersisterDelegate;
    std::unique_ptr<ShortcutDelegate>                    m_shortcutDelegate;
};

DelegateManager::DelegateManager() : d_ptr(new DelegateManagerPrivate())
{

}

DelegateManager::~DelegateManager()
{
    delete d_ptr;
}

AccountListColorDelegate*
DelegateManager::getAccountListColorDelegate()
{
    if (!d_ptr->m_accountListColorDelegate)
        d_ptr->m_accountListColorDelegate.reset(new AccountListColorDelegate);
    return d_ptr->m_accountListColorDelegate.get();
}

void
DelegateManager::setAccountListColorDelegate(AccountListColorDelegate* delegate)
{
    d_ptr->m_accountListColorDelegate.reset(delegate);
}

/**
 * The LRC implementation of this delegate is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ContactMethodSelectorDelegate*
DelegateManager::getContactMethodSelectorDelegate()
{
    return d_ptr->m_contactMethodSelectorDelegate.get();
}

void
DelegateManager::setContactMethodSelectorDelegate(ContactMethodSelectorDelegate* delegate)
{
    d_ptr->m_contactMethodSelectorDelegate.reset(delegate);
}

/**
 * The LRC implementation of this class is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ItemModelStateSerializationDelegate*
DelegateManager::getItemModelStateSerializationDelegate()
{
    return d_ptr->m_itemModelStateSerializationDelegate.get();
}

void
DelegateManager::setItemModelStateSerializationDelegate(ItemModelStateSerializationDelegate* delegate)
{
    d_ptr->m_itemModelStateSerializationDelegate.reset(delegate);
}

PixmapManipulationDelegate*
DelegateManager::getPixmapManipulationDelegate()
{
    if (!d_ptr->m_pixmapManipulationDelegate)
        d_ptr->m_pixmapManipulationDelegate.reset(new PixmapManipulationDelegate);
    return d_ptr->m_pixmapManipulationDelegate.get();
}

void
DelegateManager::setPixmapManipulationDelegate(PixmapManipulationDelegate* delegate)
{
    d_ptr->m_pixmapManipulationDelegate.reset(delegate);
}

PresenceSerializationDelegate*
DelegateManager::getPresenceSerializationDelegate()
{
    if (!d_ptr->m_presenceSerializationDelegate)
        d_ptr->m_presenceSerializationDelegate.reset(new PresenceSerializationDelegate);
    return d_ptr->m_presenceSerializationDelegate.get();
}

void
DelegateManager::setPresenceSerializationDelegate(PresenceSerializationDelegate* delegate)
{
    d_ptr->m_presenceSerializationDelegate.reset(delegate);

    /* run the load method to preserve the previous behaviour of the delegate */
    d_ptr->m_presenceSerializationDelegate->load();
}

/**
 * The LRC implementation of this delegate is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ProfilePersisterDelegate*
DelegateManager::getProfilePersisterDelegate()
{
    return d_ptr->m_profilePersisterDelegate.get();
}

void
DelegateManager::setProfilePersisterDelegate(ProfilePersisterDelegate* delegate)
{
    d_ptr->m_profilePersisterDelegate.reset(delegate);
}

ShortcutDelegate*
DelegateManager::getShortcutDelegate()
{
    if (!d_ptr->m_shortcutDelegate)
        d_ptr->m_shortcutDelegate.reset(new ShortcutDelegate);
    return d_ptr->m_shortcutDelegate.get();
}

void
DelegateManager::setShortcutDelegate(ShortcutDelegate* delegate)
{
    d_ptr->m_shortcutDelegate.reset(delegate);
}
