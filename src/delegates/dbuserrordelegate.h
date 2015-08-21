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
#ifndef DBUSERRORDELEGATE_H
#define DBUSERRORDELEGATE_H
#include <typedefs.h>

/**
 * Some clients may not have a nice way to generally handle exceptions event wide (ie: handle any
 * exception which may occur during an itteration or an event on the main loop). This delegate
 * gives them the option to implement another way to handle dbus errors.
 *
 * This implementation throws an exception with the given message.
 */
class LIB_EXPORT DBusErrorDelegate {
public:
    [[noreturn]] virtual void connectionError(const QString& error);
    [[noreturn]] virtual void invalidInterfaceError(const QString& error);
};

#endif // DBUSERRORDELEGATE_H
