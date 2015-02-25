/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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

//Qt
#include <QtCore/QMetaObject>

//Ring
#include <collectionmediator.h>

template<typename T>
CollectionEditor<T>::CollectionEditor(CollectionMediator<T>* m) : CollectionEditorBase(m->model()), m_pMediator(m)
{
   Q_ASSERT(m);
}

template<typename T>
CollectionEditor<T>::~CollectionEditor()
{

}

template<typename T>
CollectionMediator<T>* CollectionEditor<T>::mediator() const
{
   return m_pMediator;
}

template<typename T>
QMetaObject metaObject()
{
   return T::staticMetaObject();
}

///Default batch saving implementation, some collections have better APIs
template <class T> bool CollectionEditor<T>::batchSave(const QList<T*> contacts)
{
   bool ret = true;
   foreach(const T* c, contacts) {
      ret &= save(c);
   }
   return ret;
}

template <class T>
bool CollectionEditor<T>::addContactMethod( T*       item , ContactMethod* number )
{
   Q_UNUSED(item)
   Q_UNUSED(number)
   return false;
}

template <class T>
bool CollectionEditor<T>::remove(const T* item)
{
   Q_UNUSED(item)
   return false;
}
