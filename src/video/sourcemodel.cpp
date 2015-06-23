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
#include "sourcemodel.h"
#include <QtCore/QUrl>
#include <QtCore/QCoreApplication>
#include "../dbus/videomanager.h"
#include "devicemodel.h"

namespace Video {
class SourceModelPrivate
{
public:
   SourceModelPrivate();

   //Constants
   class ProtocolPrefix {
   public:
      constexpr static const char* NONE    = ""          ;
      constexpr static const char* DISPLAY = "display://";
      constexpr static const char* FILE    = "file://"   ;
      constexpr static const char* CAMERA  = "camera://"   ;
   };

   struct Display {
      Display() : rect(0,0,0,0),index(0){}
      QRect rect;
      int index ; /* X11 display ID, usually 0 */
   };

   QUrl m_CurrentFile;
   Display m_Display;
   int m_CurrentSelection;
};
}

Video::SourceModelPrivate::SourceModelPrivate() : m_CurrentSelection(-1)
{

}

Video::SourceModel* Video::SourceModel::m_spInstance = nullptr;

Video::SourceModel::SourceModel() : QAbstractListModel(QCoreApplication::instance()),
d_ptr(new Video::SourceModelPrivate())
{
   d_ptr->m_Display.rect = QRect(0,0,0,0);
}

Video::SourceModel::~SourceModel()
{
   delete d_ptr;
}

Video::SourceModel* Video::SourceModel::instance()
{
   if (!m_spInstance)
      m_spInstance = new Video::SourceModel();
   return m_spInstance;
}

QHash<int,QByteArray> Video::SourceModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }
   return roles;
}

QVariant Video::SourceModel::data( const QModelIndex& index, int role ) const
{
   switch (index.row()) {
      case ExtendedDeviceList::NONE:
         switch(role) {
            case Qt::DisplayRole:
               return tr("NONE");
         };
         break;
      case ExtendedDeviceList::SCREEN:
         switch(role) {
            case Qt::DisplayRole:
               return tr("SCREEN");
         };
         break;
      case ExtendedDeviceList::FILE:
         switch(role) {
            case Qt::DisplayRole:
               return tr("FILE");
         };
         break;
      default:
         return Video::DeviceModel::instance()->data(Video::DeviceModel::instance()->index(index.row()-ExtendedDeviceList::COUNT__,0),role);
   };
   return QVariant();
}

int Video::SourceModel::rowCount( const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return Video::DeviceModel::instance()->rowCount() + ExtendedDeviceList::COUNT__;
}

Qt::ItemFlags Video::SourceModel::flags( const QModelIndex& idx ) const
{
   switch (idx.row()) {
      case ExtendedDeviceList::NONE  :
      case ExtendedDeviceList::SCREEN:
      case ExtendedDeviceList::FILE  :
         return QAbstractItemModel::flags(idx) | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
         break;
      default:
         return Video::DeviceModel::instance()->flags(Video::DeviceModel::instance()->index(idx.row()-ExtendedDeviceList::COUNT__,0));
   };
}

bool Video::SourceModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

void Video::SourceModel::switchTo(const QModelIndex& idx)
{
   switchTo(idx.row());
}

///This model is designed for "live" switching rather than configuration
void Video::SourceModel::switchTo(const int idx)
{
   switch (idx) {
      case ExtendedDeviceList::NONE:
         DBus::VideoManager::instance().switchInput(Video::SourceModelPrivate::ProtocolPrefix::NONE);
         break;
      case ExtendedDeviceList::SCREEN:
         DBus::VideoManager::instance().switchInput( QString(Video::SourceModelPrivate::ProtocolPrefix::DISPLAY)+QString(":%1+%2,%3 %4x%5")
            .arg(d_ptr->m_Display.index)
            .arg(d_ptr->m_Display.rect.x())
            .arg(d_ptr->m_Display.rect.y())
            .arg(d_ptr->m_Display.rect.width())
            .arg(d_ptr->m_Display.rect.height()));
         break;
      case ExtendedDeviceList::FILE:
         DBus::VideoManager::instance().switchInput(
            !d_ptr->m_CurrentFile.isEmpty()?+Video::SourceModelPrivate::ProtocolPrefix::FILE+d_ptr->m_CurrentFile.toLocalFile():Video::SourceModelPrivate::ProtocolPrefix::NONE
         );
         break;
      default:
         DBus::VideoManager::instance().switchInput(Video::SourceModelPrivate::ProtocolPrefix::CAMERA +
            Video::DeviceModel::instance()->index(idx-ExtendedDeviceList::COUNT__,0).data(Qt::DisplayRole).toString());
         break;
   };
   d_ptr->m_CurrentSelection = (ExtendedDeviceList) idx;
}

void Video::SourceModel::switchTo(Video::Device* device)
{
   DBus::VideoManager::instance().switchInput(Video::SourceModelPrivate::ProtocolPrefix::CAMERA + device->id());
}

Video::Device* Video::SourceModel::deviceAt(const QModelIndex& idx) const
{
   if (!idx.isValid()) return nullptr;
   switch (idx.row()) {
      case ExtendedDeviceList::NONE:
      case ExtendedDeviceList::SCREEN:
      case ExtendedDeviceList::FILE:
         return nullptr;
      default:
         return Video::DeviceModel::instance()->devices()[idx.row()-ExtendedDeviceList::COUNT__];
   };
}

int Video::SourceModel::activeIndex() const
{
   if (d_ptr->m_CurrentSelection == -1) {
      return ExtendedDeviceList::COUNT__ + Video::DeviceModel::instance()->activeIndex();
   }
   return d_ptr->m_CurrentSelection;
}

void Video::SourceModel::setFile(const QUrl& url)
{
   d_ptr->m_CurrentFile = url;
   switchTo(ExtendedDeviceList::FILE);
}

void Video::SourceModel::setDisplay(int index, QRect rect)
{
   d_ptr->m_Display.index  = index ;
   d_ptr->m_Display.rect   = rect  ;
   switchTo(ExtendedDeviceList::SCREEN);
}
