/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
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
#ifndef MACROMODEL_P_H
#define MACROMODEL_P_H

#include <macromodel.h>

class MacroModelPrivate : public QObject
{
   Q_OBJECT
public:
   enum IndexType {
      CategoryIndex = 1,
      MacroIndex = 2
   };

   struct IndexPointer {
      IndexPointer(IndexType _type, void* _data) : type(_type),data(_data) {}
      IndexType type;
      void* data;
   };

   struct MacroCategory {
      MacroCategory():m_pPointer(nullptr){}
      ~MacroCategory() { delete m_pPointer; }
      QString m_Name;
      QList<Macro*> m_lContent;
      IndexPointer* m_pPointer;
   };

   MacroModelPrivate(MacroModel*);

   //Attributes
   QHash<QString,Macro*> m_hMacros               ;
   QList<MacroCategory*> m_lCategories           ;
   Macro*                m_pCurrentMacro         ;
   Macro*                m_pCurrentMacroMemento  ;
   static MacroModel*    m_pInstance             ;
   QList<MacroModel::MacroListener*> m_lListeners;

   //Helper
   MacroCategory* createCategory(const QString& name);
   void updateTreeModel(Macro* newMacro);

private:
   MacroModel* q_ptr;

public Q_SLOTS:
   void changed(Macro* macro);
};

class MacroPrivate : public QObject
{
   Q_OBJECT
public:
   MacroPrivate();

   //Attributes
   int         m_Position   ;
   QString     m_Name       ;
   QString     m_Description;
   QString     m_Sequence   ;
   QString     m_Escaped    ;
   QString     m_Id         ;
   int         m_Delay      ;
   QString     m_Category   ;
   QVariant    m_Action     ;
   MacroModel* m_pModel     ;

   MacroModelPrivate::MacroCategory* m_pCat    ;
   MacroModelPrivate::IndexPointer*  m_pPointer;

public Q_SLOTS:
   void nextStep();
};

#endif
