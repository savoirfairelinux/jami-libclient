/****************************************************************************
 *   Copyright (C) 2014-2018 Savoir-faire Linux                          *
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
#pragma once

#include "typedefs.h"
class HookManagerPrivate;

/**
 * This class allow to get and set the different hooks
 */
class LIB_EXPORT HookManager : public QObject
{
   Q_OBJECT

public:
   static HookManager& instance();

   //Properties
   Q_PROPERTY(QString prefix              READ prefix               WRITE setPrefix             )
   Q_PROPERTY(QString sipFeild            READ sipFeild             WRITE setSipFeild           )
   Q_PROPERTY(QString command             READ command              WRITE setCommand            )
   Q_PROPERTY(bool    sipEnabled          READ isSipEnabled         WRITE setSipEnabled         )
   Q_PROPERTY(bool    phoneNumberEnabled  READ isContactMethodEnabled WRITE setContactMethodEnabled )

   //Getters
   QString prefix           () const;
   QString sipFeild         () const;
   QString command          () const;
   bool isSipEnabled        () const;
   bool isContactMethodEnabled() const;

   //Setters
   void setPrefix             (const QString& prefix );
   void setSipFeild           (const QString& field  );
   void setCommand            (const QString& command);
   void setSipEnabled         (bool enabled          );
   void setContactMethodEnabled (bool enabled          );

private:
   explicit HookManager();
   virtual ~HookManager();

   QScopedPointer<HookManagerPrivate> d_ptr;
};
