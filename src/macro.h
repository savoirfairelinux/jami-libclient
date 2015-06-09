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
#ifndef MACRO_H
#define MACRO_H

//Qt
#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QModelIndex>

//Ring
class MacroModel;
#include "typedefs.h"
#include "itembase.h"

class MacroPrivate;

class LIB_EXPORT Macro : public ItemBase<QObject>
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

   friend class MacroModel       ; //Use factory method
   friend class MacroModelPrivate; //Use factory method

public:
   explicit Macro(const Macro* macro);
   virtual ~Macro();

   //Getters
   QString  name        () const;
   QString  description () const;
   QString  sequence    () const;
   QString  escaped     () const;
   QString  id          () const;
   int      delay       () const;
   QString  category    () const;
   QVariant action      () const;

   QModelIndex index();

   //Setters
   void setName         (const QString &value);
   void setDescription  (const QString &value);
   void setSequence     (const QString &value);
   void setEscaped      (const QString &value);
   void setId           (const QString &value);
   void setDelay        (int            value);
   void setCategory     (const QString &value);

private:
   explicit Macro(QObject* parent = nullptr);

   MacroPrivate* d_ptr;
   Q_DECLARE_PRIVATE(Macro)

public Q_SLOTS:
   void execute();

Q_SIGNALS:
   void changed(Macro*);
};

#endif
