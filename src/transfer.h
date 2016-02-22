/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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

#include <QtCore/QObject>
#include <typedefs.h>

#include "dbus/datatransfermanager.h"

struct TransferPrivate;
class Account;
class ContactMethod;

///A set of Transfer to be used by an account
class LIB_EXPORT Transfer final : public QObject
{
   Q_OBJECT
public:

   //Property
   Q_PROPERTY(Account*                  account         READ account WRITE setAccount)
   Q_PROPERTY(QString                   filename        READ filename WRITE setFilename)
   Q_PROPERTY(QString                   id              READ id)
   Q_PROPERTY(int                       size            READ size WRITE setSize)
   Q_PROPERTY(int                       progress        READ progress WRITE setProgress)
   Q_PROPERTY(ContactMethod*            contactMethod   READ contactMethod  WRITE setContactMethod)
   Q_PROPERTY(DRing::DataTransferCode status            READ status         WRITE setStatus)

   //Constructor
   explicit Transfer(Account* a, QString connectionID);
   ~Transfer();

   //Getter
   QString        id        () const;
   Account*       account   () const;
   QString        filename  () const;
   DRing::DataTransferCode status() const;
   int size()                   const;
   int progress() const;
   ContactMethod* contactMethod() const;

   //Setter
   void setStatus       (DRing::DataTransferCode status);
   void setContactMethod(ContactMethod* cm);
   void setAccount      (Account* acc);
   void setFilename     (QString name);
   void setSize         (int size);
   void setProgress     (int p);

   /*
    * cancel/refuse this transfer
    */
   void cancel();

Q_SIGNALS:
   void changed();
   void statusChanged(const QString&, DRing::DataTransferCode);
   void contactChanged(const QString&, ContactMethod* cm);

private:
   TransferPrivate* d_ptr;

};
Q_DECLARE_METATYPE(Transfer*)
