/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
 *   Author : Alexandre Viau <alexandre.viau@savoirfairelinux.com>          *
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
#include "ringdevicemodel.h"

#include "dbus/configurationmanager.h"

//Qt
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QDirIterator>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>

//Ring daemon
#include <account_const.h>

//Ring
#include <account.h>
#include <private/account_p.h>

typedef void (RingDeviceModelPrivate::*RingDeviceModelPrivateFct)();

class RingDeviceModelPrivate
{
public:
   struct Lines {
      QString id;
   };

   RingDeviceModelPrivate(RingDeviceModel* q,Account* a);

   //Attributes
   Account*                   m_pAccount ;
   QVector<Lines*>            m_lines    ;
   RingDeviceModel*           q_ptr      ;

   void reload();
   void clearLines();

};

RingDeviceModelPrivate::RingDeviceModelPrivate(RingDeviceModel* q,Account* a) : q_ptr(q),m_pAccount(a)
{
}


void RingDeviceModelPrivate::clearLines()
{
    if (m_lines.size() > 0)
    {
       q_ptr->beginRemoveRows(QModelIndex(), 0, m_lines.size());
       for (int i = 0; i < m_lines.size(); i++)
           delete m_lines[i];
       m_lines.clear();
       q_ptr->endRemoveRows();
   }
}

void RingDeviceModelPrivate::reload()
{
    clearLines();

    const QStringList accountDevices = ConfigurationManager::instance().getKnownRingDevices(this->m_pAccount->id());

    for (int i = 0; i < accountDevices.size(); ++i) {
      RingDeviceModelPrivate::Lines* l = new RingDeviceModelPrivate::Lines();
      l->id = accountDevices[i];

      q_ptr->beginInsertRows(QModelIndex(),m_lines.size(), m_lines.size());
      m_lines << l;
      q_ptr->endInsertRows();
    }

}

RingDeviceModel::RingDeviceModel(Account* a) : QAbstractTableModel(a), d_ptr(new RingDeviceModelPrivate(this,a))
{
    d_ptr->reload();
}

RingDeviceModel::~RingDeviceModel()
{
   delete d_ptr;
}

QVariant RingDeviceModel::data( const QModelIndex& index, int role) const
{
   Q_UNUSED(role);

   if (!index.isValid())
      return QVariant();

   RingDeviceModelPrivate::Lines* l =  index.row() < d_ptr->m_lines.size() ?d_ptr->m_lines[index.row()] : nullptr;

   if (!l)
      return QVariant();

   switch (static_cast<RingDeviceModel::Columns>(index.column())) {
      case RingDeviceModel::Columns::ID:
         return QVariant(l->id);
   };

   return QVariant();
}

int RingDeviceModel::rowCount( const QModelIndex& parent) const
{
   return parent.isValid()?0:d_ptr->m_lines.size();
}

int RingDeviceModel::columnCount( const QModelIndex& parent) const
{
   return parent.isValid()?0:1;
}
