/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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
#include "bootstrapmodel.h"

//Ring daemon
#include <account_const.h>

//Ring
#include <account.h>
#include <private/account_p.h>

class BootstrapModelPrivate
{
public:
   struct Lines {
      QString hostname;
      int     port    ;
   };

   BootstrapModelPrivate(BootstrapModel* q,Account* a);

   //Helper
   bool save();

   //Attributes
   Account* m_pAccount;
   QVector<Lines*> m_lines;
   BootstrapModel* q_ptr;
};

BootstrapModelPrivate::BootstrapModelPrivate(BootstrapModel* q,Account* a) : q_ptr(q),m_pAccount(a)
{

}


bool BootstrapModelPrivate::save()
{
   QString ret;
   for(const Lines* l : m_lines) {
      if (!l->hostname.trimmed().isEmpty()) {
         if (ret.size())
            ret += ';';
         ret += l->hostname + (l->port > -1? ':'+QString::number(l->port):QString());
      }
   }

   //Clear empty lines
   bool val = true;
   for(int i=0;i<m_lines.size();i++) {
      Lines* l = m_lines[i];
      if (l->hostname.isEmpty() && l->port == -1) {
         q_ptr->beginRemoveRows(QModelIndex(),i,i);
         const int idx = m_lines.indexOf(l);
         if (idx >= 0)
            m_lines.removeAt(idx);
         q_ptr->endRemoveRows();
         val = false;
      }
   }

   m_pAccount->d_ptr->setAccountProperty(DRing::Account::ConfProperties::HOSTNAME,ret);
   return val;
}

BootstrapModel::BootstrapModel(Account* a) : QAbstractTableModel(a), d_ptr(new BootstrapModelPrivate(this,a))
{

   for(const QString& line : d_ptr->m_pAccount->hostname().split(';')) {
      const QStringList& fields = line.split(':');

      if (line.size() && fields.size() && !(fields[0].isEmpty() && (fields.size()-1 && fields[1].isEmpty()))) {
         BootstrapModelPrivate::Lines* l = new BootstrapModelPrivate::Lines();
         l->hostname = fields[0].trimmed();
         l->port     = fields.size()>1?fields[1].toInt():-1; //-1 == default

         d_ptr->m_lines << l;
      }
   }
}

BootstrapModel::~BootstrapModel()
{
   delete d_ptr;
}

QHash<int,QByteArray> BootstrapModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
   }*/
   return roles;
}

bool BootstrapModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   if (!index.isValid())
      return false;

   BootstrapModelPrivate::Lines* l = index.row() < d_ptr->m_lines.size() ? d_ptr->m_lines[index.row()] : nullptr;

   if (!(role == Qt::DisplayRole || role == Qt::EditRole))
      return false;

   if (!l) {
      l = new BootstrapModelPrivate::Lines();
      l->port = -1;
      beginInsertRows(QModelIndex(),d_ptr->m_lines.size()+1,d_ptr->m_lines.size()+1);
      d_ptr->m_lines << l;
      endInsertRows();
   }

   switch (index.column()) {
      case static_cast<int>(BootstrapModel::Columns::HOSTNAME):
         l->hostname = value.toString();
         if (d_ptr->save())
            emit dataChanged(index,index);
         break;
      case static_cast<int>(BootstrapModel::Columns::PORT):
         l->port = value.toInt();
         if (l->port <= 0 || l->port > 65534)
            l->port = -1;
         if (d_ptr->save())
            emit dataChanged(index,index);
         break;
   }

   return true;
}

QVariant BootstrapModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid())
      return QVariant();

   BootstrapModelPrivate::Lines* l =  index.row() < d_ptr->m_lines.size() ?d_ptr->m_lines[index.row()] : nullptr;

   if (!l)
      return QVariant();

   switch (role) {
      case Qt::DisplayRole:
      case Qt::EditRole:
         return index.column()?QVariant(l->port == -1?QVariant():l->port):QVariant(l->hostname);
   };

   return QVariant();
}

int BootstrapModel::rowCount( const QModelIndex& parent) const
{
   return parent.isValid()?0:d_ptr->m_lines.size()+1; //Add one for new entries
}

int BootstrapModel::columnCount( const QModelIndex& parent) const
{
   return parent.isValid()?0:2;
}

Qt::ItemFlags BootstrapModel::flags( const QModelIndex& index) const
{
   return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable) : Qt::NoItemFlags;
}

QModelIndex BootstrapModel::index( int row, int column, const QModelIndex& parent) const
{
   return parent.isValid()? QModelIndex() : createIndex(row,column);
}

QVariant BootstrapModel::headerData( int section, Qt::Orientation ori, int role) const
{
   if (role == Qt::DisplayRole) {
      if (ori == Qt::Vertical)
         return section;

      switch (section) {
         case static_cast<int>(BootstrapModel::Columns::HOSTNAME):
            return tr("Hostname");
         case static_cast<int>(BootstrapModel::Columns::PORT):
            return tr("Port");
      }
   }
   return QVariant();
}

bool BootstrapModel::isCustom() const
{
   for (const BootstrapModelPrivate::Lines* line : d_ptr->m_lines) {
      if (line->hostname.size() && line->hostname != "bootstrap.ring.cx")
         return true;
   }

   return false;
}

void BootstrapModel::reset()
{
   BootstrapModelPrivate::Lines* l = d_ptr->m_lines[0];
   l->hostname = "bootstrap.ring.cx";
   l->port = -1;

   if (d_ptr->m_lines.size() > 1) {
      beginRemoveRows(QModelIndex(),1,d_ptr->m_lines.size());

      for (int i =1; i < d_ptr->m_lines.size(); i++)
         delete d_ptr->m_lines[i];

      d_ptr->m_lines.clear();
      d_ptr->m_lines << l;
      endRemoveRows();
   }
}
