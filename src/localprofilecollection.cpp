/****************************************************************************
 *   Copyright (C) 2016-2017 Savoir-faire Linux                               *
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
#include "profile.h"
#include "private/vcardutils.h"
#include "account.h"
#include "accountmodel.h"
#include "person.h"

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
    const auto& filename = path(pro);
    const auto& result = pro->person()->toVCard(pro->accounts().toList());

    qDebug() << "Saving profile in:" << filename;
    QFile file {filename};

    if (Q_UNLIKELY(!file.open(QIODevice::WriteOnly))) {
        qWarning() << "Can't write to" << filename;
        return false;
    }

    file.write(result);
    file.close();
    return true;
}

bool LocalProfileEditor::remove(const Profile* item)
{
    if (QFile::remove(path(item))) {
        mediator()->removeItem(item);
        return true;
    }

    return false;
}

bool LocalProfileEditor::edit( Profile* item)
{
   Q_UNUSED(item)
   return false;
}

bool LocalProfileEditor::addNew(Profile* pro)
{
    pro->person()->ensureUid();
    qDebug() << "Creating new profile" << pro->person()->uid();
    m_lItems << pro;
    pro->setCollection(m_pCollection);
    mediator()->addItem(pro);
    save(pro);
    return true;
}

bool LocalProfileEditor::addExisting(const Profile* item)
{
   m_lItems << const_cast<Profile*>(item);
   mediator()->addItem(item);
   return true;
}

QVector<Profile*> LocalProfileEditor::items() const
{
   return m_lItems;
}

QString LocalProfileEditor::path(const Profile* p) const
{
   const QDir profilesDir = (QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/profiles/";
   profilesDir.mkpath(profilesDir.path());
   return QString("%1/%2.vcf")
      .arg(profilesDir.absolutePath())
      .arg(QString(p->person()->uid()));
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
    const QDir profilesDir = (QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/profiles/";
    qDebug() << "Loading vcf from:" << profilesDir;

    const QStringList entries = profilesDir.entryList({QStringLiteral("*.vcf")}, QDir::Files);
    bool hasProfile = false;

    foreach (const QString& item , entries) {
        hasProfile = true;
        auto personProfile = new Person(nullptr);
        QList<Account*> accs;
        VCardUtils::mapToPerson(personProfile,QUrl(profilesDir.path()+'/'+item),&accs);
        auto profile = new Profile(this, personProfile);
        profile->setAccounts(accs.toVector());
        editor<Profile>()->addExisting(profile);
    }

    if (!hasProfile) {
        //Ring needs at least one global profile
        setupDefaultProfile();
    }
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
   QFile::remove((QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/profiles/");
   return true;
}

QByteArray LocalProfileCollection::id() const
{
   return "lpc";
}

void LocalProfileCollection::setupDefaultProfile()
{
   auto profile = new Profile(this, new Person());
   profile->person()->setFormattedName(QObject::tr("Default"));

   for (int i = 0 ; i < AccountModel::instance().size() ; i++) {
       profile->addAccount(AccountModel::instance()[i]);
   }

   editor<Profile>()->addNew(profile);
}
