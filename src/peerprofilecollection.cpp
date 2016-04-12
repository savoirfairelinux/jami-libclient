/***************************************************************************
 * Copyright (C) 2016 by Savoir-faire Linux                                *
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>*
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 **************************************************************************/

#include "peerprofilecollection.h"

//Qt
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtCore/QVector>

//Ring
#include "private/vcardutils.h"
#include "account.h"
#include "accountmodel.h"
#include "person.h"

class PeerProfileEditor final : public CollectionEditor<Person>
{
public:
   PeerProfileEditor(CollectionMediator<Person>* m);
   virtual bool save       (const Person* pers ) override;
   virtual bool remove     ( const Person* item ) override;
   virtual bool edit       ( Person*       item ) override;
   virtual bool addNew     (Person*       pers ) override;
   virtual bool addExisting( const Person* item ) override;

private:
    virtual QVector<Person*> items() const override;

    //Helpers
    QString path(const Person* p) const;

    //Attributes
    QVector<Person*> m_lItems;
};

PeerProfileEditor::PeerProfileEditor(CollectionMediator<Person>* m) : CollectionEditor<Person>(m)
{

}

PeerProfileCollection::PeerProfileCollection(CollectionMediator<Person>* mediator) :
CollectionInterface(new PeerProfileEditor(mediator))
{

}

PeerProfileCollection::~PeerProfileCollection()
{

}

bool PeerProfileEditor::save(const Person* pers)
{
    const auto& filename = path(pers);
    const auto& result = pers->toVCard();

    QFile file {filename};
    file.open(QIODevice::WriteOnly);
    file.write(result);
    file.close();
    return true;
}

bool PeerProfileEditor::remove(const Person* item)
{
    if (QFile::remove(path(item))) {
        mediator()->removeItem(item);
        return true;
    }

    return false;
}

bool PeerProfileEditor::edit( Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool PeerProfileEditor::addNew( Person* pers)
{
    if (not m_lItems.contains(pers)) {
        m_lItems << pers;
        mediator()->addItem(pers);
    }
    save(pers);
    return true;
}

bool PeerProfileEditor::addExisting(const Person* item)
{
   m_lItems << const_cast<Person*>(item);
   mediator()->addItem(item);
   return true;
}

QVector<Person*> PeerProfileEditor::items() const
{
   return m_lItems;
}

QString PeerProfileEditor::path(const Person* p) const
{
   const QDir profilesDir = (QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/peer_profiles/";
   profilesDir.mkpath(profilesDir.path());
   return QString("%1/%2.vcf")
      .arg(profilesDir.absolutePath())
      .arg(QString(p->uid()));
}

QString PeerProfileCollection::name () const
{
   return QObject::tr("Peer profiles");
}

QString PeerProfileCollection::category () const
{
   return QObject::tr("Peers Profiles Collection");
}

QVariant PeerProfileCollection::icon() const
{
   return QVariant();
}

bool PeerProfileCollection::isEnabled() const
{
   return true;
}

bool PeerProfileCollection::load()
{
    const QDir profilesDir = (QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/peer_profiles/";

    const QStringList entries = profilesDir.entryList({QStringLiteral("*.vcf")}, QDir::Files);

    foreach (const QString& item , entries) {
        auto personProfile = new Person(nullptr);
        QList<Account*> accs;
        VCardUtils::mapToPerson(personProfile,QUrl(profilesDir.path()+'/'+item),&accs);
        editor<Person>()->addExisting(personProfile);
    }

    return true;
}

bool PeerProfileCollection::reload()
{
    return true;
}

FlagPack<CollectionInterface::SupportedFeatures> PeerProfileCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE       |
      CollectionInterface::SupportedFeatures::LOAD       |
      CollectionInterface::SupportedFeatures::CLEAR      |
      CollectionInterface::SupportedFeatures::REMOVE     |
      CollectionInterface::SupportedFeatures::MANAGEABLE |
      CollectionInterface::SupportedFeatures::ADD        ;
}

bool PeerProfileCollection::clear()
{
   QFile::remove((QStandardPaths::writableLocation(QStandardPaths::DataLocation)) + "/peer_profiles/");
   return true;
}

QByteArray PeerProfileCollection::id() const
{
   return "ppc";
}
