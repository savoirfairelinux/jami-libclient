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
#ifndef AVAILABLE_ACCOUNT_MODEL_H
#define AVAILABLE_ACCOUNT_MODEL_H

#include <QtCore/QSortFilterProxyModel>

#include <accountmodel.h>

class AvailableAccountModelPrivate;

//TODO Qt5 use QAbstractItemProxyModel
/**
 * This model filter the account list to disable all unavailable accounts. It
 * also remove the enabled checkbox as it is no longer relevant.
 *
 * This model also handle all the logic behind the "current account" used by
 * default when passing a call. It use the first "READY" account in the
 * AccountModel list unless manually specified. To change the default
 * permanently, use the setDefaultAccount method.
 *
 * @todo Once the history is not saved by the daemon, implement setDefaultAccount
 */
class LIB_EXPORT AvailableAccountModel : public QSortFilterProxyModel
{
   Q_OBJECT
public:
   AvailableAccountModel(QObject* parent = nullptr);

   virtual QVariant      data            (const QModelIndex& index,int role = Qt::DisplayRole       ) const override;
   virtual Qt::ItemFlags flags           (const QModelIndex& index                                  ) const override;
   virtual bool          filterAcceptsRow(int source_row, const QModelIndex& source_parent          ) const override;

   QItemSelectionModel* selectionModel() const;

   //Getter
   static Account* currentDefaultAccount(ContactMethod* method = nullptr);

   //Singleton
   static AvailableAccountModel* instance();

Q_SIGNALS:
   void currentDefaultAccountChanged(Account*);

private:
   AvailableAccountModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(AvailableAccountModel)
};
Q_DECLARE_METATYPE(AvailableAccountModel*)

#endif