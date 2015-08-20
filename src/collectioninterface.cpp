/************************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                                  *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/

//Parent
#include "collectioninterface.h"

//Ring library
#include "person.h"
#include "call.h"
#include "contactmethod.h"
#include "collectioneditor.h"
#include "collectionextensionmodel.h"
#include "itembase.h"
#include "interfaces/instances.h"
#include "interfaces/pixmapmanipulatori.h"

//Libstdc++
#include <functional>

//Qt
#include <QtCore/QHash>
#include <QtCore/QDebug>
#include <QtCore/QIdentityProxyModel>
#include <QtCore/QCoreApplication>

class EnabledExtensionsProxy final : public QIdentityProxyModel
{
   Q_OBJECT

public:
   EnabledExtensionsProxy(QAbstractItemModel* parent);

   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant& value,int role )       override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;

private:
   QHash<int,bool> m_Disabled;
};

class CollectionInterfacePrivate
{
public:
   explicit CollectionInterfacePrivate();

   //Attributes
   EnabledExtensionsProxy* m_pEnabledExtensions;
};

EnabledExtensionsProxy::EnabledExtensionsProxy(QAbstractItemModel* parent) : QIdentityProxyModel(parent)
{
   setSourceModel(parent);
}

CollectionInterfacePrivate::CollectionInterfacePrivate(): m_pEnabledExtensions(nullptr)
{
}

void CollectionInterface::init()
{
   d_ptr2 = new CollectionInterfacePrivate();
}

///Destructor
CollectionInterface::~CollectionInterface()
{
   delete d_ptr;
}

QVariant EnabledExtensionsProxy::data( const QModelIndex& index, int role) const
{

   if (index.isValid() && role == Qt::CheckStateRole)
      return m_Disabled[index.row()] ? Qt::Unchecked : Qt::Checked;

   return QIdentityProxyModel::data(index,role);
}

bool EnabledExtensionsProxy::setData( const QModelIndex& index, const QVariant& value,int role )
{
   if (index.isValid() && role == Qt::CheckStateRole) {
      m_Disabled[index.row()] = value == Qt::Unchecked;
      return true;
   }

   return false;
}

Qt::ItemFlags EnabledExtensionsProxy::flags( const QModelIndex& index) const
{
   return QIdentityProxyModel::flags(index) | Qt::ItemIsUserCheckable;
}

QAbstractItemModel* CollectionInterface::extensionsModel() const
{
   if (!d_ptr2->m_pEnabledExtensions)
      d_ptr2->m_pEnabledExtensions = new EnabledExtensionsProxy(CollectionExtensionModel::instance());
   return d_ptr2->m_pEnabledExtensions;
}

CollectionInterface* CollectionInterface::parent  () const
{
   return d_ptr->m_pParent;
}

QVector<CollectionInterface*> CollectionInterface::children() const
{
   return d_ptr->m_lChildren;
}

QVariant CollectionInterface::icon() const
{
   return Interfaces::pixmapManipulator().collectionIcon(this);
}

bool CollectionInterface::clear()
{
   return false;
}

bool CollectionInterface::reload()
{
   return false;
}

bool CollectionInterface::enable(bool)
{
   return false;
}

QAbstractItemModel* CollectionInterface::model() const
{
   return static_cast<CollectionEditorBase*>(d_ptr->m_pEditor)->model();
}

void CollectionInterface::addChildren(CollectionInterface* c)
{
   d_ptr->m_lChildren << c;
}


bool CollectionInterface::add(ItemBase* base)
{
   return d_ptr->m_fAdd(base);
}

bool CollectionInterface::save(ItemBase* base)
{
   return d_ptr->m_fSave(base);
}

bool CollectionInterface::save(const ItemBase* base)
{
   return d_ptr->m_fSave(const_cast<ItemBase*>(base));
}

bool CollectionInterface::edit(ItemBase* base)
{
   return d_ptr->m_fEdit(base);
}

bool CollectionInterface::remove(ItemBase* base)
{
   if (d_ptr->m_fRemove(base)) {
      deactivate(base);
      return true;
   }
   return false;
}

int CollectionInterface::size() const
{
   return  d_ptr->m_fSize();
}

QList<CollectionInterface::Element> CollectionInterface::listId() const
{
   return {};
}

bool CollectionInterface::listId(std::function<void(const QList<CollectionInterface::Element>)> callback) const
{
   Q_UNUSED(callback)
   return false;
}

bool CollectionInterface::fetch( const CollectionInterface::Element& element)
{
   Q_UNUSED(element)
   return false;
}

bool CollectionInterface::fetch( const QList<CollectionInterface::Element>& elements)
{
   Q_UNUSED(elements)
   return false;
}

void CollectionInterface::activate(ItemBase* base)
{
   base->d_ptr->m_isActive = true;
}

void CollectionInterface::deactivate(ItemBase* base)
{
   base->d_ptr->m_isActive = false;
}

const QMetaObject* CollectionInterface::metaObject()
{
   return &d_ptr->m_pEditorType;
}

CollectionConfigurationInterface* CollectionInterface::configurator() const
{
   return d_ptr->m_fConfigurator();
}

void CollectionInterface::setConfigurator(std::function<CollectionConfigurationInterface*()> getter)
{
   d_ptr->m_fConfigurator = getter;
}

#include <collectioninterface.moc>
