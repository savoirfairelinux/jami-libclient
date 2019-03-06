/****************************************************************************
 *    Copyright (C) 2016-2019 Savoir-faire Linux Inc.                               *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#include "localprofilecollection.h"

//Qt
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtCore/QVector>
#include <QtCore/QDateTime>

//Ring
#include "authority/databasehelper.h"
#include "profile.h"
#include "private/vcardutils.h"
#include "account.h"
#include "accountmodel.h"
#include "person.h"
#include "profilemodel.h"

class LocalProfileEditor final : public CollectionEditor<Profile>
{
public:
   LocalProfileEditor(CollectionMediator<Profile>* m, LocalProfileCollection* parent);
   virtual bool save       ( const Profile* item ) override;
   virtual bool remove     ( const Profile* item ) override;
   virtual bool edit       ( Profile*       item ) override;
   virtual bool addNew     ( Profile*       item ) override;
   virtual bool addExisting( const Profile* item ) override;

private:
    virtual QVector<Profile*> items() const override;

    //Helpers
    QString path(const Profile* p) const;

    //Attributes
    QVector<Profile*> m_lItems;
    LocalProfileCollection* m_pCollection;
};

LocalProfileEditor::LocalProfileEditor(CollectionMediator<Profile>* m, LocalProfileCollection* parent) :
CollectionEditor<Profile>(m),m_pCollection(parent)
{

}

LocalProfileCollection::LocalProfileCollection(CollectionMediator<Profile>* mediator) :
CollectionInterface(new LocalProfileEditor(mediator,this))
{

}

LocalProfileCollection::~LocalProfileCollection()
{

}

bool LocalProfileEditor::save(const Profile* pro)
{
    // we need to bind the legacy lrc to the new one. We doing that by using profileUpdated
    emit ProfileModel::instance().profileUpdated(pro);

    return true;
}

bool LocalProfileEditor::remove(const Profile* item)
{
    return false;
}

bool LocalProfileEditor::edit( Profile* item)
{
   Q_UNUSED(item)
   return false;
}

bool LocalProfileEditor::addNew(Profile* pro)
{
   return true;
}

bool LocalProfileEditor::addExisting(const Profile* item)
{
    return true;
}

QVector<Profile*> LocalProfileEditor::items() const
{
   return m_lItems;
}

QString LocalProfileEditor::path(const Profile* p) const
{
    return QString();
}

QString LocalProfileCollection::name () const
{
   return QObject::tr("Local profiles");
}

QString LocalProfileCollection::category () const
{
   return QObject::tr("Profile Collection");
}

QVariant LocalProfileCollection::icon() const
{
   return QVariant();
}

bool LocalProfileCollection::isEnabled() const
{
   return true;
}

bool LocalProfileCollection::load()
{
    return true;
}

bool LocalProfileCollection::reload()
{
    return true;
}

FlagPack<CollectionInterface::SupportedFeatures> LocalProfileCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE       |
      CollectionInterface::SupportedFeatures::LOAD       |
      CollectionInterface::SupportedFeatures::CLEAR      |
      CollectionInterface::SupportedFeatures::REMOVE     |
      CollectionInterface::SupportedFeatures::MANAGEABLE |
      CollectionInterface::SupportedFeatures::ADD        ;
}

bool LocalProfileCollection::clear()
{
   QFile::remove((lrc::authority::storage::getPath()) + "/profiles/");
   return true;
}

QByteArray LocalProfileCollection::id() const
{
   return "lpc";
}

void LocalProfileCollection::setupDefaultProfile()
{
}
