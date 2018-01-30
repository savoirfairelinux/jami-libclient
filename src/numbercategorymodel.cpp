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
#include "numbercategorymodel.h"
#include "private/numbercategorymodel_p.h"
#include "contactmethod.h"
#include "numbercategory.h"

NumberCategoryModel::NumberCategoryModel(QObject* parent) : QAbstractListModel(parent),CollectionManagerInterface(this),d_ptr(new NumberCategoryModelPrivate())
{
}

NumberCategoryModel::~NumberCategoryModel()
{
   delete d_ptr;
}

NumberCategoryModelPrivate::~NumberCategoryModelPrivate()
{
   for (InternalTypeRepresentation* rep : m_lCategories)
      delete rep;
}

QHash<int,QByteArray> NumberCategoryModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[Role::KEY] = "key";
   }
   return roles;
}

//Abstract model member
QVariant NumberCategoryModel::data(const QModelIndex& index, int role) const
{
   if (!index.isValid()) return QVariant();
   switch (role) {
      case Qt::DisplayRole: {
         const QString name = d_ptr->m_lCategories[index.row()]->category->name();
         return name.isEmpty()?tr("Uncategorized"):name;
      }
      case Qt::DecorationRole:
         return d_ptr->m_lCategories[index.row()]->category->icon();//m_pDelegate->icon(m_lCategories[index.row()]->icon);
      case Qt::CheckStateRole:
         return d_ptr->m_lCategories[index.row()]->enabled?Qt::Checked:Qt::Unchecked;
      case Qt::UserRole:
         return 'x'+QString::number(d_ptr->m_lCategories[index.row()]->counter);
      case Role::KEY:
         return d_ptr->m_lCategories[index.row()]->category->key();
   }
   return QVariant();
}

int NumberCategoryModel::rowCount(const QModelIndex& parent) const
{
   if (parent.isValid()) return 0;
   return d_ptr->m_lCategories.size();
}

Qt::ItemFlags NumberCategoryModel::flags(const QModelIndex& index) const
{
   return (d_ptr->m_lCategories[index.row()]->category->name().isEmpty() ? Qt::NoItemFlags : Qt::ItemIsEnabled)
            | Qt::ItemIsSelectable
            | Qt::ItemIsUserCheckable;
}

bool NumberCategoryModel::setData(const QModelIndex& idx, const QVariant &value, int role)
{
   if (idx.isValid() && role == Qt::CheckStateRole) {
      d_ptr->m_lCategories[idx.row()]->enabled = value.toBool();
      emit dataChanged(idx,idx);
      return true;
   }
   return false;
}

/**
 * Manually add a new category
 */
NumberCategory* NumberCategoryModel::addCategory(const QString& name, const QVariant& icon, int key)
{
   NumberCategoryModelPrivate::InternalTypeRepresentation* rep = d_ptr->m_hByName[name];
   if (!name.size())
      return this->other();

   if (!rep) {
      rep = new NumberCategoryModelPrivate::InternalTypeRepresentation();
      rep->counter = 0;
   }

   NumberCategory* cat = addCollection<NumberCategory,QString>(name, LoadOptions::NONE);
   cat->setKey ( key  );
   cat->setIcon( icon );

   rep->category   = cat                        ;
   rep->index      = d_ptr->m_lCategories.size();
   rep->enabled    = false                      ;

   this->beginInsertRows(this->nameToIndex(name),d_ptr->m_lCategories.size(), d_ptr->m_lCategories.size());
   d_ptr->m_hToInternal[ cat           ] = rep;
   d_ptr->m_hByIdx     [ key           ] = rep;
   d_ptr->m_hByName    [ name.toLower()] = rep;
   d_ptr->m_lCategories << rep;
   this->endInsertRows();
   return cat;
}

NumberCategoryModel& NumberCategoryModel::instance()
{
    static auto instance = new NumberCategoryModel;
    return *instance;
}

/*void NumberCategoryModel::setIcon(int idx, const QVariant& icon)
{
   NumberCategoryModelPrivate::InternalTypeRepresentation* rep = d_ptr->m_hByIdx[idx];
   if (rep) {
      rep->category->setIcon(icon);
      emit dataChanged(index(d_ptr->m_lCategories.indexOf(rep),0),index(d_ptr->m_lCategories.indexOf(rep),0));
   }
}*/

QModelIndex NumberCategoryModel::nameToIndex(const QString& name) const
{
   const QString lower = name.toLower();
   if (!d_ptr->m_hByName[lower])
      return QModelIndex();
   else {
      return index(d_ptr->m_hByName[lower]->index,0);
   }
}

///Be sure the category exist, increment the counter
void NumberCategoryModelPrivate::registerNumber(ContactMethod* number)
{
   const QString lower = number->category()->name().toLower();
   NumberCategoryModelPrivate::InternalTypeRepresentation* rep = m_hByName[lower];
   if (!rep) {
      NumberCategoryModel::instance().addCategory(number->category()->name(),QVariant());
      rep = m_hByName[lower];
   }
   rep->counter++;
}

void NumberCategoryModelPrivate::unregisterNumber(ContactMethod* number)
{
   const QString lower = number->category()->name().toLower();
   NumberCategoryModelPrivate::InternalTypeRepresentation* rep = m_hByName[lower];
   if (rep)
      rep->counter--;
}

///Get the category (case insensitive)
NumberCategory* NumberCategoryModel::getCategory(const QString& type)
{
   const QString lower = type.toLower();
   if (!lower.size())
       return this->other();
   else {
      NumberCategoryModelPrivate::InternalTypeRepresentation* internal = d_ptr->m_hByName[lower];
      if (internal)
         return internal->category;
      return addCategory(lower,QVariant());
   }
}


NumberCategory* NumberCategoryModel::other()
{
   static QString translated = QObject::tr("Other");
   static QString lower      = translated.toLower();
   if (instance().d_ptr->m_hByName[lower])
       return instance().d_ptr->m_hByName[lower]->category;

   ///Singleton
   static auto other = instance().addCategory(lower, QVariant());
   return other;
}

int NumberCategoryModelPrivate::getSize(const NumberCategory* cat) const
{
   NumberCategoryModelPrivate::InternalTypeRepresentation* i = m_hToInternal[cat];
   return i ? i->counter : 0;
}

void NumberCategoryModel::collectionAddedCallback(CollectionInterface* collection)
{
   Q_UNUSED(collection)
}

bool NumberCategoryModel::addItemCallback(const ContactMethod* item)
{
   Q_UNUSED(item)
   return false;
}

bool NumberCategoryModel::removeItemCallback(const ContactMethod* item)
{
   Q_UNUSED(item)
   return false;
}

#include <numbercategorymodel.moc>
