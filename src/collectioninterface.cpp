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
#include "phonenumber.h"
#include "collectioneditor.h"
#include "itembase.h"

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

bool CollectionInterface::save(ItemBase<QObject>* base)
{
   return d_ptr->m_pSave(base);
}

bool CollectionInterface::edit(ItemBase<QObject>* base)
{
   return d_ptr->m_pEdit(base);
}

bool CollectionInterface::remove(ItemBase<QObject>* base)
{
   return d_ptr->m_pRemove(base);
}

QMetaObject CollectionInterface::metaObject()
{
   return d_ptr->m_pEditorType;
}
