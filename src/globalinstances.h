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
#pragma once

#include <typedefs.h>

#include <memory>

namespace Interfaces {
class AccountListColorizerI;
class ContactMethodSelectorI;
class DBusErrorHandlerI;
class ItemModelStateSerializerI;
class PixmapManipulatorI;
class PresenceSerializerI;
class ProfilePersisterI;
class ShortcutCreatorI;
} // namespace Interfaces

/**
 * Use these functions to get and set the global instance of the implementation of each interface.
 *
 * The setter functions demand an std::unique_ptr because they become the object owners.
 *
 * Note that certain interfaces do not have a default implementation in LRC, in this case the getter
 * function will throw an exception if no instance has been set by the client.
 */
namespace GlobalInstances {

Interfaces::AccountListColorizerI& accountListColorizer();
void setAccountListColorizer(std::unique_ptr<Interfaces::AccountListColorizerI> instance);

/**
 * LRC does not provide a default implementation of this interface, thus an exception will be thrown
 * if this getter is called without an instance being set by the client
 */
Interfaces::ContactMethodSelectorI& contactMethodSelector();
void setContactMethodSelector(std::unique_ptr<Interfaces::ContactMethodSelectorI> instance);

Interfaces::DBusErrorHandlerI& dBusErrorHandler();
void setDBusErrorHandler(std::unique_ptr<Interfaces::DBusErrorHandlerI> instance);

/**
 * LRC does not provide a default implementation of this interface, thus an exception will be thrown
 * if this getter is called without an instance being set by the client
 */
Interfaces::ItemModelStateSerializerI& itemModelStateSerializer();
void setItemModelStateSerializer(std::unique_ptr<Interfaces::ItemModelStateSerializerI> instance);

Interfaces::PixmapManipulatorI& pixmapManipulator();
void setPixmapManipulator(std::unique_ptr<Interfaces::PixmapManipulatorI> instance);

Interfaces::PresenceSerializerI& presenceSerializer();
void setPresenceSerializer(std::unique_ptr<Interfaces::PresenceSerializerI> instance);

/**
 * TODO: LRC has a default implementation of this interface; however profiles are still in an
 * experimental state, so this getter will throw an exception unless an instance is set by the
 * client
 */
Interfaces::ProfilePersisterI& profilePersister();
void setProfilePersister(std::unique_ptr<Interfaces::ProfilePersisterI> instance);

Interfaces::ShortcutCreatorI& shortcutCreator();
void setShortcutCreatorI(std::unique_ptr<Interfaces::ShortcutCreatorI> instance);


//Private use only
void setInterfaceInternal(Interfaces::AccountListColorizerI    *);
void setInterfaceInternal(Interfaces::ContactMethodSelectorI   *);
void setInterfaceInternal(Interfaces::DBusErrorHandlerI        *);
void setInterfaceInternal(Interfaces::ItemModelStateSerializerI*);
void setInterfaceInternal(Interfaces::PixmapManipulatorI       *);
void setInterfaceInternal(Interfaces::PresenceSerializerI      *);
void setInterfaceInternal(Interfaces::ProfilePersisterI        *);
void setInterfaceInternal(Interfaces::ShortcutCreatorI         *);

/**
 * Generic interface setter. This metamethod can set any type of interface
 * dynamically using variadic template, compile time type deduction and
 * the macro subsystem. Passing an invalid interface should trigger compile
 * time errors.
 *
 * The interface object is created internally and additional parameters
 * can be passed.
 */
template<class I, typename ...Ts>
void setInterface(Ts... args)
{
   try {
      auto i = new I(args...);
      setInterfaceInternal(i);
   }
   catch(void* e) { //TODO define some kind of object for errors like this
      qDebug() << "Interface could not be set";
   }
}

} // namespace GlobalInstances
