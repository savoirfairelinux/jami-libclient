/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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
#include "alsapluginmodel.h"

//Qt
#include <QtCore/QItemSelectionModel>

//Ring
#include "dbus/configurationmanager.h"

class AlsaPluginModelPrivate final : public QObject
{
   Q_OBJECT
public:
   AlsaPluginModelPrivate(Audio::AlsaPluginModel* parent);
   QStringList m_lDeviceList;
   mutable QItemSelectionModel* m_pSelectionModel;

private:
   Audio::AlsaPluginModel* q_ptr;

public Q_SLOTS:
   void setCurrentPlugin(const QModelIndex& idx);
};

AlsaPluginModelPrivate::AlsaPluginModelPrivate(Audio::AlsaPluginModel* parent) : q_ptr(parent),
m_pSelectionModel(nullptr)
{

}

///Constructor
Audio::AlsaPluginModel::AlsaPluginModel(const QObject* parent) : QAbstractListModel(const_cast<QObject*>(parent)),
d_ptr(new AlsaPluginModelPrivate(this))
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   d_ptr->m_lDeviceList = configurationManager.getAudioPluginList();
}

///Destructor
Audio::AlsaPluginModel::~AlsaPluginModel()
{

}

QHash<int,QByteArray> Audio::AlsaPluginModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

///Re-implement QAbstractListModel data
QVariant Audio::AlsaPluginModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid())
      return QVariant();
   switch(role) {
      case Qt::DisplayRole:
         return d_ptr->m_lDeviceList[index.row()];
   };
   return QVariant();
}

///Re-implement QAbstractListModel rowCount
int Audio::AlsaPluginModel::rowCount( const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;
   return d_ptr->m_lDeviceList.size();
}

///Re-implement QAbstractListModel flags
Qt::ItemFlags Audio::AlsaPluginModel::flags( const QModelIndex& index ) const
{
   Q_UNUSED(index)
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

///Setting data is disabled
bool Audio::AlsaPluginModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QItemSelectionModel* Audio::AlsaPluginModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<Audio::AlsaPluginModel*>(this));

      d_ptr->m_pSelectionModel->setCurrentIndex(currentPlugin(), QItemSelectionModel::ClearAndSelect);

      connect(d_ptr->m_pSelectionModel, &QItemSelectionModel::currentChanged, d_ptr.data(), &AlsaPluginModelPrivate::setCurrentPlugin);
   }

   return d_ptr->m_pSelectionModel;
}

///Return the current index
QModelIndex Audio::AlsaPluginModel::currentPlugin() const
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   const int idx = d_ptr->m_lDeviceList.indexOf(configurationManager.getCurrentAudioOutputPlugin());
   qDebug() << "Invalid current audio plugin";
   if (idx == -1)
      return QModelIndex();
   else
      return index(idx,0,QModelIndex());
}

///Set the current index
void AlsaPluginModelPrivate::setCurrentPlugin(const QModelIndex& idx)
{
   if (!idx.isValid())
      return;

   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   configurationManager.setAudioPlugin(m_lDeviceList[idx.row()]);
}

///Reload to current daemon state
void Audio::AlsaPluginModel::reload()
{
   const int currentRow = selectionModel()->currentIndex().row();

   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   beginResetModel();
   d_ptr->m_lDeviceList = configurationManager.getAudioPluginList();
   endResetModel();
   emit layoutChanged();
   emit dataChanged(index(0,0),index(d_ptr->m_lDeviceList.size()-1,0));

   // Restore the selection
   d_ptr->m_pSelectionModel->setCurrentIndex(index(currentRow,0), QItemSelectionModel::ClearAndSelect);

}

#include <alsapluginmodel.moc>
