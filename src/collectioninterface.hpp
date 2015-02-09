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


//TODO don't do this
#include <functional>
template<typename T> class ItemBase;

class CollectionInterfacePrivate {
public:

   ///The backend parent of nullptr
   CollectionInterface*          m_pParent    ;

   ///Keep track of all backend childrens
   QVector<CollectionInterface*> m_lChildren  ;

   ///This need to be casted to a CollectionMediator<T>
   void*                          m_pEditor    ;

   ///Use Qt introspection to make sure casting is valid
   QMetaObject                    m_pEditorType;

   std::function<bool(ItemBase<QObject>*)>  m_pSave      ;
   std::function<bool(ItemBase<QObject>*)>  m_pEdit      ;
   std::function<bool(ItemBase<QObject>*)>  m_pRemove    ;
};

template<typename T>
CollectionInterface::CollectionInterface(CollectionEditor<T>* editor, CollectionInterface* parent) :
d_ptr(new CollectionInterfacePrivate())
{
   //Ensure the type is based on QObject (required)
   d_ptr->m_pParent = parent;
//    d_ptr->m_pEditorType = T::staticMetaObject();
   d_ptr->m_pEditor = (void*) editor;

   //The cast is safe because the metatype is checked earlier
   d_ptr->m_pSave = [editor](ItemBase<QObject>* item)->bool {
      return editor->edit(static_cast<T*>(item));
   };
   d_ptr->m_pEdit = [editor](ItemBase<QObject>* item)->bool {
      return editor->edit(static_cast<T*>(item));
   };
   d_ptr->m_pRemove = [editor](ItemBase<QObject>* item)->bool {
      return editor->remove(static_cast<T*>(item));
   };
}

template<typename T>
QVector<T*> CollectionInterface::items() const
{
   return editor<T>()->items();
}

template<typename T>
CollectionEditor<T>* CollectionInterface::editor() const
{
//    Q_ASSERT(T::staticMetaObject() == d_ptr->m_pEditorType);
   return static_cast<CollectionEditor<T>*>(d_ptr->m_pEditor);
}