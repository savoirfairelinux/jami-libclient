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
#ifndef DELEGATEMANAGER_H
#define DELEGATEMANAGER_H

#include <typedefs.h>

#include <memory>

class AccountListColorDelegate;
class ItemModelStateSerializationDelegate;
class ContactMethodSelectorDelegate;
class PixmapManipulationDelegate;
class PresenceSerializationDelegate;
class ProfilePersisterDelegate;
class ShortcutDelegate;

/**
 * Manager to handle instances of each type of delegate. There is maximum one instance of each type
 * of delegate in the manager and the manager is responsible for handling the lifecycle of the
 * delegate object.
 *
 * Note that certain delegates only have an abstract implementation in LRC, thus the manager will
 * return a nullptr for these delegates if no client implementation has been set.
 *
 * Use getDelegateManager() to get the global instance of this object.
 */
class LIB_EXPORT DelegateManager {
public:
    AccountListColorDelegate* getAccountListColorDelegate();
    void setAccountListColorDelegate(AccountListColorDelegate* delegate);

    /**
     * The LRC implementation of this class is abstract, thus this will return a nullptr if no instance
     * is set by a client
     */
    ContactMethodSelectorDelegate* getContactMethodSelectorDelegate();
    void setContactMethodSelectorDelegate(ContactMethodSelectorDelegate* delegate);

    /**
     * The LRC implementation of this class is abstract, thus this will return a nullptr if no instance
     * is set by a client
     */
    ItemModelStateSerializationDelegate* getItemModelStateSerializationDelegate();
    void setItemModelStateSerializationDelegate(ItemModelStateSerializationDelegate* delegate);

    PixmapManipulationDelegate* getPixmapManipulationDelegate();
    void setPixmapManipulationDelegate(PixmapManipulationDelegate* delegate);

    PresenceSerializationDelegate* getPresenceSerializationDelegate();
    void setPresenceSerializationDelegate(PresenceSerializationDelegate* delegate);

    /**
     * The LRC implementation of this class is abstract, thus this will return a nullptr if no instance
     * is set by a client
     */
    ProfilePersisterDelegate* getProfilePersisterDelegate();
    void setProfilePersisterDelegate(ProfilePersisterDelegate* delegate);

    ShortcutDelegate* getShortcutDelegate();
    void setShortcutDelegate(ShortcutDelegate* delegate);

private:
    std::unique_ptr<AccountListColorDelegate>            m_accountListColorDelegate;
    std::unique_ptr<ContactMethodSelectorDelegate>       m_contactMethodSelectorDelegate;
    std::unique_ptr<ItemModelStateSerializationDelegate> m_itemModelStateSerializationDelegate;
    std::unique_ptr<PixmapManipulationDelegate>          m_pixmapManipulationDelegate;
    std::unique_ptr<PresenceSerializationDelegate>       m_presenceSerializationDelegate;
    std::unique_ptr<ProfilePersisterDelegate>            m_profilePersisterDelegate;
    std::unique_ptr<ShortcutDelegate>                    m_shortcutDelegate;
};

/**
 * This should be used to get the global instance of the DelegateManager by the client(s) and by LRC
 */
std::shared_ptr<DelegateManager> getDelegateManager();

#endif // DELEGATEMANAGER_H
