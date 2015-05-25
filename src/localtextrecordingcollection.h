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
#ifndef LOCALTEXTRECORDINGCOLLECTION_H
#define LOCALTEXTRECORDINGCOLLECTION_H

#include <collectioninterface.h>
#include <collectioneditor.h>

#include <typedefs.h>

namespace Media {
   class Recording;
   class TextRecording;
}

class LIB_EXPORT LocalTextRecordingCollection : public CollectionInterface
{
public:
   explicit LocalTextRecordingCollection(CollectionMediator<Media::Recording>* mediator);
   virtual ~LocalTextRecordingCollection();

   virtual bool load  () override;
   virtual bool reload() override;
   virtual bool clear () override;

   virtual QString    name     () const override;
   virtual QString    category () const override;
   virtual QVariant   icon     () const override;
   virtual bool       isEnabled() const override;
   virtual QByteArray id       () const override;
   virtual bool fetch(const Element& e) override;
   virtual bool listId(std::function<void(const QList<Element>)> callback) const override;

   Media::TextRecording* fetchFor (const ContactMethod* cm);
   Media::TextRecording* createFor(const ContactMethod* cm);

   virtual FlagPack<SupportedFeatures> supportedFeatures() const override;

   static LocalTextRecordingCollection* instance();

};

#endif