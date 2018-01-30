/****************************************************************************
 *   Copyright (C) 2013-2018 Savoir-faire Linux                          *
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

#include <collectioninterface.h>

#include "typedefs.h"

class QPixmap;
class NumberCategoryPrivate;
class ContactMethod;
class NumberCategoryModel;
template <typename T> class CollectionManagerInterface;

/**
 * This class represent a ContactMethod category. Categories usually
 * come from the contact provider, but can be added dynamically too
 */
class LIB_EXPORT NumberCategory : public CollectionInterface {

public:
   friend class NumberCategoryModel;
   friend class CollectionManagerInterface<ContactMethod>;

   virtual QString    name     () const override;
   virtual QString    category () const override;
   virtual QVariant   icon     () const override;
   virtual bool       isEnabled() const override;
   virtual QByteArray id       () const override;
   virtual int        size     () const override;

   //Getters
   int      key (                                      ) const;
   QVariant icon(bool isTracked, bool isPresent = false) const;


   virtual FlagPack<SupportedFeatures> supportedFeatures() const override;
   virtual bool load() override;

   //Setter
   void setIcon(const QVariant& pixmap );
   void setName(const QString&  name   );
   void setKey (const int       key    );

private:
   NumberCategory(CollectionMediator<ContactMethod>* mediator, const QString& name);
   virtual ~NumberCategory();

   const QScopedPointer<NumberCategoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(NumberCategory)

};
