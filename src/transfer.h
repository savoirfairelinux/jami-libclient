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
#include <itembase.h>

class TransferPrivate;
class Account;
class ContactMethod;
class FileTransferModel;

///A
class LIB_EXPORT Transfer : public ItemBase
{
    Q_OBJECT
public:

    friend class FileTransferModel;

    //Property
    Q_PROPERTY(Account*                  account         READ account                               )
    Q_PROPERTY(QString                   filename        READ filename      WRITE setFilename       )
    Q_PROPERTY(QString                   id              READ id                                    )
    Q_PROPERTY(int                       size            READ size          WRITE setSize           )
    Q_PROPERTY(int                       sent            READ sent          WRITE setSent           )
    Q_PROPERTY(int                       progress        READ progress                              )
    Q_PROPERTY(ContactMethod*            contactMethod   READ contactMethod WRITE setContactMethod  )
    Q_PROPERTY(int                       status          READ status        WRITE setStatus         )
    Q_PROPERTY(bool                      outgoing        READ isOutgoing                            )
    Q_PROPERTY(bool                      pending         READ isPending                             )

    //Getter
    QString         id      () const;
    Account*        account () const;
    QString         filename() const;
    int             status  () const;
    int             size    () const;
    int             sent    () const;
    int             progress() const;
    bool            isOutgoing() const;
    bool            isPending()  const;
    ContactMethod*  contactMethod() const;

    //Constructor
    explicit Transfer(Account* a, const QString& connectionID, bool isOutgoing = true);

    virtual ~Transfer();

    //Setter
    void setStatus       (int status);
    void setContactMethod(ContactMethod* cm);
    void setFilename     (QString name);
    void setSize         (int size);
    void setSent         (int s);

    /*
    * cancel/refuse this transfer
    */
    void cancel();

    /*
    * @param pathname: destination path
    */
    void accept(const QString& pathname);

    QString dataTransferCodeToString();

Q_SIGNALS:
    void changed();
    void statusChanged(const QString&, int);
    void contactChanged(const QString&, ContactMethod* cm);

private:

    TransferPrivate* d_ptr;

};
Q_DECLARE_METATYPE(Transfer*)
