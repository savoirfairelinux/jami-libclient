/*
 *  Copyright (C) 2020 Savoir-faire Linux Inc.
 *
 *  Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */
#pragma once
#ifdef ENABLE_TEST
#include "../../test/mocks/conversationmanager_mock.h"
#else
#ifdef ENABLE_LIBWRAP
#include "../qtwrapper/conversationmanager_wrap.h"
#else
#include "conversationmanager_dbus_interface.h"
#include <QDBusPendingReply>
#endif
#endif
#include <typedefs.h>

namespace ConversationManager {

/// Singleton to access dbus "ConversationManager" interface
LIB_EXPORT ConversationManagerInterface& instance();

} // namespace ConversationManager
