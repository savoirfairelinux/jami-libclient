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

#include "sqlitetransfercollection.h"

//Ring
#include "transfer.h"
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

class SqliteTransferEditor final : public CollectionEditor<Transfer>
{
public:
    SqliteTransferEditor(CollectionMediator<Transfer>* m, SqliteTransferCollection* parent);
    virtual bool save       ( const Transfer* item ) override;
    virtual bool remove     ( const Transfer* item ) override;
    virtual bool edit       ( Transfer*       item ) override;
    virtual bool addNew     ( Transfer*       item ) override;
    virtual bool addExisting( const Transfer* item ) override;

private:
    virtual QVector<Transfer*> items() const override;

    //Helpers
    void saveTransfer(const Transfer* transfer);
    bool removeTransfer(const Transfer* toRemove);

    //Attributes
    QVector<Transfer*> m_lItems;
    SqliteTransferCollection* m_pCollection;
};

SqliteTransferEditor::SqliteTransferEditor(CollectionMediator<Transfer>* m, SqliteTransferCollection* parent) :
    CollectionEditor<Transfer>(m),m_pCollection(parent)
{
}

SqliteTransferCollection::SqliteTransferCollection(CollectionMediator<Transfer>* mediator) :
    CollectionInterface(new SqliteTransferEditor(mediator,this)),m_pMediator(mediator)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + "lrc.db3");
    if (db.open()) {
        QSqlQuery createQuery(QString("CREATE TABLE IF NOT EXISTS %1 (%2 TEXT UNIQUE, %3 TEXT, %4 INTEGER, %5 TEXT, %6 TEXT, %7 INTEGER, %8 INTEGER, %9 TEXT)")
                                      .arg(SqliteTransferCollection::TRANSFER_TABLE,//1
                                          Transfer::HistoryMapFields::ID,           //2
                                          Transfer::HistoryMapFields::TIMESTAMP,    //3
                                          Transfer::HistoryMapFields::ACCOUNT_ID,   //4
                                          Transfer::HistoryMapFields::DISPLAY_NAME, //5
                                          Transfer::HistoryMapFields::PEER_NUMBER,  //6
                                          Transfer::HistoryMapFields::OUTGOING,     //7
                                          Transfer::HistoryMapFields::STATE,        //8
                                          Transfer::HistoryMapFields::PATH));       //9

        qDebug() << "Transfer database opened table state : " << createQuery.exec();

    } else {
        qDebug() << "Error opening lrc database";
    }
}

SqliteTransferCollection::~SqliteTransferCollection()
{
    if (db.isOpen()) {
        db.close();
    }
}

void SqliteTransferEditor::saveTransfer(const Transfer* transfer)
{
    if (m_pCollection->db.isOpen()) {

        const Account* a = transfer->account();

        QSqlQuery query(m_pCollection->db);
        query.prepare(QString("INSERT INTO %1 (%2, %3, %4, %5, %6, %7, %8, %9) "
                              "VALUES (:%1, :%2, :%3, :%4, :%5, :%6, :%7, :%8)")
                              .arg(SqliteTransferCollection::TRANSFER_TABLE,//1
                                  Transfer::HistoryMapFields::ID,           //2
                                  Transfer::HistoryMapFields::TIMESTAMP,    //3
                                  Transfer::HistoryMapFields::ACCOUNT_ID,   //4
                                  Transfer::HistoryMapFields::DISPLAY_NAME, //5
                                  Transfer::HistoryMapFields::PEER_NUMBER,  //6
                                  Transfer::HistoryMapFields::OUTGOING,     //7
                                  Transfer::HistoryMapFields::STATE,        //8
                                  Transfer::HistoryMapFields::PATH));       //9

        query.bindValue(0, transfer->id());
        //query.bindValue(1, QString::number(transfer->timestamp()));
        query.bindValue(3, a ? QString(a->id()) : "");
        query.bindValue(4, transfer->account()->id());
        query.bindValue(5, transfer->filename());
        query.bindValue(6, transfer->contactMethod()->uri());
        query.bindValue(7, transfer->isOutgoing());
        query.bindValue(8, transfer->status());
        //query.bindValue(9, transfer->path());
        if (!query.exec()) {
            qWarning() << "Unable to save history";
        }
    } else
        qWarning() << "Unable to save transfer";
}

bool SqliteTransferEditor::removeTransfer(const Transfer* toRemove)
{
    if (m_pCollection->db.isOpen() && toRemove != nullptr) {
        QSqlQuery query(m_pCollection->db);
        query.prepare(QString("DELETE FROM %1 WHERE transferid=(:id))").arg(SqliteTransferCollection::TRANSFER_TABLE));
        query.bindValue(":id", toRemove->id());
        return query.exec();
    }
    return false;
}

bool SqliteTransferEditor::save(const Transfer* transfer)
{
    if (transfer->collection()->editor<Transfer>() != this)
        return addNew(const_cast<Transfer*>(transfer));

    return false;
}

bool SqliteTransferEditor::remove(const Transfer* item)
{
    if (removeTransfer(item)) {
        mediator()->removeItem(item);
        return true;
    }
    return false;
}

bool SqliteTransferEditor::edit( Transfer* item)
{
    Q_UNUSED(item)
    return false;
}

bool SqliteTransferEditor::addNew( Transfer* transfer)
{
    if ((transfer->collection() && transfer->collection()->editor<Transfer>() == this)
        || transfer->id().isEmpty())
        return false;

    saveTransfer(transfer);

    const_cast<Transfer*>(transfer)->setCollection(m_pCollection);
    addExisting(transfer);
    return true;
}

bool SqliteTransferEditor::addExisting(const Transfer* item)
{
    m_lItems << const_cast<Transfer*>(item);
    mediator()->addItem(item);
    return true;
}

QVector<Transfer*> SqliteTransferEditor::items() const
{
    return m_lItems;
}

QString SqliteTransferCollection::name () const
{
    return QObject::tr("Local history");
}

QString SqliteTransferCollection::category () const
{
    return QObject::tr("Transfer");
}

QVariant SqliteTransferCollection::icon() const
{
    return QVariant();
}

bool SqliteTransferCollection::isEnabled() const
{
    return true;
}

bool SqliteTransferCollection::load()
{
    if (db.isOpen()) {
        QMap<QString,QString> hc;
        QSqlQuery query(db);
        query.setForwardOnly(true);
        query.prepare(QString("SELECT * FROM %1").arg(SqliteTransferCollection::TRANSFER_TABLE));
        query.exec();
        /*QMap<QString, int> indexOfFields;
        indexOfFields[Transfer::TransferMapFields::CALLID] = query.record().indexOf(Transfer::TransferMapFields::CALLID);
        indexOfFields[Transfer::TransferMapFields::TIMESTAMP_START] = query.record().indexOf(Transfer::TransferMapFields::TIMESTAMP_START);
        indexOfFields[Transfer::TransferMapFields::TIMESTAMP_STOP] = query.record().indexOf(Transfer::TransferMapFields::TIMESTAMP_STOP);
        indexOfFields[Transfer::TransferMapFields::ACCOUNT_ID] = query.record().indexOf(Transfer::TransferMapFields::ACCOUNT_ID);
        indexOfFields[Transfer::TransferMapFields::DISPLAY_NAME] = query.record().indexOf(Transfer::TransferMapFields::DISPLAY_NAME);
        indexOfFields[Transfer::TransferMapFields::PEER_NUMBER] = query.record().indexOf(Transfer::TransferMapFields::PEER_NUMBER);
        indexOfFields[Transfer::TransferMapFields::DIRECTION] = query.record().indexOf(Transfer::TransferMapFields::DIRECTION);
        indexOfFields[Transfer::TransferMapFields::MISSED] = query.record().indexOf(Transfer::TransferMapFields::MISSED);
        indexOfFields[Transfer::TransferMapFields::CONTACT_USED] = query.record().indexOf(Transfer::TransferMapFields::CONTACT_USED);
        while (query.next()) {
            QMapIterator<QString, int> i(indexOfFields);
            while (i.hasNext()) {
                i.next();
                hc[i.key()] = query.value(i.value()).toString();
            }
            Transfer* pastTransfer = Transfer::buildTransferTransfer(hc);
            pastTransfer->setCollection(this);
            editor<Transfer>()->addExisting(pastTransfer);
            hc.clear();
        }*/
        return true;
    } else
        qWarning() << "Transfer doesn't exist or is not readable";
    return false;
}

bool SqliteTransferCollection::reload()
{
    return false;
}

FlagPack<CollectionInterface::SupportedFeatures> SqliteTransferCollection::supportedFeatures() const
{
    return  CollectionInterface::SupportedFeatures::NONE       |
            CollectionInterface::SupportedFeatures::LOAD       |
            CollectionInterface::SupportedFeatures::CLEAR      |
            CollectionInterface::SupportedFeatures::REMOVE     |
            CollectionInterface::SupportedFeatures::MANAGEABLE |
            CollectionInterface::SupportedFeatures::ADD        ;
}

bool SqliteTransferCollection::clear()
{
    QSqlQuery query("DELETE FROM transfer_history", db);
    return query.exec();
}

QByteArray SqliteTransferCollection::id() const
{
    return "sqtb";
}
