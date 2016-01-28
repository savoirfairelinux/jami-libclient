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
#include "macro.h"

//Qt
#include <QtCore/QTimer>
#include <QtWidgets/QAction>

//Ring
#include "audio/outputdevicemodel.h"
#include "private/macromodel_p.h"


MacroPrivate::MacroPrivate() : m_Position(0),m_Delay(0),m_pCat(nullptr),m_pPointer(nullptr),
m_pModel(nullptr)
{

}

Macro::Macro(QObject* parent) : ItemBase(parent), d_ptr(new MacroPrivate())
{
}

Macro::Macro(const Macro* macro) : ItemBase(nullptr), d_ptr(new MacroPrivate())
{
   d_ptr->m_Position    = macro->d_ptr->m_Position   ;
   d_ptr->m_Name        = macro->d_ptr->m_Name       ;
   d_ptr->m_Description = macro->d_ptr->m_Description;
   d_ptr->m_Sequence    = macro->d_ptr->m_Sequence   ;
   d_ptr->m_Escaped     = macro->d_ptr->m_Escaped    ;
   d_ptr->m_Id          = macro->d_ptr->m_Id         ;
   d_ptr->m_Delay       = macro->d_ptr->m_Delay      ;
   d_ptr->m_Category    = macro->d_ptr->m_Category   ;
   d_ptr->m_Action      = macro->d_ptr->m_Action     ;
   d_ptr->m_pCat        = macro->d_ptr->m_pCat       ;
   d_ptr->m_pModel      = macro->d_ptr->m_pModel     ;
   d_ptr->m_pPointer    = macro->d_ptr->m_pPointer   ;
}

Macro::~Macro()
{
   delete d_ptr;
}

void Macro::execute() {
   d_ptr->m_Escaped = d_ptr->m_Sequence;
   while (d_ptr->m_Escaped.indexOf("\\n") != -1) {
      d_ptr->m_Escaped = d_ptr->m_Escaped.replace("\\n","\n");
   }
   d_ptr->nextStep();
}

void MacroPrivate::nextStep()
{
   if (m_Position < m_Escaped.size()) {
      if (!MacroModel::instance()->d_ptr->m_lListeners.size())
         Audio::OutputDeviceModel::playDTMF(QString(m_Escaped[m_Position]));
      else {
         foreach(MacroModel::MacroListener* l, MacroModel::instance()->d_ptr->m_lListeners) {
            l->addDTMF(QString(m_Escaped[m_Position]));
         }
      }
      m_Position++;
      QTimer::singleShot(m_Delay?m_Delay:100,this,SLOT(nextStep()));
   }
   else {
      m_Position = 0;
   }
}

QModelIndex Macro::index()
{
   QModelIndex parent = d_ptr->m_pModel->index(d_ptr->m_pModel->d_ptr->m_lCategories.indexOf(d_ptr->m_pCat),0,QModelIndex());
   return  d_ptr->m_pModel->index(d_ptr->m_pCat->m_lContent.indexOf(this),0,parent);
}

void Macro::setName(const QString &value)
{
   d_ptr->m_Name = value;
   emit changed(this);
   //d_ptr->m_Action->setText(m_Name);
}

void Macro::setDescription(const QString &value)
{
   d_ptr->m_Description = value;
   emit changed(this);
}
void Macro::setSequence(const QString &value)
{
   d_ptr->m_Sequence = value;
   emit changed(this);
}

void Macro::setEscaped(const QString &value)
{
   d_ptr->m_Escaped = value;
   emit changed(this);
}

void Macro::setId(const QString &value)
{
   d_ptr->m_Id = value;
   emit changed(this);
}

void Macro::setDelay(int value)
{
   d_ptr->m_Delay = value;
   emit changed(this);
}

void Macro::setCategory(const QString &value)
{
   d_ptr->m_Category = value;
   emit changed(this);
}

QString Macro::name() const
{
   return d_ptr->m_Name;
}

QString Macro::description() const
{
   return d_ptr->m_Description;
}

QString Macro::sequence() const
{
   return d_ptr->m_Sequence;
}

QString Macro::escaped() const
{
   return d_ptr->m_Escaped;
}

QString Macro::id() const
{
   return d_ptr->m_Id;
}

int Macro::delay() const
{
   return d_ptr->m_Delay;
}

QString  Macro::category() const
{
   return d_ptr->m_Category;
}

QVariant Macro::action() const
{
   return d_ptr->m_Action;
}

#include <macro.moc>
