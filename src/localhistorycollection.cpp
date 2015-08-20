/************************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                                       *
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
#include "localhistorycollection.h"

//Qt
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QStandardPaths>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>

//Ring
#include "call.h"
#include "media/media.h"
#include "media/recording.h"
#include "media/avrecording.h"
#include "account.h"
#include "person.h"
#include "certificate.h"
#include "contactmethod.h"
#include "categorizedhistorymodel.h"
#include "delegates/delegatemanager.h"
#include "delegates/pixmapmanipulationdelegate.h"

class LocalHistoryEditor final : public CollectionEditor<Call>
{
public:
   LocalHistoryEditor(CollectionMediator<Call>* m, LocalHistoryCollection* parent);
   virtual bool save       ( const Call* item ) override;
   virtual bool remove     ( const Call* item ) override;
   virtual bool edit       ( Call*       item ) override;
   virtual bool addNew     ( Call*       item ) override;
   virtual bool addExisting( const Call* item ) override;

private:
   virtual QVector<Call*> items() const override;

   //Helpers
   void saveCall(QTextStream& stream, const Call* call);
   bool regenFile(const Call* toIgnore);

   //Attributes
   QVector<Call*> m_lItems;
   LocalHistoryCollection* m_pCollection;
};

LocalHistoryEditor::LocalHistoryEditor(CollectionMediator<Call>* m, LocalHistoryCollection* parent) :
CollectionEditor<Call>(m),m_pCollection(parent)
{

}

LocalHistoryCollection::LocalHistoryCollection(CollectionMediator<Call>* mediator) :
CollectionInterface(new LocalHistoryEditor(mediator,this)),m_pMediator(mediator)
{
//    setObjectName("LocalHistoryCollection");
}

LocalHistoryCollection::~LocalHistoryCollection()
{

}

void LocalHistoryEditor::saveCall(QTextStream& stream, const Call* call)
{
   const QString direction = (call->direction()==Call::Direction::INCOMING)?
      Call::HistoryStateName::INCOMING : Call::HistoryStateName::OUTGOING;

   const Account* a = call->account();
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::CALLID          ).arg(call->historyId()                     );
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::TIMESTAMP_START ).arg(call->startTimeStamp()         );
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::TIMESTAMP_STOP  ).arg(call->stopTimeStamp()          );
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::ACCOUNT_ID      ).arg(a?QString(a->id()):""          );
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::DISPLAY_NAME    ).arg(call->peerName()               );
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::PEER_NUMBER     ).arg(call->peerContactMethod()->uri() );
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::DIRECTION       ).arg(direction                      );
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::MISSED          ).arg(call->isMissed()               );
   stream << QString("%1=%2\n").arg(Call::HistoryMapFields::CONTACT_USED    ).arg(false                          );//TODO

   //TODO handle more than one recording
   if (call->hasRecording(Media::Media::Type::AUDIO,Media::Media::Direction::IN)) {
      stream << QString("%1=%2\n").arg(Call::HistoryMapFields::RECORDING_PATH  ).arg(((Media::AVRecording*)call->recordings(Media::Media::Type::AUDIO,Media::Media::Direction::IN)[0])->path().path());
   }

   if (call->peerContactMethod()->contact()) {
      stream << QString("%1=%2\n").arg(Call::HistoryMapFields::CONTACT_UID  ).arg(
         QString(call->peerContactMethod()->contact()->uid())
      );
   }
   if (call->certificate())
      stream << QString("%1=%2\n").arg(Call::HistoryMapFields::CERT_PATH).arg(call->certificate()->path());
   stream << "\n";
   stream.flush();
}

bool LocalHistoryEditor::regenFile(const Call* toIgnore)
{
   QDir dir(QString('/'));
   dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + QString());

   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') +"history.ini");
   if ( file.open(QIODevice::WriteOnly | QIODevice::Text) ) {
      QTextStream stream(&file);
      for (const Call* c : CategorizedHistoryModel::instance()->getHistoryCalls()) {
         if (c != toIgnore)
            saveCall(stream, c);
      }
      file.close();
      return true;
   }
   return false;
}

bool LocalHistoryEditor::save(const Call* call)
{
   if (call->collection()->editor<Call>() != this)
      return addNew(const_cast<Call*>(call));

   return regenFile(nullptr);
}

bool LocalHistoryEditor::remove(const Call* item)
{
   if (regenFile(item)) {
      mediator()->removeItem(item);
      return true;
   }
   return false;
}

bool LocalHistoryEditor::edit( Call* item)
{
   Q_UNUSED(item)
   return false;
}

bool LocalHistoryEditor::addNew( Call* call)
{
   QDir dir(QString('/'));
   dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + QString());

   if ((call->collection() && call->collection()->editor<Call>() == this)  || call->historyId().isEmpty()) return false;
   //TODO support \r and \n\r end of line
   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/')+"history.ini");

   if ( file.open(QIODevice::Append | QIODevice::Text) ) {
      QTextStream streamFileOut(&file);
      saveCall(streamFileOut, call);
      file.close();

      const_cast<Call*>(call)->setCollection(m_pCollection);
      addExisting(call);
      return true;
   }
   else
      qWarning() << "Unable to save history";
   return false;
}

bool LocalHistoryEditor::addExisting(const Call* item)
{
   m_lItems << const_cast<Call*>(item);
   mediator()->addItem(item);
   return true;
}

QVector<Call*> LocalHistoryEditor::items() const
{
   return m_lItems;
}

QString LocalHistoryCollection::name () const
{
   return QObject::tr("Local history");
}

QString LocalHistoryCollection::category () const
{
   return QObject::tr("History");
}

QVariant LocalHistoryCollection::icon() const
{
   return getDelegateManager()->getPixmapManipulationDelegate()->collectionIcon(this,PixmapManipulationDelegate::CollectionIconHint::HISTORY);
}

bool LocalHistoryCollection::isEnabled() const
{
   return true;
}

bool LocalHistoryCollection::load()
{
   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') +"history.ini");
   if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
      QMap<QString,QString> hc;
      QStringList lines;

      while (!file.atEnd())
         lines << file.readLine().trimmed();
      file.close();

      for (const QString& line : lines) {
         //The item is complete
         if ((line.isEmpty() || !line.size()) && hc.size()) {
            Call* pastCall = Call::buildHistoryCall(hc);
            if (pastCall->peerName().isEmpty()) {
               pastCall->setPeerName(QObject::tr("Unknown"));
            }
            pastCall->setCollection(this);

            editor<Call>()->addExisting(pastCall);
            hc.clear();
         }
         // Add to the current set
         else {
            const int idx = line.indexOf("=");
            if (idx >= 0)
               hc[line.left(idx)] = line.right(line.size()-idx-1);
         }
      }
      return true;
   }
   else
      qWarning() << "History doesn't exist or is not readable";
   return false;
}

bool LocalHistoryCollection::reload()
{
   return false;
}

FlagPack<CollectionInterface::SupportedFeatures> LocalHistoryCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE       |
      CollectionInterface::SupportedFeatures::LOAD       |
      CollectionInterface::SupportedFeatures::CLEAR      |
      CollectionInterface::SupportedFeatures::REMOVE     |
      CollectionInterface::SupportedFeatures::MANAGEABLE |
      CollectionInterface::SupportedFeatures::ADD        ;
}

bool LocalHistoryCollection::clear()
{
   QFile::remove(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') +"history.ini");
   return true;
}

QByteArray LocalHistoryCollection::id() const
{
   return "mhb";
}
