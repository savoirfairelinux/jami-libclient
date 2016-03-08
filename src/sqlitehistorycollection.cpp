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
{}

SqliteHistoryCollection::SqliteHistoryCollection(CollectionMediator<Call>* mediator) :
    CollectionInterface(new SqliteHistoryEditor(mediator,this)),m_pMediator(mediator)
{
    if (SqlManager::instance().isOpen()) {

        SqlManager::instance().createTable<Call>();
        auto filePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "history.ini";
        if (QFile::exists(filePath))
            legacyImport(filePath);
    } else {
        qDebug() << "Error opening history database";
    }
}

void SqliteHistoryCollection::legacyImport(const QString& filePath)
{
    QFile file(filePath);
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QMap<QString,QVariant> hc;
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
{}

void SqliteHistoryEditor::saveCall(const Call* call)
{
    if (not SqlManager::instance().saveItem<Call>(*call))
        qWarning() << "Unable to save history call";
}

bool SqliteHistoryEditor::removeCall(const Call* toRemove)
{
    return SqlManager::instance().deleteItem<Call>(toRemove);
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
    if ((call->collection() && call->collection()->editor<Call>() == this)
            || call->historyId().isEmpty()) return false;

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
        auto objList = SqlManager::instance().loadItems<Call>();
        foreach (auto obj, objList) {
            Call* pastCall = Call::buildHistoryCall(obj);
            pastCall->setCollection(this);
            editor<Call>()->addExisting(pastCall);
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
    return SqlManager::instance().deleteAll<Call>();
}

QByteArray SqliteHistoryCollection::id() const
{
    return "sqhb";
}
