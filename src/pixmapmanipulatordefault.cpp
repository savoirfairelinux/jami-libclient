/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-faire Linux                          *
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

//Qt
#include <QtCore/QSize>
#include <QtCore/QVariant>
#include <QtCore/QModelIndex>

//Ring
#include <useractionmodel.h>

namespace Interfaces {

QVariant PixmapManipulatorDefault::contactPhoto(Person* c, const QSize& size, bool displayPresence)
{
    Q_UNUSED(c)
    Q_UNUSED(size)
    Q_UNUSED(displayPresence)
    return QVariant();
}

QVariant PixmapManipulatorDefault::numberCategoryIcon(const QVariant& p, const QSize& size, bool displayPresence, bool isPresent)
{
    Q_UNUSED(p)
    Q_UNUSED(size)
    Q_UNUSED(displayPresence)
    Q_UNUSED(isPresent)
    return QVariant();
}

QVariant PixmapManipulatorDefault::callPhoto(Call* c, const QSize& size, bool displayPresence)
{
    Q_UNUSED(c)
    Q_UNUSED(size)
    Q_UNUSED(displayPresence)
    return QVariant();
}

QVariant PixmapManipulatorDefault::callPhoto(const ContactMethod* c, const QSize& size, bool displayPresence)
{
    Q_UNUSED(c)
    Q_UNUSED(size)
    Q_UNUSED(displayPresence)
    return QVariant();
}

QVariant PixmapManipulatorDefault::securityIssueIcon(const QModelIndex& index)
{
    Q_UNUSED(index)
    return QVariant();
}

QByteArray PixmapManipulatorDefault::toByteArray(const QVariant& pxm)
{
    Q_UNUSED(pxm)
    return QByteArray();
}

QVariant PixmapManipulatorDefault::personPhoto(const QByteArray& data, const QString& type)
{
    Q_UNUSED(data)
    Q_UNUSED(type)
    return QVariant();
}

QVariant PixmapManipulatorDefault::userActionIcon(const UserActionElement& state) const
{
    Q_UNUSED(state)
    return QVariant();
}

QVariant PixmapManipulatorDefault::collectionIcon(const CollectionInterface* interface, PixmapManipulatorI::CollectionIconHint hint) const
{
    Q_UNUSED(interface)
    Q_UNUSED(hint)
    return QVariant();
}

QVariant PixmapManipulatorDefault::securityLevelIcon(const SecurityEvaluationModel::SecurityLevel level) const
{
    Q_UNUSED(level)
    return QVariant();
}

QVariant PixmapManipulatorDefault::historySortingCategoryIcon(const CategorizedHistoryModel::SortedProxy::Categories cat) const
{
    Q_UNUSED(cat)
    return QVariant();
}

QVariant PixmapManipulatorDefault::contactSortingCategoryIcon(const CategorizedContactModel::SortedProxy::Categories cat) const
{
    Q_UNUSED(cat)
    return QVariant();
}

} // namespace Interfaces
