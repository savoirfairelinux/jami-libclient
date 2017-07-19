/****************************************************************************
 *   Copyright (C) 2014-2017 Savoir-faire Linux                          *
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
#include <media_const.h>

namespace Video {
class SourceModelPrivate : public QObject
{
    Q_OBJECT
public:
    explicit SourceModelPrivate(SourceModel *parent);

    struct Display {
        Display() : rect(0,0,0,0),index(0){}
        QRect rect;
        int index ; /* X11 display ID, usually 0 */
    };

    QUrl m_CurrentFile;
    Display m_Display;
    int m_CurrentSelection;
    QString m_CurrentSelectionId; // if the current selection is a camera, store the device id here
    bool m_DeviceModelReloading;

private:
    SourceModel* q_ptr;

private Q_SLOTS:
    void devicesAboutToReload();
    void devicesReloaded();
};
}

Video::SourceModelPrivate::SourceModelPrivate(SourceModel *parent) : QObject(parent), q_ptr(parent),
m_CurrentSelection(-1), m_DeviceModelReloading(false)
{
    connect(&Video::DeviceModel::instance(), &QAbstractItemModel::modelAboutToBeReset, this, &SourceModelPrivate::devicesAboutToReload);
    connect(&Video::DeviceModel::instance(), &QAbstractItemModel::modelReset, this, &SourceModelPrivate::devicesReloaded);
}

Video::SourceModel::SourceModel(QObject* parent) : QAbstractListModel(parent),
d_ptr(new Video::SourceModelPrivate(this))
{
    d_ptr->m_Display.rect = QRect(0,0,0,0);

    // set the active device to the default one, if it exists
    auto deviceIdx = Video::DeviceModel::instance().activeIndex();
    if (deviceIdx > -1) {
        d_ptr->m_CurrentSelection = deviceIdx + ExtendedDeviceList::COUNT__;
        d_ptr->m_CurrentSelectionId = Video::DeviceModel::instance().activeDevice()->id();
    }
}

Video::SourceModel::~SourceModel()
{
   delete d_ptr;
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
         return Video::DeviceModel::instance().data(Video::DeviceModel::instance().index(index.row()-ExtendedDeviceList::COUNT__,0),role);
   };
   return QVariant();
}

int Video::SourceModel::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    return d_ptr->m_DeviceModelReloading ?
        ExtendedDeviceList::COUNT__ : Video::DeviceModel::instance().rowCount() + ExtendedDeviceList::COUNT__;
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
         return Video::DeviceModel::instance().flags(Video::DeviceModel::instance().index(idx.row()-ExtendedDeviceList::COUNT__,0));
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
   auto newIdx = idx > -1 ? idx : ExtendedDeviceList::NONE;
   QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
   switch (newIdx) {
      case ExtendedDeviceList::NONE:
         d_ptr->m_CurrentSelectionId = QString();
         VideoManager::instance().switchInput(DRing::Media::VideoProtocolPrefix::NONE);
         break;
      case ExtendedDeviceList::SCREEN:
         d_ptr->m_CurrentSelectionId = QString();
         VideoManager::instance().switchInput(QString("%1%2:%3+%4,%5 %6x%7")
                  .arg(DRing::Media::VideoProtocolPrefix::DISPLAY)
                  .arg(sep)
                  .arg(d_ptr->m_Display.index)
                  .arg(d_ptr->m_Display.rect.x())
                  .arg(d_ptr->m_Display.rect.y())
                  .arg(d_ptr->m_Display.rect.width())
                  .arg(d_ptr->m_Display.rect.height()));
         break;
      case ExtendedDeviceList::FILE:
         d_ptr->m_CurrentSelectionId = QString();
         VideoManager::instance().switchInput(
            !d_ptr->m_CurrentFile.isEmpty() ? QString("%1%2%3")
               .arg(DRing::Media::VideoProtocolPrefix::FILE)
               .arg(sep)
               .arg(d_ptr->m_CurrentFile.toLocalFile())
            : DRing::Media::VideoProtocolPrefix::NONE
         );
         break;
      default:
         d_ptr->m_CurrentSelectionId = Video::DeviceModel::instance().index(idx-ExtendedDeviceList::COUNT__,0).data(Qt::DisplayRole).toString();
         VideoManager::instance().switchInput(QString("%1%2%3")
            .arg(DRing::Media::VideoProtocolPrefix::CAMERA)
            .arg(sep)
            .arg(d_ptr->m_CurrentSelectionId));
         break;
   };
   d_ptr->m_CurrentSelection = newIdx;
}

///Set the index of the currently used source
void Video::SourceModel::setUsedIndex(QString &deviceStr)
{
    int idx = 0;
    //find out index here
    if (deviceStr.length() <= 0) {
        idx = ExtendedDeviceList::NONE;
    }
    else if (deviceStr.indexOf(DRing::Media::VideoProtocolPrefix::DISPLAY) == 0) {
        // Look for the display string into the incoming device string
        idx = ExtendedDeviceList::SCREEN;
    }
    else if (deviceStr.indexOf(DRing::Media::VideoProtocolPrefix::FILE) == 0) {
        idx = ExtendedDeviceList::FILE;
    }
    else if (deviceStr.indexOf(DRing::Media::VideoProtocolPrefix::CAMERA) == 0) {
        QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
        auto fullPrefix = QString("%1%2")
           .arg(DRing::Media::VideoProtocolPrefix::CAMERA)
           .arg(DRing::Media::VideoProtocolPrefix::SEPARATOR);
        Video::Device* dev = Video::DeviceModel::instance().getDevice(deviceStr.replace(fullPrefix,""));
        if (dev == nullptr) {
            // Device not found we dont know what camera is used
            idx = ExtendedDeviceList::NONE;
            return;
        }

        Video::DeviceModel::instance().setActive(dev);
        idx = ExtendedDeviceList::COUNT__ + Video::DeviceModel::instance().activeIndex();
    }
    else {
        idx = ExtendedDeviceList::NONE;
    }

    d_ptr->m_CurrentSelection = idx;
}

void Video::SourceModel::switchTo(Video::Device* device)
{
   switchTo(getDeviceIndex(device));
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
         return Video::DeviceModel::instance().devices()[idx.row()-ExtendedDeviceList::COUNT__];
   };
}

int Video::SourceModel::activeIndex() const
{
    /* its possible for the saved selection to be invalid if the device model is in the process
     * of being reloaded */
    return d_ptr->m_CurrentSelection >= rowCount() ? -1 : d_ptr->m_CurrentSelection;
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

int Video::SourceModel::getDeviceIndex(Video::Device* device)
{
    int index = Video::DeviceModel::instance().devices().indexOf(device);
    return index > -1 ? ExtendedDeviceList::COUNT__ + index : -1;
}

void Video::SourceModelPrivate::devicesAboutToReload()
{
    // are there any camera devices to reload?
    if (q_ptr->rowCount() > SourceModel::ExtendedDeviceList::COUNT__) {
        int first = SourceModel::ExtendedDeviceList::COUNT__;
        int last = q_ptr->rowCount() - 1;
        q_ptr->beginRemoveRows(QModelIndex(), first, last);
        /* we keep the current selection until the device model is reloaded, so we can try to select
         * the same one */
        m_DeviceModelReloading = true;
        q_ptr->removeRows(first, last);
    }
}

void Video::SourceModelPrivate::devicesReloaded()
{
    // insert rows if we have any devices
    if (Video::DeviceModel::instance().rowCount() > 0) {
        int first = SourceModel::ExtendedDeviceList::COUNT__;
        int last = SourceModel::ExtendedDeviceList::COUNT__ + Video::DeviceModel::instance().rowCount() - 1;
        q_ptr->beginInsertRows(QModelIndex(), first, last);
        m_DeviceModelReloading = false;
        if (m_CurrentSelection >= SourceModel::ExtendedDeviceList::COUNT__) {
            // the device order may have changed, try to get the same one as before
            if (auto device = Video::DeviceModel::instance().getDevice(m_CurrentSelectionId)) {
                m_CurrentSelection = q_ptr->getDeviceIndex(device);
            } else {
                m_CurrentSelectionId = QString();
                m_CurrentSelection = -1;
            }
        }
        q_ptr->insertRows(first, last);
    } else {
        m_DeviceModelReloading = false;
    }
}

#include <sourcemodel.moc>
