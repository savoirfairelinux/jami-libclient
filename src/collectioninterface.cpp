/************************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                                       *
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
#include "itembase.h"
#include "delegates/pixmapmanipulationdelegate.h"

//Libstdc++
#include <functional>

//Qt
#include <QtCore/QHash>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

///Destructor
CollectionInterface::~CollectionInterface()
{
   delete d_ptr;
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
   return PixmapManipulationDelegate::instance()->collectionIcon(this);
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


bool CollectionInterface::add(ItemBase<QObject>* base)
{
   return d_ptr->m_fAdd(base);
}

bool CollectionInterface::save(ItemBase<QObject>* base)
{
   return d_ptr->m_fSave(base);
}

bool CollectionInterface::save(const ItemBase<QObject>* base)
{
   return d_ptr->m_fSave(const_cast<ItemBase<QObject>*>(base));
}

bool CollectionInterface::edit(ItemBase<QObject>* base)
{
   return d_ptr->m_fEdit(base);
}

bool CollectionInterface::remove(ItemBase<QObject>* base)
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

void CollectionInterface::activate(ItemBase<QObject>* base)
{
   base->d_ptr->m_isActive = true;
}

void CollectionInterface::deactivate(ItemBase<QObject>* base)
{
   base->d_ptr->m_isActive = false;
}

QMetaObject CollectionInterface::metaObject()
{
   return d_ptr->m_pEditorType;
}

CollectionConfigurationInterface* CollectionInterface::configurator() const
{
   return d_ptr->m_fConfigurator();
}

void CollectionInterface::setConfigurator(std::function<CollectionConfigurationInterface*()> getter)
{
   d_ptr->m_fConfigurator = getter;
}
