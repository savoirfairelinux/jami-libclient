/****************************************************************************
 *    Copyright (C) 2013-2022 Savoir-faire Linux Inc.                       *
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
#include "pixmapmanipulatordefault.h"

// Qt
#include <QtCore/QSize>
#include <QtCore/QVariant>
#include <QtCore/QModelIndex>

// LRC
#include "api/account.h"
#include "api/conversation.h"

namespace Interfaces {

QVariant
PixmapManipulatorDefault::numberCategoryIcon(const QVariant& p,
                                             const QSize& size,
                                             bool displayPresence,
                                             bool isPresent)
{
    Q_UNUSED(p)
    Q_UNUSED(size)
    Q_UNUSED(displayPresence)
    Q_UNUSED(isPresent)
    return QVariant();
}

QVariant
PixmapManipulatorDefault::conversationPhoto(const lrc::api::conversation::Info& conversation,
                                            const lrc::api::account::Info& accountInfo,
                                            const QSize& size,
                                            bool displayPresence)
{
    Q_UNUSED(conversation)
    Q_UNUSED(accountInfo)
    Q_UNUSED(size)
    Q_UNUSED(displayPresence)
    return QVariant();
}

QByteArray
PixmapManipulatorDefault::toByteArray(const QVariant& pxm)
{
    Q_UNUSED(pxm)
    return QByteArray();
}

QVariant
PixmapManipulatorDefault::personPhoto(const QByteArray& data, const QString& type)
{
    Q_UNUSED(data)
    Q_UNUSED(type)
    return QVariant();
}

QVariant
PixmapManipulatorDefault::userActionIcon(const UserActionElement& state) const
{
    Q_UNUSED(state)
    return QVariant();
}

QVariant
PixmapManipulatorDefault::decorationRole(const QModelIndex& index)
{
    Q_UNUSED(index)
    return QVariant();
}

QVariant
PixmapManipulatorDefault::decorationRole(const lrc::api::conversation::Info& conversation,
                                         const lrc::api::account::Info& accountInfo)
{
    Q_UNUSED(conversation)
    Q_UNUSED(accountInfo)
    return QVariant();
}

} // namespace Interfaces
