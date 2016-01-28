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
#include "numbercategory.h"

#include <QtCore/QSize>

#include "globalinstances.h"
#include "interfaces/pixmapmanipulatori.h"
#include "collectionmediator.h"
#include "collectioneditor.h"
#include "contactmethod.h"
#include "numbercategorymodel.h"
#include "private/numbercategorymodel_p.h"

class NumberCategoryPrivate
{
public:
   NumberCategoryPrivate();

   //Attributes
   QString  m_Name ;
   QVariant m_pIcon;
   int      m_Key  ;
};

NumberCategoryPrivate::NumberCategoryPrivate() : m_pIcon(), m_Name(), m_Key(-1)
{
}

NumberCategory::NumberCategory(CollectionMediator<ContactMethod>* mediator, const QString& name) : CollectionInterface(static_cast<CollectionEditor<ContactMethod>*>(nullptr)), d_ptr(new NumberCategoryPrivate())
{
   Q_UNUSED(mediator)
   d_ptr->m_Name = name;
}

NumberCategory::~NumberCategory()
{
}

QVariant NumberCategory::icon() const
{
   return GlobalInstances::pixmapManipulator().numberCategoryIcon(d_ptr->m_pIcon,QSize(),false,false);
}

QVariant NumberCategory::icon(bool isTracked, bool isPresent) const
{
   return GlobalInstances::pixmapManipulator().numberCategoryIcon(d_ptr->m_pIcon,QSize(),isTracked,isPresent);
}

QString  NumberCategory::name() const
{
   return d_ptr->m_Name;
}

QString NumberCategory::category() const
{
   return QObject::tr("Phone number types");
}

bool NumberCategory::isEnabled() const
{
   return true;
}

QByteArray NumberCategory::id() const
{
   return "numbercat"+d_ptr->m_Name.toLatin1();
}

bool NumberCategory::load()
{
   return false;
}

int NumberCategory::size() const
{
   return NumberCategoryModel::instance()->d_ptr->getSize(this);
}

int NumberCategory::key() const
{
   return d_ptr->m_Key;
}

FlagPack<CollectionInterface::SupportedFeatures> NumberCategory::supportedFeatures() const
{
   return CollectionInterface::SupportedFeatures::NONE         |
          CollectionInterface::SupportedFeatures::MANAGEABLE   |
          CollectionInterface::SupportedFeatures::LOAD         ;
}

void NumberCategory::setIcon(const QVariant& pixmap)
{
   d_ptr->m_pIcon = pixmap;
}

void NumberCategory::setName(const QString& name)
{
   d_ptr->m_Name = name;
}

void NumberCategory::setKey(const int key)
{
   d_ptr->m_Key = key;
}
