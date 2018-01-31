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
#include "localtextrecordingcollection.h"

//Qt
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

//Ring
#include <globalinstances.h>
#include <interfaces/pixmapmanipulatori.h>
#include <media/recordingmodel.h>
#include <media/recording.h>
#include <media/textrecording.h>
#include <private/textrecording_p.h>
#include <private/contactmethod_p.h>
#include <media/media.h>

/*
 * This collection store and load the instant messaging conversations. Lets call
 * them "imc" for this section. An imc is a graph of one or more groups. Groups
 * are concatenated into a json file with the ContactMethod sha1 as filename.
 * Each group has a next group. If the next group is null, then the next one in
 * file will be used. If it isn't then another json file can be linked. Once the
 * second file has no next group, the next group from the first file is used.
 *
 * If more than 1 peer is part of the conversation, then their hash are
 * concatenated then hashed in sha1 again.
 */

class LocalTextRecordingEditor final : public CollectionEditor<Media::Recording>
{
public:
   LocalTextRecordingEditor(CollectionMediator<Media::Recording>* m) : CollectionEditor<Media::Recording>(m) {}
   virtual bool save       ( const Media::Recording* item ) override;
   virtual bool remove     ( const Media::Recording* item ) override;
   virtual bool edit       ( Media::Recording*       item ) override;
   virtual bool addNew     ( Media::Recording*       item ) override;
   virtual bool addExisting( const Media::Recording* item ) override;
   QString fetch(const QByteArray& sha1);

   void clearAll();

private:
   virtual QVector<Media::Recording*> items() const override;
   //Attributes
   QVector<Media::Recording*> m_lNumbers;
};

LocalTextRecordingCollection::LocalTextRecordingCollection(CollectionMediator<Media::Recording>* mediator) :
   CollectionInterface(new LocalTextRecordingEditor(mediator))
{
   load();
}

LocalTextRecordingCollection::~LocalTextRecordingCollection()
{

}

LocalTextRecordingCollection& LocalTextRecordingCollection::instance()
{
   static auto instance = Media::RecordingModel::instance().addCollection<LocalTextRecordingCollection>();
   return *instance;
}

bool LocalTextRecordingEditor::save(const Media::Recording* recording)
{
   Q_UNUSED(recording)
   QHash<QByteArray,QByteArray> ret = static_cast<const Media::TextRecording*>(recording)->d_ptr->toJsons();

   QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

   //Make sure the directory exist
   dir.mkdir("text/");

   //Save each file
   for (QHash<QByteArray,QByteArray>::const_iterator i = ret.begin(); i != ret.end(); ++i) {
      QFile file(QString("%1/text/%2.json").arg(dir.path()).arg(QString(i.key())));

      if ( file.open(QIODevice::WriteOnly | QIODevice::Text) ) {
         QTextStream streamFileOut(&file);
         streamFileOut.setCodec("UTF-8");
         streamFileOut << i.value();
         streamFileOut.flush();
         file.close();
      }
   }

   return true;
}

void LocalTextRecordingEditor::clearAll()
{
    for (Media::Recording *recording : items()) {
        auto textRecording = qobject_cast<Media::TextRecording*>(recording);
        textRecording->d_ptr->clear();
        save(recording);
    }
}

bool LocalTextRecordingEditor::remove(const Media::Recording* item)
{
   Q_UNUSED(item)
   //TODO
   return false;
}

bool LocalTextRecordingEditor::edit( Media::Recording* item)
{
   Q_UNUSED(item)
   return false;
}

bool LocalTextRecordingEditor::addNew( Media::Recording* item)
{
   Q_UNUSED(item)
   addExisting(item);
   return save(item);
}

bool LocalTextRecordingEditor::addExisting(const Media::Recording* item)
{
   m_lNumbers << const_cast<Media::Recording*>(item);
   mediator()->addItem(item);
   return false;
}

QString LocalTextRecordingEditor::fetch(const QByteArray& sha1)
{
   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/text/" + sha1 + ".json");

   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return QByteArray();
   }

   return QString::fromUtf8(file.readAll());
}

QVector<Media::Recording*> LocalTextRecordingEditor::items() const
{
   return m_lNumbers;
}

QString LocalTextRecordingCollection::name () const
{
   return QObject::tr("Local text recordings");
}

QString LocalTextRecordingCollection::category () const
{
   return QObject::tr("Recording");
}

QVariant LocalTextRecordingCollection::icon() const
{
   return GlobalInstances::pixmapManipulator().collectionIcon(this,Interfaces::PixmapManipulatorI::CollectionIconHint::RECORDING);
}

bool LocalTextRecordingCollection::isEnabled() const
{
   return true;
}

bool LocalTextRecordingCollection::load()
{
    // load all text recordings so we can recover CMs that are not in the call history
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/text/");
    if (dir.exists()) {
        // get .json files, sorted by time, latest first
        QStringList filters;
        filters << "*.json";
        auto list = dir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Time);

        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);

            QString content;
            QFile file(fileInfo.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                content = QString::fromUtf8(file.readAll());
            } else {
                qWarning() << "Could not open text recording json file";
            }

            if (!content.isEmpty()) {
                QJsonParseError err;
                QJsonDocument loadDoc = QJsonDocument::fromJson(content.toUtf8(), &err);

                if (err.error == QJsonParseError::ParseError::NoError) {
                    Media::TextRecording* r = Media::TextRecording::fromJson({loadDoc.object()}, nullptr, this);

                    editor<Media::Recording>()->addExisting(r);

                    // get CMs from recording
                    for (ContactMethod *cm : r->peers()) {
                        // since we load the recordings in order from newest to oldest, if there is
                        // more than one found associated with a CM, we take the newest one
                        if (!cm->d_ptr->m_pTextRecording) {
                            cm->d_ptr->setTextRecording(r);
                        } else {
                            qWarning() << "CM already has text recording" << cm;
                        }
                    }
                } else {
                    qWarning() << "Error Decoding Text Message History Json" << err.errorString();
                }
            } else {
                qWarning() << "Text recording file is empty";
            }
        }
    }

    // always return true, even if noting was loaded, since the collection can still be used to
    // save files
    return true;
}

bool LocalTextRecordingCollection::reload()
{
   return false;
}

FlagPack<CollectionInterface::SupportedFeatures> LocalTextRecordingCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE      |
      CollectionInterface::SupportedFeatures::LOAD      |
      CollectionInterface::SupportedFeatures::ADD       |
      CollectionInterface::SupportedFeatures::SAVE      |
      CollectionInterface::SupportedFeatures::MANAGEABLE|
      CollectionInterface::SupportedFeatures::SAVE_ALL  |
      CollectionInterface::SupportedFeatures::LISTABLE  |
      CollectionInterface::SupportedFeatures::REMOVE    |
      CollectionInterface::SupportedFeatures::CLEAR     ;
}

bool LocalTextRecordingCollection::clear()
{
    static_cast<LocalTextRecordingEditor *>(editor<Media::Recording>())->clearAll();

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/text");

    // TODO: the file deletion should be done on each individual file to be able to catch errors
    // and to prevent us deleting files which are not the recordings
    return dir.removeRecursively();
}

QByteArray LocalTextRecordingCollection::id() const
{
   return "localtextrecording";
}

bool LocalTextRecordingCollection::listId(std::function<void(const QList<Element>)> callback) const
{
   QList<Element> list;

   QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/text/");

   if (!dir.exists())
      return false;

   for (const QString& str : dir.entryList({"*.json"}) ) {
      list << str.toLatin1();
   }

   callback(list);
   return true;
}

QList<CollectionInterface::Element> LocalTextRecordingCollection::listId() const
{
   return {};
}

bool LocalTextRecordingCollection::fetch(const Element& e)
{
   Q_UNUSED(e);
   return false;
}

bool LocalTextRecordingCollection::fetch( const QList<CollectionInterface::Element>& elements)
{
   Q_UNUSED(elements)
   return false;
}

Media::TextRecording* LocalTextRecordingCollection::fetchFor(const ContactMethod* cm)
{
   const QByteArray& sha1 = cm->sha1();
   const QString content = static_cast<LocalTextRecordingEditor*>(editor<Media::Recording>())->fetch(sha1);

   if (content.isEmpty())
      return nullptr;

   QJsonParseError err;
   QJsonDocument loadDoc = QJsonDocument::fromJson(content.toUtf8(), &err);

   if (err.error != QJsonParseError::ParseError::NoError) {
       qWarning() << "Error Decoding Text Message History Json" << err.errorString();
       return nullptr;
   }

   Media::TextRecording* r = Media::TextRecording::fromJson({loadDoc.object()}, cm, this);

   editor<Media::Recording>()->addExisting(r);

   return r;
}

Media::TextRecording* LocalTextRecordingCollection::createFor(const ContactMethod* cm)
{
   Media::TextRecording* r = fetchFor(cm);

   if (!r) {
      r = new Media::TextRecording();
      r->setCollection(this);
      cm->d_ptr->setTextRecording(r);
   }

   return r;
}
