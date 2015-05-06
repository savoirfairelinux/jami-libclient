/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
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
#include "collectionmanagerinterface.h"

//Ring
#include <collectionmodel.h>
#include "private/collectionmodel_p.h"

class CollectionManagerInterfaceBasePrivate
{
public:

};

void CollectionManagerInterfaceBase::registerToModel(CollectionInterface* col) const
{
   CollectionModel::instance()->d_ptr->registerNew(col);
}

void CollectionManagerInterfaceBase::addCreatorToList(CollectionCreationInterface* creator)
{
   CollectionModel::instance()->d_ptr->m_lCreator << creator;
}

void CollectionManagerInterfaceBase::addConfiguratorToList(CollectionConfigurationInterface* configurator)
{
   CollectionModel::instance()->d_ptr->m_lConfigurator << configurator;
}

void CollectionManagerInterfaceBase::setCollectionConfigurator(CollectionInterface* col, std::function<CollectionConfigurationInterface*()> getter)
{
   col->setConfigurator(getter);
}
