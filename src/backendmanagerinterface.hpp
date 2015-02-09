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
class BackendManagerInterfacePrivate
{
public:
   BackendManagerInterfacePrivate(BackendManagerInterface<T>* p) : m_pMediator(nullptr),q_ptr(p)
   {}
   ~BackendManagerInterfacePrivate();

   QVector< CollectionInterface* > m_lBackends;
   QVector< CollectionInterface* > m_lEnabledBackends;
   mutable CollectionMediator<T>*  m_pMediator;
   BackendManagerInterface<T>*      q_ptr;

   CollectionMediator<T>* itemMediator() const;
};

template<class T>
CollectionMediator<T>* BackendManagerInterfacePrivate<T>::itemMediator() const
{
   if (!m_pMediator) {
      m_pMediator = new CollectionMediator<T>(q_ptr);
   }
   return m_pMediator;
}

template<class T>
BackendManagerInterfacePrivate<T>::~BackendManagerInterfacePrivate()
{
   if (m_pMediator)
      delete m_pMediator;
}

template<class T>
template <class T2>
CollectionInterface* BackendManagerInterface<T>::addBackend(const LoadOptions options)
{
   T2* backend = new T2(d_ptr->itemMediator());
   d_ptr->m_lBackends << backend;

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
BackendManagerInterface<T>::BackendManagerInterface() : d_ptr(new BackendManagerInterfacePrivate<T>(this))
{

}

template<class T>
const QVector< CollectionInterface* > BackendManagerInterface<T>::backends() const
{
   return d_ptr->m_lBackends;
}

template<class T>
const QVector< CollectionInterface* > BackendManagerInterface<T>::enabledBackends() const
{
   return d_ptr->m_lEnabledBackends;
}

/// Do this manager have active backends
template<class T>
bool BackendManagerInterface<T>::hasEnabledBackends() const
{
   return d_ptr->m_lEnabledBackends.size();
}

template<class T>
bool BackendManagerInterface<T>::hasBackends() const
{
   return d_ptr->m_lBackends.size();
}

template<class T>
bool BackendManagerInterface<T>::clearAllBackends() const
{
   return false;
}
