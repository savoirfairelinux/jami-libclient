/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                       *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#include "api/collections/searchResultList.h"

#include "api/element.h"

#include "dbus/configurationmanager.h"

namespace lrc
{

namespace api
{

class SearchResultListPimpl : public QObject
{
    Q_OBJECT
public:
    SearchResultListPimpl(const QString& owner);
    ~SearchResultListPimpl();

public Q_SLOTS:
    void slotSearchUserEnded(const QString& accountID, int state, const QString& query, const VectorMapStringString& results);

public:
    std::mutex elementsMutex_;
    QMap<QString, Element> elements;
    QString accountId_;
};

SearchResultListPimpl::SearchResultListPimpl(const QString& owner) : accountId_(owner)
{
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::userSearchEnded,
            this,
            &SearchResultListPimpl::slotSearchUserEnded,
            Qt::QueuedConnection);
}

SearchResultListPimpl::~SearchResultListPimpl()
{
    disconnect(&ConfigurationManager::instance(),
               &ConfigurationManagerInterface::userSearchEnded,
               this,
               &SearchResultListPimpl::slotSearchUserEnded);
}

void
SearchResultListPimpl::slotSearchUserEnded(const QString& accountID, int state, const QString& query, const VectorMapStringString& results)
{
    qWarning() << "@@@ slotSearchUserEnded: " << query << " " << state;
    for (const auto& map: results.toStdVector()) {
        qWarning() << "";
        for (const auto& [key, v]: map.toStdMap()) {
            qWarning() << "@@@ result: " << key << " " << v;
        }
    }
}

SearchResultList::SearchResultList(const QString &owner)
: Collection(owner)
, pimpl_ {std::make_unique<SearchResultListPimpl>(owner)}
{

}

SearchResultList::~SearchResultList()
{

}

QVector<Element>
SearchResultList::filter(const QString& search)
{
    ConfigurationManager::instance().searchUser(pimpl_->accountId_, search);
    return {};
}

} // namespace api
} // namespace lrc

#include "api/collections/moc_searchResultList.cpp"
#include "searchResultList.moc"
