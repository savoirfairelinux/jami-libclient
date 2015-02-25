/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                           *
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
#include "pixmapmanipulationdelegate.h"

#include <useractionmodel.h>

#include <QtCore/QSize>

PixmapManipulationDelegate* PixmapManipulationDelegate::m_spInstance = new PixmapManipulationDelegate();

PixmapManipulationDelegate::PixmapManipulationDelegate() {
   m_spInstance = this;
}

QVariant PixmapManipulationDelegate::contactPhoto(Person* c, const QSize& size, bool displayPresence)
{
   Q_UNUSED(c)
   Q_UNUSED(size)
   Q_UNUSED(displayPresence)
   return QVariant();
}

QVariant PixmapManipulationDelegate::numberCategoryIcon(const QVariant& p, const QSize& size, bool displayPresence, bool isPresent)
{
   Q_UNUSED(p)
   Q_UNUSED(size)
   Q_UNUSED(displayPresence)
   Q_UNUSED(isPresent)
   return QVariant();
}

QVariant PixmapManipulationDelegate::callPhoto(Call* c, const QSize& size, bool displayPresence)
{
   Q_UNUSED(c)
   Q_UNUSED(size)
   Q_UNUSED(displayPresence)
   return QVariant();
}

QVariant PixmapManipulationDelegate::callPhoto(const ContactMethod* c, const QSize& size, bool displayPresence)
{
   Q_UNUSED(c)
   Q_UNUSED(size)
   Q_UNUSED(displayPresence)
   return QVariant();
}

PixmapManipulationDelegate* PixmapManipulationDelegate::instance()
{
   return m_spInstance;
}

QVariant PixmapManipulationDelegate::serurityIssueIcon(const QModelIndex& index)
{
   Q_UNUSED(index)
   return QVariant();
}

QByteArray PixmapManipulationDelegate::toByteArray(const QVariant& pxm)
{
   Q_UNUSED(pxm)
   return QByteArray();
}

QVariant PixmapManipulationDelegate::profilePhoto(const QByteArray& data, const QString& type)
{
   Q_UNUSED(data)
   Q_UNUSED(type)
   return QVariant();
}

QVariant PixmapManipulationDelegate::userActionIcon(const UserActionElement& state) const
{
   Q_UNUSED(state)
   return QVariant();
}
