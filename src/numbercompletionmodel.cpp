/****************************************************************************
 *   Copyright (C) 2013 by Savoir-Faire Linux                               *
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
#include "numbercompletionmodel.h"

//Qt
#include <QtCore/QCoreApplication>

//System
#include <cmath>

//SFLPhone
#include "phonedirectorymodel.h"
#include "phonenumber.h"
#include "accountlistmodel.h"

NumberCompletionModel::NumberCompletionModel() : QAbstractTableModel(QCoreApplication::instance())
{
   setObjectName("NumberCompletionModel");
}

NumberCompletionModel::~NumberCompletionModel()
{
   
}

QVariant NumberCompletionModel::data(const QModelIndex& index, int role ) const
{
   if (!index.isValid()) return QVariant();
   const QMap<int,PhoneNumber*>::iterator i = const_cast<NumberCompletionModel*>(this)->m_hNumbers.begin()+index.row();
   const PhoneNumber* n = i.value();
   const int weight     = i.key  ();

   switch (static_cast<NumberCompletionModel::Columns>(index.column())) {
      case NumberCompletionModel::Columns::CONTENT:
         switch (role) {
            case Qt::DisplayRole:
               return n->uri();
         };
         break;
      case NumberCompletionModel::Columns::NAME:
         switch (role) {
            case Qt::DisplayRole:
               return n->primaryName();
         };
         break;
      case NumberCompletionModel::Columns::ACCOUNT:
         switch (role) {
            case Qt::DisplayRole:
               return n->account()?n->account()->id():AccountListModel::instance()->firstRegisteredAccount()->id();
         };
         break;
      case NumberCompletionModel::Columns::WEIGHT:
         switch (role) {
            case Qt::DisplayRole:
               return weight;
         };
         break;
   };
   return QVariant();
}

int NumberCompletionModel::rowCount(const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;
   return m_hNumbers.size();
}

int NumberCompletionModel::columnCount(const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;
   return 4;
}

Qt::ItemFlags NumberCompletionModel::flags(const QModelIndex& index ) const
{
   if (!index.isValid()) return Qt::NoItemFlags;
   return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant NumberCompletionModel::headerData (int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   Q_UNUSED(orientation)
   constexpr static const char* headers[] = {"URI", "Name", "Account", "Weight"};
   if (role == Qt::DisplayRole) return headers[section];
   return QVariant();
}

bool NumberCompletionModel::setData(const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED( index )
   Q_UNUSED( value )
   Q_UNUSED( role  )
   return false;
}

void NumberCompletionModel::setPrefix(const QString& str)
{
   m_Prefix = str;
   updateModel();
}

void NumberCompletionModel::updateModel()
{
   QSet<PhoneNumber*> numbers;
   locateNameRange  ( m_Prefix, numbers );
   locateNumberRange( m_Prefix, numbers );

   m_hNumbers.clear();
   foreach(PhoneNumber* n,numbers)
      m_hNumbers.insert(getWeight(n),n);
   emit layoutChanged();
}

void NumberCompletionModel::getRange(QMap<QString,PhoneDirectoryModel::NumberWrapper*> map, const QString& prefix, QSet<PhoneNumber*>& set)
{
   if (prefix.isEmpty())
      return;
   QMap<QString,PhoneDirectoryModel::NumberWrapper*>::iterator iBeg = map.begin();
   QMap<QString,PhoneDirectoryModel::NumberWrapper*>::iterator iEnd = map.end  ()-1;

   const QString pref = prefix.toLower();

   const int prefixLen = pref.size();
   int size = map.size()/2;
   bool startOk(false),endOk(false);
   while (size > 1 || (startOk&&endOk)) {
      QMap<QString,PhoneDirectoryModel::NumberWrapper*>::iterator mid = (iBeg+size);
      if (mid != map.end() && mid.key().left(prefixLen) == pref && iBeg.key().left(prefixLen) < pref) {
         //Too far, need to go back
         iBeg = mid;
         while ((iBeg-1).key().left(prefixLen) == pref && iBeg != map.begin())
            iBeg--;
         startOk = true;
      }
      else if ((!startOk) && mid.key().left(prefixLen) < pref)
         iBeg = mid;
      else if(!endOk)
         iEnd = mid;

      while ((iEnd).key().left(prefixLen) == pref && iEnd+1 != map.end())
         iEnd++;

      endOk = (iEnd.key().left(prefixLen) == pref);

      size = ::ceil(size/2.0f);
   }

   while (iBeg.key().left(prefixLen) != pref && iBeg != iEnd)
      iBeg++;

   if (iEnd == iBeg && iBeg.key().left(prefixLen) != pref) {
      iEnd   = map.end();
      iBeg = map.end();
   }
   QMap<QString, PhoneDirectoryModel::NumberWrapper*>::iterator i = iBeg;
   while(i != iEnd) {
      foreach(PhoneNumber* n,i.value()->numbers) {
         set << n;
      }
      i++;
   }
}

void NumberCompletionModel::locateNameRange(const QString& prefix, QSet<PhoneNumber*>& set)
{
   getRange(PhoneDirectoryModel::instance()->m_lSortedNames,prefix,set);
}

void NumberCompletionModel::locateNumberRange(const QString& prefix, QSet<PhoneNumber*>& set)
{
   getRange(PhoneDirectoryModel::instance()->m_hSortedNumbers,prefix,set);
}

uint NumberCompletionModel::getWeight(PhoneNumber* number)
{
   Q_UNUSED(number)
   uint weight = 1;
   weight += (number->weekCount()+1)*150;
   weight += (number->trimCount()+1)*75 ;
   weight += (number->callCount()+1)*35 ;
   weight *= (number->uri().indexOf(m_Prefix)!= -1?3:1);
   weight *= (number->present()?2:1);
   return weight;
}
