/****************************************************************************
 *   Copyright (C) 2015-2018 Savoir-faire Linux                               *
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
#include "localrecordingcollection.h"

//Qt
#include <QtCore/QUrl>

#include <globalinstances.h>
#include <interfaces/pixmapmanipulatori.h>
#include <media/recordingmodel.h>
#include <media/recording.h>
#include <media/avrecording.h>

class LocalRecordingEditor final : public CollectionEditor<LRCMedia::Recording>
{
public:
   LocalRecordingEditor(CollectionMediator<LRCMedia::Recording>* m) : CollectionEditor<LRCMedia::Recording>(m) {}
   virtual bool save       ( const LRCMedia::Recording* item ) override;
   virtual bool remove     ( const LRCMedia::Recording* item ) override;
   virtual bool edit       (LRCMedia::Recording*       item ) override;
   virtual bool addNew     (LRCMedia::Recording*       item ) override;
   virtual bool addExisting( const LRCMedia::Recording* item ) override;

private:
   virtual QVector<LRCMedia::Recording*> items() const override;
   //Attributes
   QVector<LRCMedia::Recording*> m_lNumbers;
};

LocalRecordingCollection::LocalRecordingCollection(CollectionMediator<LRCMedia::Recording>* mediator) :
   CollectionInterface(new LocalRecordingEditor(mediator))
{
   load();
}

LocalRecordingCollection::~LocalRecordingCollection()
{

}

LocalRecordingCollection& LocalRecordingCollection::instance()
{
   static auto instance = LRCMedia::RecordingModel::instance().addCollection<LocalRecordingCollection>();
   return *instance;
}

bool LocalRecordingEditor::save(const LRCMedia::Recording* recording)
{
   Q_UNUSED(recording)
   return true;
}

bool LocalRecordingEditor::remove(const LRCMedia::Recording* item)
{
   Q_UNUSED(item)
   //TODO
   return false;
}

bool LocalRecordingEditor::edit(LRCMedia::Recording* item)
{
   Q_UNUSED(item)
   return false;
}

bool LocalRecordingEditor::addNew(LRCMedia::Recording* item)
{
   addExisting(item);
   return save(item);
}

bool LocalRecordingEditor::addExisting(const LRCMedia::Recording* item)
{
   m_lNumbers << const_cast<LRCMedia::Recording*>(item);
   mediator()->addItem(item);
   return false;
}

QVector<LRCMedia::Recording*> LocalRecordingEditor::items() const
{
   return m_lNumbers;
}

QString LocalRecordingCollection::name () const
{
   return QObject::tr("Local recordings");
}

QString LocalRecordingCollection::category () const
{
   return QObject::tr("Recording");
}

QVariant LocalRecordingCollection::icon() const
{
   return GlobalInstances::pixmapManipulator().collectionIcon(this,Interfaces::PixmapManipulatorI::CollectionIconHint::RECORDING);
}

bool LocalRecordingCollection::isEnabled() const
{
   return true;
}

bool LocalRecordingCollection::load()
{
   //This collection is special as it use the history collection
   //as its source, there is no loading
   return true;
}

bool LocalRecordingCollection::reload()
{
   return false;
}

FlagPack<CollectionInterface::SupportedFeatures> LocalRecordingCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE      |
      CollectionInterface::SupportedFeatures::LOAD      |
      CollectionInterface::SupportedFeatures::ADD       |
      CollectionInterface::SupportedFeatures::MANAGEABLE|
      CollectionInterface::SupportedFeatures::REMOVE    ;
}

bool LocalRecordingCollection::clear()
{
   return false;
}

QByteArray LocalRecordingCollection::id() const
{
   return "localrecording";
}

LRCMedia::Recording* LocalRecordingCollection::addFromPath(const QString& path)
{
    LRCMedia::AVRecording* rec = new LRCMedia::AVRecording();
   rec->setPath(path);

   editor<LRCMedia::Recording>()->addExisting(rec);
   return rec;
}
