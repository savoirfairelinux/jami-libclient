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
#include "instances.h"

#include <memory>

#include "accountlistcolorizeri.h"
#include "accountlistcolorizerdefault.h"
#include "contactmethodselectori.h"
#include "itemmodelstateserializeri.h"
#include "pixmapmanipulatori.h"
#include "pixmapmanipulatordefault.h"
#include "presenceserializeri.h"
#include "presenceserializerdefault.h"
#include "profilepersisteri.h"
#include "shortcutcreatori.h"
#include "shortcutcreatordefault.h"

namespace Interfaces {

class InstanceManager
{
public:
    std::unique_ptr<AccountListColorizerI>     m_accountListColorizer;
    std::unique_ptr<ContactMethodSelectorI>    m_contactMethodSelector;
    std::unique_ptr<ItemModelStateSerializerI> m_itemModelStateSerializer;
    std::unique_ptr<PixmapManipulatorI>        m_pixmapManipulator;
    std::unique_ptr<PresenceSerializerI>       m_presenceSerializer;
    std::unique_ptr<ProfilePersisterI>         m_profilePersister;
    std::unique_ptr<ShortcutCreatorI>          m_shortcutCreator;
};

static InstanceManager&
instanceManager()
{
    static std::unique_ptr<InstanceManager> manager{new InstanceManager};
    return *manager.get();
}

AccountListColorizerI&
accountListColorizer()
{
    if (!instanceManager().m_accountListColorizer)
        instanceManager().m_accountListColorizer.reset(new AccountListColorizerDefault);
    return *instanceManager().m_accountListColorizer.get();
}

void
setAccountListColorizer(std::unique_ptr<AccountListColorizerI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_accountListColorizer = std::move(instance);
}

/**
 * LRC does not provide a default implementation of this interface, thus an exception will be thrown
 * if this getter is called without an instance being set by the client
 */
ContactMethodSelectorI&
contactMethodSelector()
{
    if (!instanceManager().m_contactMethodSelector)
        throw "no instance of ContactMethodSelector available";
    return *instanceManager().m_contactMethodSelector.get();
}

void
setContactMethodSelector(std::unique_ptr<ContactMethodSelectorI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_contactMethodSelector = std::move(instance);
}

/**
 * LRC does not provide a default implementation of this interface, thus an exception will be thrown
 * if this getter is called without an instance being set by the client
 */
ItemModelStateSerializerI&
itemModelStateSerializer()
{
    if (!instanceManager().m_itemModelStateSerializer)
        throw "no instance of ItemModelStateSerializer available";
    return *instanceManager().m_itemModelStateSerializer.get();
}

void
setItemModelStateSerializer(std::unique_ptr<ItemModelStateSerializerI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_itemModelStateSerializer = std::move(instance);
}

PixmapManipulatorI&
pixmapManipulator()
{
    if (!instanceManager().m_pixmapManipulator)
        instanceManager().m_pixmapManipulator.reset(new PixmapManipulatorDefault);
    return *instanceManager().m_pixmapManipulator.get();
}

void
setPixmapManipulator(std::unique_ptr<PixmapManipulatorI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_pixmapManipulator = std::move(instance);
}

PresenceSerializerI&
presenceSerializer()
{
    if (!instanceManager().m_presenceSerializer)
        instanceManager().m_presenceSerializer.reset(new PresenceSerializerDefault);
    return *instanceManager().m_presenceSerializer.get();
}

void
setPresenceSerializer(std::unique_ptr<PresenceSerializerI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_presenceSerializer = std::move(instance);
}

/**
 * TODO: LRC has a default implementation of this interface; however profiles are still in an
 * experimental state, so this getter will throw an exception unless an instance is set by the
 * client
 */
ProfilePersisterI&
profilePersister()
{
    if (!instanceManager().m_profilePersister)
        throw "no instance of ProfilePersister available";
    return *instanceManager().m_profilePersister.get();
}

void
setProfilePersister(std::unique_ptr<ProfilePersisterI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_profilePersister = std::move(instance);
}

ShortcutCreatorI&
shortcutCreator()
{
    if (!instanceManager().m_shortcutCreator)
        instanceManager().m_shortcutCreator.reset(new ShortcutCreatorDefault);
    return *instanceManager().m_shortcutCreator.get();
}

void
setShortcutCreatorI(std::unique_ptr<ShortcutCreatorI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_shortcutCreator = std::move(instance);
}

} // namespace Interfaces
