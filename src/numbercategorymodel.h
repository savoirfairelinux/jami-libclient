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
#ifndef NUMBERCATEGORYMODEL_H
#define NUMBERCATEGORYMODEL_H
#include "typedefs.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QVector>
#include "collectionmanagerinterface.h"

class NumberCategoryDelegate;
class ContactMethod;
class NumberCategory;
class NumberCategoryModelPrivate;

class LIB_EXPORT NumberCategoryModel : public QAbstractListModel, public CollectionManagerInterface<ContactMethod> {
   Q_OBJECT
public:

   enum Role {
      INDEX = 100,
   };

   //Abstract model member
   virtual QVariant       data    (const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount (const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags    (const QModelIndex& index                                 ) const override;
   virtual bool          setData  (const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Mutator
   NumberCategory* addCategory(const QString& name, const QVariant& icon, int index = -1, bool enabled = true);
   void setIcon(int index, const QVariant& icon);
   void save();

   //Singleton
   static NumberCategoryModel* instance();

   //Getter
   QModelIndex nameToIndex(const QString& name) const;
   NumberCategory* getCategory(const QString& type);
   static NumberCategory* other();
   int getSize(const NumberCategory* cat) const;

   //Mutator
   void registerNumber  (ContactMethod* number); //FIXME this should be private
   void unregisterNumber(ContactMethod* number);

private:
   explicit NumberCategoryModel(QObject* parent = nullptr);
   ~NumberCategoryModel();

   QScopedPointer<NumberCategoryModelPrivate> d_ptr;

   //Re-implementation
   virtual void collectionAddedCallback(CollectionInterface* collection) override;
   virtual bool addItemCallback(const ContactMethod* item) override;
   virtual bool removeItemCallback(const ContactMethod* item) override;

   //Singleton
   static NumberCategoryModel* m_spInstance;
};

#endif //NUMBERCATEGORYMODEL_H
