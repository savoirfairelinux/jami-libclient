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

   std::function<bool(ItemBase<QObject>*)>  m_fAdd       ;
   std::function<bool(ItemBase<QObject>*)>  m_fSave      ;
   std::function<bool(ItemBase<QObject>*)>  m_fEdit      ;
   std::function<bool(ItemBase<QObject>*)>  m_fRemove    ;
   std::function<int()                   >  m_fSize      ;

   std::function<CollectionConfigurationInterface*()> m_fConfigurator;
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
   d_ptr->m_fAdd = [editor](ItemBase<QObject>* item)->bool {
      return editor?editor->addNew(static_cast<T*>(item)) : false;
   };
   d_ptr->m_fSave = [editor](ItemBase<QObject>* item)->bool {
      return editor?editor->edit(static_cast<T*>(item)) : false;
   };
   d_ptr->m_fEdit = [editor](ItemBase<QObject>* item)->bool {
      return editor?editor->edit(static_cast<T*>(item)) : false;
   };
   d_ptr->m_fRemove = [editor](ItemBase<QObject>* item)->bool {
      return editor?editor->remove(static_cast<T*>(item)) : false;
   };
   d_ptr->m_fSize = [editor]()->int {
      return editor?editor->items().size() : 0;
   };
}

template<typename T>
QVector<T*> CollectionInterface::items() const
{
   const CollectionEditor<T>* e = editor<T>();
   return e?e->items() : QVector<T*>();
}

template<typename T>
CollectionEditor<T>* CollectionInterface::editor() const
{
//    Q_ASSERT(T::staticMetaObject() == d_ptr->m_pEditorType);
   return static_cast<CollectionEditor<T>*>(d_ptr->m_pEditor);
}