/****************************************************************************
 *   Copyright (C) 2015-2016 by Savoir-faire Linux                               *
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

typedef void (BootstrapModelPrivate::*BootstrapModelPrivateFct)();

class BootstrapModelPrivate
{
public:
   struct Lines {
      QString hostname;
      int     port    ;
   };

   BootstrapModelPrivate(BootstrapModel* q,Account* a);

   //Callbacks
   void  clear   ();
   void  reload  ();
   void  save    ();
   void  nothing ();
   void  modify  ();
   void  reset   ();

   //Helper
   void clearLines();
   inline void performAction(const BootstrapModel::EditAction action);

   //Attributes
   Account*                   m_pAccount ;
   QVector<Lines*>            m_lines    ;
   BootstrapModel*            q_ptr      ;
   BootstrapModel::EditState  m_EditState;
   static Matrix2D<BootstrapModel::EditState, BootstrapModel::EditAction, BootstrapModelPrivateFct> m_mStateMachine;
};

BootstrapModelPrivate::BootstrapModelPrivate(BootstrapModel* q,Account* a) : q_ptr(q),m_pAccount(a)
{

}

#define BMP &BootstrapModelPrivate
Matrix2D<BootstrapModel::EditState, BootstrapModel::EditAction, BootstrapModelPrivateFct> BootstrapModelPrivate::m_mStateMachine ={{
   /*                     SAVE         MODIFY        RELOAD        CLEAR           RESET      */
   /* LOADING   */ {{ BMP::nothing, BMP::nothing, BMP::reload , BMP::nothing , BMP::nothing }},
   /* READY     */ {{ BMP::nothing, BMP::modify , BMP::reload , BMP::clear,    BMP::reset   }},
   /* MODIFIED  */ {{ BMP::save   , BMP::nothing, BMP::reload , BMP::clear,    BMP::reset   }},
   /* OUTDATED  */ {{ BMP::save   , BMP::nothing, BMP::reload , BMP::clear,    BMP::reset   }},
   /* RELOADING */ {{ BMP::nothing, BMP::nothing, BMP::nothing, BMP::nothing,  BMP::nothing }},
}};
#undef BMP


void BootstrapModelPrivate::performAction(const BootstrapModel::EditAction action)
{
   (this->*(m_mStateMachine[m_EditState][action]))();
}

void BootstrapModelPrivate::nothing()
{
   //nothing
}

void BootstrapModelPrivate::save()
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
   for(int i=0;i<m_lines.size();i++) {
      Lines* l = m_lines[i];
      if (l->hostname.isEmpty() && l->port == -1) {
         q_ptr->beginRemoveRows(QModelIndex(),i,i);
         const int idx = m_lines.indexOf(l);
         if (idx >= 0)
            m_lines.removeAt(idx);
         q_ptr->endRemoveRows();
      }
   }

   m_pAccount->d_ptr->setAccountProperty(DRing::Account::ConfProperties::HOSTNAME,ret);
   m_EditState = BootstrapModel::EditState::READY;
}

void BootstrapModelPrivate::modify()
{
   m_EditState = BootstrapModel::EditState::MODIFIED;
   m_pAccount << Account::EditAction::MODIFY;
}

void BootstrapModelPrivate::reload()
{
    m_EditState = BootstrapModel::EditState::RELOADING;
    clearLines();

    for(const QString& line : m_pAccount->hostname().split(';')) {
      const QStringList& fields = line.split(':');

      if (line.size() && fields.size() && !(fields[0].isEmpty() && (fields.size()-1 && fields[1].isEmpty()))) {
         BootstrapModelPrivate::Lines* l = new BootstrapModelPrivate::Lines();
         l->hostname = fields[0].trimmed();
         l->port     = fields.size()>1?fields[1].toInt():-1; //-1 == default

         q_ptr->beginInsertRows(QModelIndex(),m_lines.size()+1,m_lines.size()+1);
         m_lines << l;
         q_ptr->endInsertRows();
      }
   }
   m_EditState = BootstrapModel::EditState::READY;
}

void BootstrapModelPrivate::clear()
{
   clearLines();
   q_ptr << BootstrapModel::EditAction::MODIFY;
}

void BootstrapModelPrivate::clearLines()
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

BootstrapModel::BootstrapModel(Account* a) : QAbstractTableModel(a), d_ptr(new BootstrapModelPrivate(this,a))
{
    d_ptr->m_EditState = BootstrapModel::EditState::LOADING;
    this << EditAction::RELOAD;
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

//DEPRECATED
void BootstrapModel::reset()
{
    this << EditAction::RESET;
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

   switch (static_cast<BootstrapModel::Columns>(index.column())) {
      case BootstrapModel::Columns::HOSTNAME:
         l->hostname = value.toString();
            d_ptr->save();
            emit dataChanged(index,index);
         break;
      case BootstrapModel::Columns::PORT:
         l->port = value.toInt();
         if (l->port <= 0 || l->port > 65534)
            l->port = -1;
            d_ptr->save();
            emit dataChanged(index,index);
         break;
   }

   this << EditAction::MODIFY;

   return true;
}

QVariant BootstrapModel::data( const QModelIndex& index, int role) const
{
   Q_UNUSED(role);

   if (!index.isValid())
      return QVariant();

   BootstrapModelPrivate::Lines* l =  index.row() < d_ptr->m_lines.size() ?d_ptr->m_lines[index.row()] : nullptr;

   if (!l)
      return QVariant();

   switch (static_cast<BootstrapModel::Columns>(index.column())) {
      case BootstrapModel::Columns::PORT:
         return QVariant(l->port == -1?QVariant():l->port);
      case BootstrapModel::Columns::HOSTNAME:
         return l->hostname;
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

bool BootstrapModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid())
        return false;

    if ((row+count) > d_ptr->m_lines.size())
        return false;

    this->beginRemoveRows(QModelIndex(), row, row+count-1);

    for (int i = 0; i < count; i++)
    {
        BootstrapModelPrivate::Lines* l = d_ptr->m_lines[row+i];
        d_ptr->m_lines.remove(row);
        delete l;
    }

    d_ptr->save();
    this->endRemoveRows();

    return true;
}

Qt::ItemFlags BootstrapModel::flags( const QModelIndex& index) const
{
   return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable) : Qt::NoItemFlags;
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

void BootstrapModelPrivate::reset()
{
   clearLines();

   BootstrapModelPrivate::Lines* l = new BootstrapModelPrivate::Lines();
   l->hostname = "bootstrap.ring.cx";
   l->port = -1;

   q_ptr->beginInsertRows(QModelIndex(), m_lines.size()+1, m_lines.size()+1);
   m_lines << l;
   q_ptr->endInsertRows();

   q_ptr << BootstrapModel::EditAction::MODIFY;
}

BootstrapModel* BootstrapModel::operator<<(BootstrapModel::EditAction& action)
{
   performAction(action);
   return this;
}

BootstrapModel* operator<<(BootstrapModel* a, BootstrapModel::EditAction action)
{
   return (!a)?nullptr : (*a) << action;
}

///Change the current edition state
bool BootstrapModel::performAction(const BootstrapModel::EditAction action)
{
   BootstrapModel::EditState curState = d_ptr->m_EditState;
   d_ptr->performAction(action);
   return curState != d_ptr->m_EditState;
}
