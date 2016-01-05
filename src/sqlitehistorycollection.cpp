/************************************************************************************
 *   Copyright (C) 2014-2016 by Savoir-faire Linux                                  *
 *   Author : Edric Milaret <edric.ladent-milaret@savoirfairelinux.com              *
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

#include "sqlitehistorycollection.h"

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
#include "globalinstances.h"
#include "interfaces/pixmapmanipulatori.h"
#include "database/sqlmanager.h"

class SqliteHistoryEditor final : public CollectionEditor<Call>
{
public:
    SqliteHistoryEditor(CollectionMediator<Call>* m, SqliteHistoryCollection* parent);
    virtual bool save       ( const Call* item ) override;
    virtual bool remove     ( const Call* item ) override;
    virtual bool edit       ( Call*       item ) override;
    virtual bool addNew     ( Call*       item ) override;
    virtual bool addExisting( const Call* item ) override;

private:
    virtual QVector<Call*> items() const override;

    //Helpers
    void saveCall(const Call* call);
    bool removeCall(const Call* toRemove);

    //Attributes
    QVector<Call*> m_lItems;
    SqliteHistoryCollection* m_pCollection;
};

SqliteHistoryEditor::SqliteHistoryEditor(CollectionMediator<Call>* m, SqliteHistoryCollection* parent) :
    CollectionEditor<Call>(m),m_pCollection(parent)
{
}

SqliteHistoryCollection::SqliteHistoryCollection(CollectionMediator<Call>* mediator) :
    CollectionInterface(new SqliteHistoryEditor(mediator,this)),m_pMediator(mediator)
{

    if (SqlManager::instance().isOpen()) {
        QSqlQuery createQuery(QString("CREATE TABLE IF NOT EXISTS call_history (%1 TEXT UNIQUE, %2 TEXT, %3 TEXT, %4 TEXT, %5 TEXT, %6 TEXT, %7 TEXT, %8 INTEGER, %9 INTEGER, %10 TEXT, %11 TEXT, %12 TEXT)")
                              .arg(Call::HistoryMapFields::CALLID, Call::HistoryMapFields::TIMESTAMP_START, Call::HistoryMapFields::TIMESTAMP_STOP, Call::HistoryMapFields::ACCOUNT_ID,
                                   Call::HistoryMapFields::DISPLAY_NAME, Call::HistoryMapFields::PEER_NUMBER, Call::HistoryMapFields::DIRECTION, Call::HistoryMapFields::MISSED,
                                   Call::HistoryMapFields::CONTACT_USED).arg(Call::HistoryMapFields::RECORDING_PATH, Call::HistoryMapFields::CONTACT_UID, Call::HistoryMapFields::CERT_PATH));
        qDebug() << "History database opened table state : " << createQuery.exec();

        auto filePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') +"history.ini";
        if (QFile::exists(filePath)) {
            legacyImport(filePath);
        }
    } else {
        qDebug() << "Error opening history database";
    }
}

void SqliteHistoryCollection::legacyImport(const QString& filePath)
{
    QFile file(filePath);
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
             editor<Call>()->addNew(pastCall);
             hc.clear();
          }
          // Add to the current set
          else {
             const int idx = line.indexOf("=");
             if (idx >= 0)
                hc[line.left(idx)] = line.right(line.size()-idx-1);
          }
       }
       QFile::remove(filePath);
    }
    else
       qWarning() << "Could not import legacy historic file";
}

SqliteHistoryCollection::~SqliteHistoryCollection()
{
}

void SqliteHistoryEditor::saveCall(const Call* call)
{
    if (SqlManager::instance().isOpen()) {

        const QString direction = (call->direction()==Call::Direction::INCOMING)?
                    Call::HistoryStateName::INCOMING : Call::HistoryStateName::OUTGOING;

        const Account* a = call->account();

        QSqlQuery query;
        query.prepare(QString("INSERT INTO call_history (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12) "
                              "VALUES (:%1, :%2, :%3, :%4, :%5, :%6, :%7, :%8, :%9, :%10, :%11, :%12)")
                      .arg(Call::HistoryMapFields::CALLID, Call::HistoryMapFields::TIMESTAMP_START, Call::HistoryMapFields::TIMESTAMP_STOP, Call::HistoryMapFields::ACCOUNT_ID,
                           Call::HistoryMapFields::DISPLAY_NAME, Call::HistoryMapFields::PEER_NUMBER, Call::HistoryMapFields::DIRECTION, Call::HistoryMapFields::MISSED,
                           Call::HistoryMapFields::CONTACT_USED).arg(Call::HistoryMapFields::RECORDING_PATH, Call::HistoryMapFields::CONTACT_UID, Call::HistoryMapFields::CERT_PATH));

        query.bindValue(0, call->historyId());
        query.bindValue(1, QString::number(call->startTimeStamp()));
        query.bindValue(2, QString::number(call->stopTimeStamp()));
        query.bindValue(3, a?QString(a->id()):"");
        query.bindValue(4, call->peerName());
        query.bindValue(5, call->peerContactMethod()->uri());
        query.bindValue(6, direction);
        query.bindValue(7, call->isMissed());
        query.bindValue(8, false);
        query.bindValue(9, call->hasRecording(Media::Media::Type::AUDIO,Media::Media::Direction::IN) ? ((Media::AVRecording*)call->recordings(Media::Media::Type::AUDIO,Media::Media::Direction::IN)[0])->path().path() : "");
        query.bindValue(10, call->peerContactMethod()->contact() ? call->peerContactMethod()->contact()->uid() : "");
        query.bindValue(11, call->certificate() ? call->certificate()->path() : "");
        if (!query.exec()) {
            qWarning() << "Unable to save history";
        }
    } else
        qWarning() << "Unable to save history";
}

bool SqliteHistoryEditor::removeCall(const Call* toRemove)
{
    if (SqlManager::instance().isOpen() && toRemove != nullptr) {
        QSqlQuery query;
        query.prepare("DELETE FROM call_history WHERE callid=%1");
        query.bindValue(0, toRemove->historyId());
        return query.exec();
    }
    return false;
}

bool SqliteHistoryEditor::save(const Call* call)
{
    if (call->collection()->editor<Call>() != this)
        return addNew(const_cast<Call*>(call));

    return false;
}

bool SqliteHistoryEditor::remove(const Call* item)
{
    if (removeCall(item)) {
        mediator()->removeItem(item);
        return true;
    }
    return false;
}

bool SqliteHistoryEditor::edit( Call* item)
{
    Q_UNUSED(item)
    return false;
}

bool SqliteHistoryEditor::addNew( Call* call)
{
    if ((call->collection() && call->collection()->editor<Call>() == this)  || call->historyId().isEmpty()) return false;

    saveCall(call);

    const_cast<Call*>(call)->setCollection(m_pCollection);
    addExisting(call);
    return true;
}

bool SqliteHistoryEditor::addExisting(const Call* item)
{
    m_lItems << const_cast<Call*>(item);
    mediator()->addItem(item);
    return true;
}

QVector<Call*> SqliteHistoryEditor::items() const
{
    return m_lItems;
}

QString SqliteHistoryCollection::name () const
{
    return QObject::tr("Local history");
}

QString SqliteHistoryCollection::category () const
{
    return QObject::tr("History");
}

QVariant SqliteHistoryCollection::icon() const
{
    return GlobalInstances::pixmapManipulator().collectionIcon(this,Interfaces::PixmapManipulatorI::CollectionIconHint::HISTORY);
}

bool SqliteHistoryCollection::isEnabled() const
{
    return true;
}

bool SqliteHistoryCollection::load()
{
    if (SqlManager::instance().isOpen()) {
        QMap<QString,QString> hc;
        QSqlQuery query;
        query.setForwardOnly(true);
        query.prepare("SELECT * FROM call_history");
        query.exec();
        QMap<QString, int> indexOfFields;
        indexOfFields[Call::HistoryMapFields::CALLID] = query.record().indexOf(Call::HistoryMapFields::CALLID);
        indexOfFields[Call::HistoryMapFields::TIMESTAMP_START] = query.record().indexOf(Call::HistoryMapFields::TIMESTAMP_START);
        indexOfFields[Call::HistoryMapFields::TIMESTAMP_STOP] = query.record().indexOf(Call::HistoryMapFields::TIMESTAMP_STOP);
        indexOfFields[Call::HistoryMapFields::ACCOUNT_ID] = query.record().indexOf(Call::HistoryMapFields::ACCOUNT_ID);
        indexOfFields[Call::HistoryMapFields::DISPLAY_NAME] = query.record().indexOf(Call::HistoryMapFields::DISPLAY_NAME);
        indexOfFields[Call::HistoryMapFields::PEER_NUMBER] = query.record().indexOf(Call::HistoryMapFields::PEER_NUMBER);
        indexOfFields[Call::HistoryMapFields::DIRECTION] = query.record().indexOf(Call::HistoryMapFields::DIRECTION);
        indexOfFields[Call::HistoryMapFields::MISSED] = query.record().indexOf(Call::HistoryMapFields::MISSED);
        indexOfFields[Call::HistoryMapFields::CONTACT_USED] = query.record().indexOf(Call::HistoryMapFields::CONTACT_USED);
        while (query.next()) {
            QMapIterator<QString, int> i(indexOfFields);
            while (i.hasNext()) {
                i.next();
                hc[i.key()] = query.value(i.value()).toString();
            }
            Call* pastCall = Call::buildHistoryCall(hc);
            pastCall->setCollection(this);
            editor<Call>()->addExisting(pastCall);
            hc.clear();
        }
        return true;
    } else
        qWarning() << "History doesn't exist or is not readable";
    return false;
}

bool SqliteHistoryCollection::reload()
{
    return false;
}

FlagPack<CollectionInterface::SupportedFeatures> SqliteHistoryCollection::supportedFeatures() const
{
    return
            CollectionInterface::SupportedFeatures::NONE       |
            CollectionInterface::SupportedFeatures::LOAD       |
            CollectionInterface::SupportedFeatures::CLEAR      |
            CollectionInterface::SupportedFeatures::REMOVE     |
            CollectionInterface::SupportedFeatures::MANAGEABLE |
            CollectionInterface::SupportedFeatures::ADD        ;
}

bool SqliteHistoryCollection::clear()
{
    QSqlQuery query("DELETE FROM call_history");
    return query.exec();
}

QByteArray SqliteHistoryCollection::id() const
{
    return "sqhb";
}
