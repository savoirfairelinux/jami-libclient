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
#ifndef CIPHERMODEL_H
#define CIPHERMODEL_H

#include "typedefs.h"
#include <QtCore/QAbstractListModel>

class CipherModelPrivate;
class Account;

/**This model is exposed for each account. It allow to ensure that the selected
 * ciphers are supported by the system. This was previously done using a simple
 * line edit in each UI, but it allowed to enter random (and invalid) values,
 * making the process of configuring a secure account harder.
 *
 * @todo This model need to automatically sort the ciphers in the optimal order
 * when sabing them.
 */
class LIB_EXPORT CipherModel : public QAbstractListModel {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

   friend class Account;

public:

   //Property
   Q_PROPERTY(bool useDefault READ useDefault WRITE setUseDefault)

   //Model functions
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getter
   bool useDefault() const;

   //Setter
   void setUseDefault(bool value);

private:

   //Private constructor, can only be called by 'Account'
   explicit CipherModel(Account* parent);
   virtual ~CipherModel();

   CipherModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(CipherModel)

Q_SIGNALS:
   void modified();

};
Q_DECLARE_METATYPE(CipherModel*)
#endif
