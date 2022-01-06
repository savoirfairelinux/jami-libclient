/****************************************************************************
 *    Copyright (C) 2015-2022 Savoir-faire Linux Inc.                       *
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
#include "globalinstances.h"

#include <memory>

#include "interfaces/dbuserrorhandleri.h"
#include "interfaces/pixmapmanipulatori.h"

#include "dbuserrorhandlerdefault.h"
#include "pixmapmanipulatordefault.h"

namespace GlobalInstances {

struct InstanceManager
{
    std::unique_ptr<Interfaces::DBusErrorHandlerI> m_dBusErrorHandler;
    std::unique_ptr<Interfaces::PixmapManipulatorI> m_pixmapManipulator;
};

static InstanceManager&
instanceManager()
{
    static std::unique_ptr<InstanceManager> manager {new InstanceManager};
    return *manager.get();
}
/**
 * LRC does not provide a default implementation of this interface, thus an exception will be thrown
 * if this getter is called without an instance being set by the client
 */

Interfaces::DBusErrorHandlerI&
dBusErrorHandler()
{
    if (!instanceManager().m_dBusErrorHandler)
        instanceManager().m_dBusErrorHandler.reset(new Interfaces::DBusErrorHandlerDefault);
    return *instanceManager().m_dBusErrorHandler;
}

void
setDBusErrorHandler(std::unique_ptr<Interfaces::DBusErrorHandlerI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_dBusErrorHandler = std::move(instance);
}

/**
 * LRC does not provide a default implementation of this interface, thus an exception will be thrown
 * if this getter is called without an instance being set by the client
 */

Interfaces::PixmapManipulatorI&
pixmapManipulator()
{
    if (!instanceManager().m_pixmapManipulator)
        instanceManager().m_pixmapManipulator.reset(new Interfaces::PixmapManipulatorDefault);
    return *instanceManager().m_pixmapManipulator.get();
}

void
setPixmapManipulator(std::unique_ptr<Interfaces::PixmapManipulatorI> instance)
{
    // do not allow empty pointers
    if (!instance) {
        qWarning() << "ignoring empty unique_ptr";
        return;
    }
    instanceManager().m_pixmapManipulator = std::move(instance);
}

/*
 * This API have some advantage over a more "explicit" one
 * 1) It treat interfaces as class instead of as objects, making conceptual sense
 * 2) It remove the boilerplate code related to creating the unique_ptr away from
 *    the client
 * 3) It offer a transparent entry point for interface without having to
 *    extend the API when adding new interfaces
 * 4) It mimic the addCollection interface, making the API more consistent. It
 *    also does so without the tick layer of black magic used in the Media and
 *    collection APIs.
 */
#define REGISTER_INTERFACE(I, m) \
    void setInterfaceInternal(I* i) { instanceManager().m = std::unique_ptr<I>(i); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"

REGISTER_INTERFACE(Interfaces::DBusErrorHandlerI, m_dBusErrorHandler)
REGISTER_INTERFACE(Interfaces::PixmapManipulatorI, m_pixmapManipulator)

#pragma GCC diagnostic pop

#undef REGISTER_INTERFACE

} // namespace GlobalInstances
