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

   QVector< CollectionInterface* > m_lBackends;
   QVector< CollectionInterface* > m_lEnabledBackends;
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
   T2* backend = new T2(d_ptr->itemMediator(),args...);

   //This will force the T2 to be a CollectionInterface subclass
   CollectionInterface* b = backend;
   d_ptr->m_lBackends << b;

   if (options & LoadOptions::FORCE_ENABLED) { //TODO check is the backend is checked

      //Some backends can fail to load directly
      //eventually it will necessary to add an async version of this
      //to load the backend only when it is loaded
      if (backend->load())
         d_ptr->m_lEnabledBackends << backend;
   }

   return backend;
}

template<class T>
CollectionManagerInterface<T>::CollectionManagerInterface(QAbstractItemModel* self) : d_ptr(new CollectionManagerInterfacePrivate<T>(self,this))
{

}

template<class T>
const QVector< CollectionInterface* > CollectionManagerInterface<T>::backends() const
{
   return d_ptr->m_lBackends;
}

template<class T>
const QVector< CollectionInterface* > CollectionManagerInterface<T>::enabledBackends() const
{
   return d_ptr->m_lEnabledBackends;
}

/// Do this manager have active backends
template<class T>
bool CollectionManagerInterface<T>::hasEnabledBackends() const
{
   return d_ptr->m_lEnabledBackends.size();
}

template<class T>
bool CollectionManagerInterface<T>::hasBackends() const
{
   return d_ptr->m_lBackends.size();
}

template<class T>
bool CollectionManagerInterface<T>::clearAllBackends() const
{
   return false;
}

template<class T>
void CollectionManagerInterface<T>::backendAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
}

template<class T>
bool CollectionManagerInterface<T>::deleteItem(T* item)
{
   if (item->backend()->model() == (QAbstractItemModel*) this) {
      if (item->backend()->supportedFeatures() & CollectionInterface::SupportedFeatures::REMOVE) {
         static_cast<CollectionInterface*>(item->backend())->editor<T>()->remove(item);
         return true;
      }
      else
         qDebug() << item << "cannot be deleted, the backend doesn't support removing items";
   }
   else
      qDebug() << item << "cannot be deleted, it is not managed by" << this;
   return false;
}

template<class T>
bool CollectionManagerInterface<T>::enableBackend( CollectionInterface*  backend, bool enabled)
{
   Q_UNUSED(enabled) //TODO implement it
   backend->load();
   return true;
}
