/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
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
#include "resolution.h"

//Ring
#include "../dbus/videomanager.h"
#include "channel.h"
#include "rate.h"
#include "device.h"

//Ring private
#include "../private/videodevice_p.h"
#include "../private/videorate_p.h"
#include "../private/videoresolution_p.h"

//Qt
#include <QtCore/QStringList>

Video::Resolution::Resolution(const QString& size, Video::Channel* chan)
: QAbstractListModel(chan),d_ptr(new VideoResolutionPrivate())
{
   Q_ASSERT(chan != nullptr);

   d_ptr->m_pChannel = chan;
   d_ptr->m_pCurrentRate = nullptr;

   if (size.split('x').size() == 2) {
      setWidth(size.split('x')[0].toInt());
      setHeight(size.split('x')[1].toInt());
   }
}

const QString Video::Resolution::name() const
{
   return QString::number(width())+'x'+QString::number(height());
}


QVariant Video::Resolution::data( const QModelIndex& index, int role) const
{
   if (index.isValid() && role == Qt::DisplayRole && index.row() < d_ptr->m_lValidRates.size()) {
      return d_ptr->m_lValidRates[index.row()]->name();
   }
   return QVariant();
}

int Video::Resolution::rowCount( const QModelIndex& parent) const
{
   return (parent.isValid())?0:d_ptr->m_lValidRates.size();
}

Qt::ItemFlags Video::Resolution::flags( const QModelIndex& idx) const
{
   if (idx.column() == 0)
      return QAbstractItemModel::flags(idx) | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   return QAbstractItemModel::flags(idx);
}

bool Video::Resolution::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}


const QList<Video::Rate*> Video::Resolution::validRates() const {
   return d_ptr->m_lValidRates;
}


bool Video::Resolution::setActiveRate(Video::Rate* rate) {
   if (!rate || (d_ptr->m_lValidRates.indexOf(rate) == -1)) {
      qWarning() << "Trying to set an invalid rate" << rate;
      return false;
   }
   d_ptr->m_pCurrentRate = rate;
   d_ptr->m_pChannel->device()->save();
   return true;
}


bool Video::Resolution::setActiveRate(int idx)
{
   if (idx >= d_ptr->m_lValidRates.size() || idx < 0) return false;
   return setActiveRate(d_ptr->m_lValidRates[idx]);
}

Video::Rate* Video::Resolution::activeRate() const
{
   if (!d_ptr->m_pChannel) {
      qWarning() << "Trying to get the active rate of an unattached resolution";
      return nullptr;
   }
   if (!d_ptr->m_pCurrentRate && d_ptr->m_pChannel && d_ptr->m_pChannel->device()->isActive()) {
      VideoManagerInterface& interface = DBus::VideoManager::instance();
      const QString rate = QMap<QString,QString>(
         interface.getSettings(d_ptr->m_pChannel->device()->id()))[VideoDevicePrivate::PreferenceNames::RATE];
      foreach(Video::Rate* r, d_ptr->m_lValidRates) {
         if (r->name() == rate) {
            d_ptr->m_pCurrentRate = r;
            break;
         }
      }
   }
   if ((!d_ptr->m_pCurrentRate) && d_ptr->m_lValidRates.size())
      d_ptr->m_pCurrentRate = d_ptr->m_lValidRates[0];

   return d_ptr->m_pCurrentRate;
}

int Video::Resolution::relativeIndex() const
{
   return d_ptr->m_pChannel?d_ptr->m_pChannel->validResolutions().indexOf(const_cast<Video::Resolution*>(this)):-1;
}

int Video::Resolution::width() const
{
   return d_ptr->m_Size.width();
}

int Video::Resolution::height() const
{
   return d_ptr->m_Size.height();
}

QSize Video::Resolution::size() const
{
   return d_ptr->m_Size;
}

void Video::Resolution::setWidth(int width)
{
   d_ptr->m_Size.setWidth(width);
}

void Video::Resolution::setHeight(int height)
{
   d_ptr->m_Size.setHeight(height);
}
