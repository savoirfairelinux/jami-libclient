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
#include "localringtonecollection.h"

//Qt
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QStandardPaths>

//Ring
#include <collectioneditor.h>
#include <ringtonemodel.h>
#include <ringtone.h>
#include <delegates/pixmapmanipulationdelegate.h>

namespace Serializable {
   class RingtoneNode
   {
   public:
      Ringtone* ringtone;

      void read (const QJsonObject &json);
      void write(QJsonObject       &json);
   };
}

class LocalRingtoneEditor : public CollectionEditor<Ringtone>
{
public:
   LocalRingtoneEditor(CollectionMediator<Ringtone>* m) : CollectionEditor<Ringtone>(m),m_Tracked(false) {}
   virtual bool save       ( const Ringtone* item ) override;
   virtual bool remove     ( const Ringtone* item ) override;
   virtual bool addNew     ( const Ringtone* item ) override;
   virtual bool addExisting( const Ringtone* item ) override;

   //Attributes
   QVector<Ringtone*> m_lNumbers;
   QList<Serializable::RingtoneNode> m_Nodes;
   bool m_Tracked;

private:
   virtual QVector<Ringtone*> items() const override;
};

class LocalRingtoneCollectionPrivate
{
public:

   //Attributes
   constexpr static const char FILENAME[] = "ringtone.json";
};

constexpr const char LocalRingtoneCollectionPrivate::FILENAME[];

LocalRingtoneCollection::LocalRingtoneCollection(CollectionMediator<Ringtone>* mediator) :
   CollectionInterface(new LocalRingtoneEditor(mediator))
{
   load();
}


LocalRingtoneCollection::~LocalRingtoneCollection()
{

}

bool LocalRingtoneCollection::load()
{
   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') +LocalRingtoneCollectionPrivate::FILENAME);
   LocalRingtoneEditor* e = static_cast<LocalRingtoneEditor*>(editor<Ringtone>());

   if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
      const QByteArray content = file.readAll();
      QJsonDocument loadDoc = QJsonDocument::fromJson(content);
      QJsonArray a = loadDoc.array();
      //QHash<QString,bool> uniqueRingtone; //TODO make sure there is no duplicates

      //TODO implement custom ringtone path, that's the code to load/save them
      for (int i = 0; i < a.size(); ++i) {
         QJsonObject o = a[i].toObject();
         Serializable::RingtoneNode n;
         n.read(o);

         n.ringtone->setCollection(this);
         e->addExisting(n.ringtone);

         e->m_Nodes << n;
      }
   }
   else
      qWarning() << "Ringtones doesn't exist or is not readable";

//TODO remove that and do a proper collection for each platforms
#ifdef Q_OS_LINUX
   QDir ringtonesDir(QFileInfo(QCoreApplication::applicationFilePath()).path()+"/../share/ring/ringtones/");
#elif defined(Q_OS_WIN)
   QDir ringtonesDir(QFileInfo(QCoreApplication::applicationFilePath()).path()+"/ringtones/");
#elif defined(Q_OS_OSX)
   QDir ringtonesDir(QFileInfo(QCoreApplication::applicationFilePath()).path()+"/ringtones/"); //FIXME
#endif

   if(!ringtonesDir.exists())
      return true;

   const QStringList entries = ringtonesDir.entryList({"*.wav", "*.ul", "*.au", ".mp3", ".flac"}, QDir::Files);
   for (const QString& item : entries) {
      QFileInfo fileinfo(ringtonesDir.absolutePath()+item);
      Ringtone* info = new Ringtone();
      info->setPath(fileinfo.absoluteFilePath());
      info->setName(item);
      e->addExisting(info);
   }

   return true;
}

bool LocalRingtoneEditor::save(const Ringtone* ringtone)
{
   Q_UNUSED(ringtone)

   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/')+LocalRingtoneCollectionPrivate::FILENAME);
   if ( file.open(QIODevice::WriteOnly | QIODevice::Text) ) {

      QJsonArray a;
      for (Serializable::RingtoneNode& g : m_Nodes) {
         QJsonObject o;
         g.write(o);
         a.append(o);
      }

      QJsonDocument doc(a);

      QTextStream streamFileOut(&file);
      streamFileOut << doc.toJson();
      streamFileOut.flush();
      file.close();

      return true;
   }
   else
      qWarning() << "Unable to save ringtones";

   return false;
}

bool LocalRingtoneEditor::remove(const Ringtone* item)
{
   Q_UNUSED(item)

   if (m_lNumbers.indexOf(const_cast<Ringtone*>(item)) != -1) {
      m_lNumbers.removeAt(m_lNumbers.indexOf(const_cast<Ringtone*>(item)));
      mediator()->removeItem(item);

      for (int i =0;i<m_Nodes.size();i++) {
         if (m_Nodes[i].ringtone == item) {
            m_Nodes.removeAt(i);
            break;
         }
      }

      return save(nullptr);
   }
   return false;
}

bool LocalRingtoneEditor::addNew(const Ringtone* ringtone)
{
   Serializable::RingtoneNode n;

   n.ringtone = const_cast<Ringtone*>(ringtone);
   m_Nodes << n;

   if (!save(ringtone))
      qWarning() << "Unable to save ringtones";

   addExisting(ringtone);
   return save(ringtone);
}

bool LocalRingtoneEditor::addExisting(const Ringtone* item)
{
   m_lNumbers << const_cast<Ringtone*>(item);
   mediator()->addItem(item);
   return false;
}

QVector<Ringtone*> LocalRingtoneEditor::items() const
{
   return m_lNumbers;
}

QString LocalRingtoneCollection::name () const
{
   return QObject::tr("Local ringtones");
}

QString LocalRingtoneCollection::category () const
{
   return QObject::tr("Ringtone");
}

QVariant LocalRingtoneCollection::icon() const
{
   return PixmapManipulationDelegate::instance()->collectionIcon(this,PixmapManipulationDelegate::CollectionIconHint::MACRO);
}

bool LocalRingtoneCollection::isEnabled() const
{
   return true;
}

bool LocalRingtoneCollection::reload()
{
   return false;
}

FlagPack<CollectionInterface::SupportedFeatures> LocalRingtoneCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE      |
      CollectionInterface::SupportedFeatures::LOAD      |
      CollectionInterface::SupportedFeatures::CLEAR     |
      CollectionInterface::SupportedFeatures::ADD       |
      CollectionInterface::SupportedFeatures::MANAGEABLE|
      CollectionInterface::SupportedFeatures::REMOVE    ;
}

bool LocalRingtoneCollection::clear()
{
   return QFile::remove(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + LocalRingtoneCollectionPrivate::FILENAME);
}

QByteArray LocalRingtoneCollection::id() const
{
   return "localringtone";
}

void Serializable::RingtoneNode::read(const QJsonObject &json)
{
   ringtone = new Ringtone();
   ringtone->setPath(json["path"].toString());
}

void Serializable::RingtoneNode::write(QJsonObject& json)
{
   json["path"] = ringtone->path().path();
}

#include <localringtonecollection.moc>

