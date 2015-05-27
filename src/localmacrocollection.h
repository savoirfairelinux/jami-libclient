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
#ifndef LOCALMACROCOLLECTION_H
#define LOCALMACROCOLLECTION_H

#include <collectioninterface.h>

class Macro;

class LocalMacroCollectionPrivate;

class LIB_EXPORT LocalMacroCollection : public CollectionInterface
{
public:
   explicit LocalMacroCollection(CollectionMediator<Macro>* mediator);
   virtual ~LocalMacroCollection();

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
   LocalMacroCollectionPrivate* d_ptr;
   Q_DECLARE_PRIVATE(LocalMacroCollection)
};

#endif
