/***************************************************************************
 * Copyright (C) 2016 by Savoir-faire Linux                                *
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>*
 * Author: Stepan Salenikovich <stepan.salenikovich@savoirfairelinux.com>  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 **************************************************************************/

#pragma once

#include "collectioninterface.h"
#include "collectioneditor.h"

class Person;

template<typename T> class CollectionMediator;

/**
 * The PendingPeerProfileCollection stores the vCards of peers which have sent a trust request but
 * have not yet been accepted. Thus their certificates should not be trusted. Once they are trusted
 * they should be moved to the PeerProfileCollection. If they are banned, they should be deleted or
 * moved to a banned collection (TODO)
 */
class LIB_EXPORT PendingPeerProfileCollection : public CollectionInterface
{
public:
   explicit PendingPeerProfileCollection(CollectionMediator<Person>* mediator);
   virtual ~PendingPeerProfileCollection();

   virtual bool load () override;
   virtual bool reload() override;
   virtual bool clear () override;

   virtual QString    name     () const override;
   virtual QString    category () const override;
   virtual QVariant   icon     () const override;
   virtual bool       isEnabled() const override;
   virtual QByteArray id       () const override;

   virtual FlagPack<SupportedFeatures> supportedFeatures() const override;

};
