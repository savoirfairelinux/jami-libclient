/****************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                               *
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
#include "presencecollectionextension.h"
#include "collectioninterface.h"
#include "contactmethod.h"
#include "person.h"
#include "presencestatusmodel.h"

PresenceCollectionExtension::PresenceCollectionExtension(QObject* parent) :
   CollectionExtensionInterface(parent)
{

}

QVariant PresenceCollectionExtension::data(CollectionInterface* backend, const QModelIndex& index, int role ) const
{
   if ((!backend) || (!index.isValid()))
      return QVariant();
   switch (role) {
      case Qt::CheckStateRole:
         return PresenceStatusModel::instance()->isAutoTracked(backend)?Qt::Checked:Qt::Unchecked;
   };
   return QVariant();
}

Qt::ItemFlags PresenceCollectionExtension::flags(CollectionInterface* backend, const QModelIndex& index ) const
{
   Q_UNUSED(backend)
   Q_UNUSED(index)
   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool PresenceCollectionExtension::setData(CollectionInterface* backend, const QModelIndex& index, const QVariant &value, int role )
{
   Q_UNUSED(backend)
   if (index.isValid() && role == Qt::CheckStateRole) {
      switch(value.toInt()){
         case Qt::Checked:
            foreach(Person* c, backend->items<Person>()) {
               foreach(ContactMethod* n,c->phoneNumbers()) {
                  n->setTracked(true);
               }
            }
            PresenceStatusModel::instance()->setAutoTracked(backend,true);
            emit dataChanged(index);
            break;
         case Qt::Unchecked:
            foreach(Person* c, backend->items<Person>()) {
               foreach(ContactMethod* n,c->phoneNumbers()) {
                  n->setTracked(false);
               }
            }
            PresenceStatusModel::instance()->setAutoTracked(backend,false);
            emit dataChanged(index);
            break;
      };
      return true;
   }
   return false;
}

QString PresenceCollectionExtension::headerName() const
{
   return tr("Track presence");
}
