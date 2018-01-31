/****************************************************************************
 *   Copyright (C) 2013-2018 Savoir-faire Linux                          *
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
#include "lastusednumbermodel.h"
#include "call.h"
#include "uri.h"
#include "contactmethod.h"

struct ChainedContactMethod {
   ChainedContactMethod(ContactMethod* n) : m_pPrevious(nullptr),m_pNext(nullptr),m_pSelf(n){}
   ChainedContactMethod* m_pPrevious;
   ChainedContactMethod* m_pNext;
   ContactMethod*  m_pSelf;
};

class LastUsedNumberModelPrivate
{
public:
   LastUsedNumberModelPrivate();

   //Const
   constexpr static const int MAX_ITEM = 15;

   //Attributes
   ChainedContactMethod* m_pFirstNode;
   QHash<ContactMethod*,ChainedContactMethod*> m_hNumbers;
   bool m_IsValid;
   ChainedContactMethod* m_lLastNumbers[MAX_ITEM] {};
};

LastUsedNumberModelPrivate::LastUsedNumberModelPrivate():m_pFirstNode(nullptr),m_IsValid(false)
{}

LastUsedNumberModel::LastUsedNumberModel() : QAbstractListModel(),d_ptr(new LastUsedNumberModelPrivate())
{
   for (int i=0;i<LastUsedNumberModelPrivate::MAX_ITEM;i++)
      d_ptr->m_lLastNumbers[i] = nullptr;
}

LastUsedNumberModel::~LastUsedNumberModel()
{
   delete d_ptr;
}

LastUsedNumberModel& LastUsedNumberModel::instance()
{
    static auto instance = new LastUsedNumberModel();
    return *instance;
}

QHash<int,QByteArray> LastUsedNumberModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

///Push 'call' phoneNumber on the top of the stack
void LastUsedNumberModel::addCall(Call* call)
{
   ContactMethod* number = call->peerContactMethod();
   ChainedContactMethod* node = d_ptr->m_hNumbers[number];
   if (!number || ( node && d_ptr->m_pFirstNode == node) ) {
      //TODO enable threaded numbers now
      return;
   }

   if (!node) {
      node = new ChainedContactMethod(number);
      d_ptr->m_hNumbers[number] = node;
   }
   else {
      if (node->m_pPrevious)
         node->m_pPrevious->m_pNext = node->m_pNext;
      if (node->m_pNext)
         node->m_pNext->m_pPrevious = node->m_pPrevious;
   }
   if (d_ptr->m_pFirstNode) {
      d_ptr->m_pFirstNode->m_pPrevious = node;
      node->m_pNext = d_ptr->m_pFirstNode;
   }
   d_ptr->m_pFirstNode = node;
   d_ptr->m_IsValid = false;
   emit layoutChanged();
}


QVariant LastUsedNumberModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid())
      return QVariant();
   if (!d_ptr->m_IsValid) {
      ChainedContactMethod* current = d_ptr->m_pFirstNode;
      for (int i=0;i<LastUsedNumberModelPrivate::MAX_ITEM;i++) { //Can only grow, no need to clear
         d_ptr->m_lLastNumbers[i] = current;
         current = current->m_pNext;
         if (!current)
            break;
      }
      d_ptr->m_IsValid = true;
   }
   switch (role) {
      case Qt::DisplayRole: {
         return d_ptr->m_lLastNumbers[index.row()]->m_pSelf->uri();
      }
   };
   return QVariant();
}

int LastUsedNumberModel::rowCount( const QModelIndex& parent) const
{
   if (parent.isValid())
      return 0;
   return d_ptr->m_hNumbers.size() < LastUsedNumberModelPrivate::MAX_ITEM?d_ptr->m_hNumbers.size():LastUsedNumberModelPrivate::MAX_ITEM;
}

Qt::ItemFlags LastUsedNumberModel::flags( const QModelIndex& index) const
{
   Q_UNUSED(index)
   return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool LastUsedNumberModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}
