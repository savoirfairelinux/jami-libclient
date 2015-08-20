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
#include "delegates.h"

#include <memory>

#include "accountlistcolordelegate.h"
#include "contactmethodselectordelegate.h"
#include "itemmodelstateserializationdelegate.h"
#include "pixmapmanipulationdelegate.h"
#include "presenceserializationdelegate.h"
#include "profilepersisterdelegate.h"
#include "shortcutdelegate.h"

namespace Delegates {

class DelegateManager
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

static DelegateManager*
getDelegateManager()
{
    static std::unique_ptr<DelegateManager> manager{new DelegateManager};
    return manager.get();
}

AccountListColorDelegate*
getAccountListColorDelegate()
{
    if (!getDelegateManager()->m_accountListColorDelegate)
        getDelegateManager()->m_accountListColorDelegate.reset(new AccountListColorDelegate);
    return getDelegateManager()->m_accountListColorDelegate.get();
}

void
setAccountListColorDelegate(AccountListColorDelegate* delegate)
{
    getDelegateManager()->m_accountListColorDelegate.reset(delegate);
}

/**
 * The LRC implementation of this delegate is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ContactMethodSelectorDelegate*
getContactMethodSelectorDelegate()
{
    return getDelegateManager()->m_contactMethodSelectorDelegate.get();
}

void
setContactMethodSelectorDelegate(ContactMethodSelectorDelegate* delegate)
{
    getDelegateManager()->m_contactMethodSelectorDelegate.reset(delegate);
}

/**
 * The LRC implementation of this class is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ItemModelStateSerializationDelegate*
getItemModelStateSerializationDelegate()
{
    return getDelegateManager()->m_itemModelStateSerializationDelegate.get();
}

void
setItemModelStateSerializationDelegate(ItemModelStateSerializationDelegate* delegate)
{
    getDelegateManager()->m_itemModelStateSerializationDelegate.reset(delegate);
}

PixmapManipulationDelegate*
getPixmapManipulationDelegate()
{
    if (!getDelegateManager()->m_pixmapManipulationDelegate)
        getDelegateManager()->m_pixmapManipulationDelegate.reset(new PixmapManipulationDelegate);
    return getDelegateManager()->m_pixmapManipulationDelegate.get();
}

void
setPixmapManipulationDelegate(PixmapManipulationDelegate* delegate)
{
    getDelegateManager()->m_pixmapManipulationDelegate.reset(delegate);
}

PresenceSerializationDelegate*
getPresenceSerializationDelegate()
{
    if (!getDelegateManager()->m_presenceSerializationDelegate)
        getDelegateManager()->m_presenceSerializationDelegate.reset(new PresenceSerializationDelegate);
    return getDelegateManager()->m_presenceSerializationDelegate.get();
}

void
setPresenceSerializationDelegate(PresenceSerializationDelegate* delegate)
{
    getDelegateManager()->m_presenceSerializationDelegate.reset(delegate);

    /* run the load method to preserve the previous behaviour of the delegate */
    getDelegateManager()->m_presenceSerializationDelegate->load();
}

/**
 * The LRC implementation of this delegate is abstract, thus this will return a nullptr if no instance
 * is set by a client
 */
ProfilePersisterDelegate*
getProfilePersisterDelegate()
{
    return getDelegateManager()->m_profilePersisterDelegate.get();
}

void
setProfilePersisterDelegate(ProfilePersisterDelegate* delegate)
{
    getDelegateManager()->m_profilePersisterDelegate.reset(delegate);
}

ShortcutDelegate*
getShortcutDelegate()
{
    if (!getDelegateManager()->m_shortcutDelegate)
        getDelegateManager()->m_shortcutDelegate.reset(new ShortcutDelegate);
    return getDelegateManager()->m_shortcutDelegate.get();
}

void
setShortcutDelegate(ShortcutDelegate* delegate)
{
    getDelegateManager()->m_shortcutDelegate.reset(delegate);
}

} // namespace Delegates
