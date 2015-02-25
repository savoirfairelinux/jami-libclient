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

template <class T>
class CollectionManagerInterfacePrivate
{
public:
   ///All manager should be QAbstractItemModel, this wont compile (on purpose) if the aren't
   CollectionManagerInterfacePrivate(QAbstractItemModel* p2, CollectionManagerInterface<T>* p) :
   m_pMediator(nullptr),q_ptr(p2),i_ptr(p)
   {}
   ~CollectionManagerInterfacePrivate();

   QVector< CollectionInterface* > m_lCollections;
   QVector< CollectionInterface* > m_lEnabledCollections;
   mutable CollectionMediator<T>*  m_pMediator;
   QAbstractItemModel*             q_ptr;
   CollectionManagerInterface<T>*  i_ptr;

   CollectionMediator<T>* itemMediator() const;
};

template<class T>
CollectionMediator<T>* CollectionManagerInterfacePrivate<T>::itemMediator() const
{
   if (!m_pMediator) {
      m_pMediator = new CollectionMediator<T>(i_ptr,q_ptr);
   }
   return m_pMediator;
}

template<class T>
CollectionManagerInterfacePrivate<T>::~CollectionManagerInterfacePrivate()
{
   if (m_pMediator)
      delete m_pMediator;
}

template<class T>
template <class T2, typename ...Ts>
T2* CollectionManagerInterface<T>::addBackend(Ts... args, const LoadOptions options)
{
   T2* collection = new T2(d_ptr->itemMediator(),args...);

   //This will force the T2 to be a CollectionInterface subclass
   CollectionInterface* b = collection;
   d_ptr->m_lCollections << b;

   if (options & LoadOptions::FORCE_ENABLED) { //TODO check is the collection is checked

      //Some collections can fail to load directly
      //eventually it will necessary to add an async version of this
      //to load the collection only when it is loaded
      if (collection->load())
         d_ptr->m_lEnabledCollections << collection;
   }

   return collection;
}

template<class T>
CollectionManagerInterface<T>::CollectionManagerInterface(QAbstractItemModel* self) : d_ptr(new CollectionManagerInterfacePrivate<T>(self,this))
{

}

template<class T>
const QVector< CollectionInterface* > CollectionManagerInterface<T>::collections() const
{
   return d_ptr->m_lCollections;
}

template<class T>
const QVector< CollectionInterface* > CollectionManagerInterface<T>::enabledCollections() const
{
   return d_ptr->m_lEnabledCollections;
}

/// Do this manager have active collections
template<class T>
bool CollectionManagerInterface<T>::hasEnabledCollections() const
{
   return d_ptr->m_lEnabledCollections.size();
}

template<class T>
bool CollectionManagerInterface<T>::hasCollections() const
{
   return d_ptr->m_lCollections.size();
}

template<class T>
bool CollectionManagerInterface<T>::clearAllCollections() const
{
   return false;
}

template<class T>
void CollectionManagerInterface<T>::collectionAddedCallback(CollectionInterface* collection)
{
   Q_UNUSED(collection)
}

template<class T>
bool CollectionManagerInterface<T>::deleteItem(T* item)
{
   if (item->collection()->model() == (QAbstractItemModel*) this) {
      if (item->collection()->supportedFeatures() & CollectionInterface::SupportedFeatures::REMOVE) {
         static_cast<CollectionInterface*>(item->collection())->editor<T>()->remove(item);
         return true;
      }
      else
         qDebug() << item << "cannot be deleted, the collection doesn't support removing items";
   }
   else
      qDebug() << item << "cannot be deleted, it is not managed by" << this;
   return false;
}

template<class T>
bool CollectionManagerInterface<T>::enableBackend( CollectionInterface*  collection, bool enabled)
{
   Q_UNUSED(enabled) //TODO implement it
   collection->load();
   return true;
}
