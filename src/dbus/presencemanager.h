/****************************************************************************
 *   Copyright (C) 2012-2018 Savoir-faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#ifdef ENABLE_TEST
 #include "../../test/mocks/presencemanager_mock.h"
#else
 #ifdef ENABLE_LIBWRAP
  #include "../qtwrapper/presencemanager_wrap.h"
 #else
  #include "presencemanager_dbus_interface.h"
  #include <QDBusPendingReply>
 #endif
#endif

#include <typedefs.h>

namespace PresenceManager {

PresenceManagerInterface& LIB_EXPORT instance();

}
