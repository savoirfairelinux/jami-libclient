/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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

#include "macromodel.h"

//System
#include <time.h>

//Qt
#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>

//Ring
#include "macro.h"
#include "private/macromodel_p.h"
#include "delegates/delegates.h"
#include "delegates/shortcutdelegate.h"

///Init static attributes
MacroModel*  MacroModelPrivate::m_pInstance = nullptr;

MacroModelPrivate::MacroModelPrivate(MacroModel* parent) : q_ptr(parent),
m_pCurrentMacro(nullptr),m_pCurrentMacroMemento(nullptr)
{

}

MacroModel::MacroModel(QObject* parent) : QAbstractItemModel(parent), d_ptr(new MacroModelPrivate(this)), CollectionManagerInterface<Macro>(this)
{

}

MacroModel::~MacroModel()
{
   delete d_ptr;
}

///Singleton
MacroModel* MacroModel::instance()
{
   if (MacroModelPrivate::m_pInstance == nullptr) {
      MacroModelPrivate::m_pInstance = new MacroModel(0);
   }
   return MacroModelPrivate::m_pInstance;
}

void MacroModel::addListener(MacroListener* interface)
{
   MacroModel* m = instance();
   m->d_ptr->m_lListeners << interface;
}

MacroModelPrivate::MacroCategory* MacroModelPrivate::createCategory(const QString& name)
{
   MacroCategory* cat = new MacroCategory;
   cat->m_Name = name;
   cat->m_pPointer = new IndexPointer(IndexType::CategoryIndex,cat);
   m_lCategories << cat;
   emit q_ptr->dataChanged(q_ptr->index((m_lCategories.size()-2 > 0)? m_lCategories.size()-2:0,0,QModelIndex()),
                    q_ptr->index((m_lCategories.size()-1>0  )? m_lCategories.size()-1:0,0,QModelIndex()));
   emit q_ptr->layoutChanged();
   return cat;
}

void MacroModelPrivate::updateTreeModel(Macro* newMacro)
{
   QString catName = newMacro->d_ptr->m_Category.isEmpty()?tr("Other"):newMacro->d_ptr->m_Category;
   foreach (MacroCategory* cat, m_lCategories) {
      if (cat->m_Name == catName) {
         cat->m_lContent << newMacro;
         newMacro->d_ptr->m_pCat = cat;
         newMacro->d_ptr->m_Category = cat->m_Name;
         newMacro->d_ptr->m_pPointer = new IndexPointer(IndexType::MacroIndex,newMacro);
         return;
      }
   }
   MacroCategory* cat = createCategory(catName);
   cat->m_lContent << newMacro;
   newMacro->d_ptr->m_pCat = cat;
   newMacro->d_ptr->m_pPointer = new IndexPointer(IndexType::MacroIndex,newMacro);
}

//Remove the selected macro
bool MacroModel::removeMacro(const QModelIndex& idx)
{
   MacroModelPrivate::IndexPointer* modelItem = (MacroModelPrivate::IndexPointer*)idx.internalPointer();
   if (modelItem && modelItem->type == MacroModelPrivate::IndexType::MacroIndex) {
      Macro* macro = static_cast<Macro*>(modelItem->data);
      macro->remove();
      MacroModelPrivate::MacroCategory* cat = macro->d_ptr->m_pCat;
      cat->m_lContent.removeAll(macro);
      emit layoutChanged();
   }
   else
      qWarning() << "Cannot remove macro: none is selected";
   return true;
}

void MacroModel::setCurrent(const QModelIndex& current, const QModelIndex& previous)
{
   Q_UNUSED(previous)
   if (!current.isValid())
      return;
   MacroModelPrivate::IndexPointer* modelItem = (MacroModelPrivate::IndexPointer*)current.internalPointer();
   if (modelItem && modelItem->type == MacroModelPrivate::IndexType::MacroIndex) {
      Macro* macro = static_cast<Macro*>(modelItem->data);
      d_ptr->m_pCurrentMacro = macro;
      emit selectMacro(d_ptr->m_pCurrentMacro);
   }
}

void MacroModel::save()
{
   foreach (MacroModelPrivate::MacroCategory* cat, d_ptr->m_lCategories) {
      foreach(Macro* macro,cat->m_lContent) {
         macro->save();
         return;
      }
   }
}

bool MacroModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QVariant MacroModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid())
      return QVariant();
   if (!index.parent().isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
      return QVariant(d_ptr->m_lCategories[index.row()]->m_Name);
   }
   else if (index.parent().isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
      return QVariant(d_ptr->m_lCategories[index.parent().row()]->m_lContent[index.row()]->d_ptr->m_Name);
   }
   return QVariant();
}

QVariant MacroModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QVariant(tr("Macros"));
   return QVariant();
}

int MacroModel::rowCount( const QModelIndex& parent ) const
{
   if (!parent.isValid()) {
      return d_ptr->m_lCategories.size();
   }
   else if (!parent.parent().isValid() && parent.row() < d_ptr->m_lCategories.size()) {
      return d_ptr->m_lCategories[parent.row()]->m_lContent.size();
   }
   return 0;
}

Qt::ItemFlags MacroModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid())
      return 0;
   return Qt::ItemIsEnabled | ((index.parent().isValid())?Qt::ItemIsSelectable:Qt::ItemIsEnabled);
}

int MacroModel::columnCount ( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 1;
}

QModelIndex MacroModel::parent( const QModelIndex& index) const
{
   if (!index.isValid())
      return QModelIndex();
   MacroModelPrivate::IndexPointer* modelItem = (MacroModelPrivate::IndexPointer*)index.internalPointer();
   if (modelItem && modelItem->type == MacroModelPrivate::IndexType::MacroIndex) {
      const int idx = d_ptr->m_lCategories.indexOf(static_cast<Macro*>(modelItem->data)->d_ptr->m_pCat);
      if (idx != -1) {
         return MacroModel::index(idx,0,QModelIndex());
      }
   }
   return QModelIndex();
}

QModelIndex MacroModel::index( int row, int column, const QModelIndex& parent) const
{
   if (!parent.isValid() && d_ptr->m_lCategories.size() > row) {
      return createIndex(row,column,d_ptr->m_lCategories[row]->m_pPointer);
   }
   else if (parent.isValid() && d_ptr->m_lCategories[parent.row()]->m_lContent.size() > row) {
      return createIndex(row,column,d_ptr->m_lCategories[parent.row()]->m_lContent[row]->d_ptr->m_pPointer);
   }
   return QModelIndex();
}

void MacroModelPrivate::changed(Macro* macro)
{
   if (macro && macro->d_ptr->m_pCat) {
      QModelIndex parent = q_ptr->index(m_lCategories.indexOf(macro->d_ptr->m_pCat),0);
      emit q_ptr->dataChanged(q_ptr->index(0,0,parent),q_ptr->index(q_ptr->rowCount(parent),0,parent));
      if (macro->d_ptr->m_pCat->m_Name != macro->d_ptr->m_Category) {
         MacroCategory* newIdx = nullptr;
         foreach (MacroCategory* cat, m_lCategories) {
            if (cat->m_Name == macro->d_ptr->m_Category) {
               newIdx = cat;
               break;
            }
         }
         if (macro->d_ptr->m_pCat->m_lContent.size() == 1 && !newIdx) { //Rename
            int idx = m_lCategories.indexOf(macro->d_ptr->m_pCat);
            macro->d_ptr->m_pCat->m_Name = macro->d_ptr->m_Category;
            emit q_ptr->dataChanged(q_ptr->index(idx,0),q_ptr->index(idx,0));
            return;
         }
         else {
            macro->d_ptr->m_pCat->m_lContent.removeAll(macro);
            if (!macro->d_ptr->m_pCat->m_lContent.size()) {
               m_lCategories.removeAll(macro->d_ptr->m_pCat);
               delete macro->d_ptr->m_pCat;
               emit q_ptr->dataChanged(q_ptr->index(0,0),q_ptr->index(m_lCategories.size()-1,0));
            }
            macro->d_ptr->m_pCat = nullptr;
         }

         if (newIdx) {
            newIdx->m_lContent << macro;
            macro->d_ptr->m_pCat = newIdx;
            macro->d_ptr->m_Category = newIdx->m_Name;
            return;
         }
         MacroCategory* cat = createCategory(macro->d_ptr->m_Category);
         cat->m_lContent << macro;
         macro->d_ptr->m_pCat = cat;
         macro->d_ptr->m_Category = cat->m_Name;
         emit q_ptr->layoutChanged();
      }
   }
}


//Add a new macro if the current one can be saved
Macro* MacroModel::newMacro(const QString& id)
{
   d_ptr->m_pCurrentMacro = new Macro(this);

   d_ptr->m_pCurrentMacro->d_ptr->m_Name = tr("New");
   d_ptr->m_pCurrentMacro->d_ptr->m_Category = tr("Other");
   d_ptr->m_pCurrentMacro->d_ptr->m_pModel = this;
   if (id.isEmpty()) {
      time_t curTime;
      ::time(&curTime);
      d_ptr->m_pCurrentMacro->d_ptr->m_Id = QString::number(curTime);
      while (d_ptr->m_hMacros[d_ptr->m_pCurrentMacro->d_ptr->m_Id]) {
         d_ptr->m_pCurrentMacro->d_ptr->m_Id += '1';
      }
   }
   else
      d_ptr->m_pCurrentMacro->d_ptr->m_Id += id;
   d_ptr->m_hMacros[d_ptr->m_pCurrentMacro->d_ptr->m_Id] = d_ptr->m_pCurrentMacro;

   if (collections().size()) {
      collections()[0]->add(d_ptr->m_pCurrentMacro);
   }
   else {
      qWarning() << "No macro collection are enabled";
   }

   d_ptr->updateTreeModel(d_ptr->m_pCurrentMacro);
   connect(d_ptr->m_pCurrentMacro,SIGNAL(changed(Macro*)),d_ptr,SLOT(changed(Macro*)));
   emit dataChanged(index(0,0),index(d_ptr->m_lCategories.size()-1,0));
   emit layoutChanged ();
   emit selectMacro(d_ptr->m_pCurrentMacro);

   d_ptr->m_pCurrentMacro->d_ptr->m_Action = Delegates::getShortcutDelegate()->createAction(d_ptr->m_pCurrentMacro);
   emit addAction(d_ptr->m_pCurrentMacro->d_ptr->m_Action);

   return d_ptr->m_pCurrentMacro;
}

Macro* MacroModel::getCurrentMacro()
{
   return d_ptr->m_pCurrentMacro;
}


void MacroModel::collectionAddedCallback(CollectionInterface* item)
{
   Q_UNUSED(item)
}

bool MacroModel::addItemCallback(const Macro* item)
{
   Q_UNUSED(item)
   return true;
}

bool MacroModel::removeItemCallback(const Macro* item)
{
   Q_UNUSED(item)
   return true;
}
