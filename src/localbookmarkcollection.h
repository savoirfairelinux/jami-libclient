/************************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                                  *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/
#ifndef LOCALBOOKMARKCOLLECTION_H
#define LOCALBOOKMARKCOLLECTION_H

#include <collectioninterface.h>

class ContactMethod;

class LocalBookmarkCollectionPrivate;

class LIB_EXPORT LocalBookmarkCollection : public CollectionInterface
{
public:
   explicit LocalBookmarkCollection(CollectionMediator<ContactMethod>* mediator);
   virtual ~LocalBookmarkCollection();

   virtual bool load  () override;
   virtual bool reload() override;
   virtual bool clear () override;

   virtual QString    name     () const override;
   virtual QString    category () const override;
   virtual QVariant   icon     () const override;
   virtual bool       isEnabled() const override;
   virtual QByteArray id       () const override;

   virtual FlagPack<SupportedFeatures> supportedFeatures() const override;

   //Setter
   void setPresenceTracked(bool tracked);

   //Getter
   bool isPresenceTracked() const;

private:
   LocalBookmarkCollectionPrivate* d_ptr;
   Q_DECLARE_PRIVATE(LocalBookmarkCollection)
};

#endif
