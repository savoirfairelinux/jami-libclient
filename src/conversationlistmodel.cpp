/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "conversationlistmodel.h"

#include "uri.h"

ConversationListModel::ConversationListModel(LRCInstance* instance, QObject* parent)
    : ConversationListModelBase(instance, parent)
{
    if (!model_)
        return;

    connect(
        model_,
        &ConversationModel::beginInsertRows,
        this,
        [this](int position, int rows) {
            beginInsertRows(QModelIndex(), position, position + (rows - 1));
        },
        Qt::DirectConnection);
    connect(model_,
            &ConversationModel::endInsertRows,
            this,
            &ConversationListModel::endInsertRows,
            Qt::DirectConnection);

    connect(
        model_,
        &ConversationModel::beginRemoveRows,
        this,
        [this](int position, int rows) {
            beginRemoveRows(QModelIndex(), position, position + (rows - 1));
        },
        Qt::DirectConnection);
    connect(model_,
            &ConversationModel::endRemoveRows,
            this,
            &ConversationListModel::endRemoveRows,
            Qt::DirectConnection);

    connect(model_, &ConversationModel::dataChanged, this, [this](int position) {
        const auto index = createIndex(position, 0);
        Q_EMIT ConversationListModel::dataChanged(index, index);
    });
}

int
ConversationListModel::rowCount(const QModelIndex& parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (!parent.isValid() && model_) {
        return model_->getConversations().size();
    }
    return 0;
}

QVariant
ConversationListModel::data(const QModelIndex& index, int role) const
{
    const auto& data = model_->getConversations();
    if (!index.isValid() || data.empty())
        return {};
    return dataForItem(data.at(index.row()), role);
}

ConversationListProxyModel::ConversationListProxyModel(QAbstractListModel* model, QObject* parent)
    : SelectableListProxyModel(model, parent)
{
    setSortRole(ConversationList::Role::LastInteractionTimeStamp);
    sort(0, Qt::DescendingOrder);
    setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
}

bool
ConversationListProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    auto rx = filterRegExp();
    auto uriStripper = URI(rx.pattern());
    bool stripScheme = (uriStripper.schemeType() < URI::SchemeType::COUNT__);
    FlagPack<URI::Section> flags = URI::Section::USER_INFO | URI::Section::HOSTNAME
                                   | URI::Section::PORT;
    if (!stripScheme) {
        flags |= URI::Section::SCHEME;
    }
    rx.setPattern(uriStripper.format(flags));
    auto uri = index.data(ConversationList::Role::URI).toString();
    auto alias = index.data(ConversationList::Role::Alias).toString();
    auto registeredName = index.data(ConversationList::Role::RegisteredName).toString();
    auto itemProfileType = index.data(ConversationList::Role::ContactType).toInt();
    auto typeFilter = static_cast<profile::Type>(itemProfileType) == currentTypeFilter_;
    if (index.data(ConversationList::Role::IsBanned).toBool()) {
        return typeFilter
               && (rx.exactMatch(uri) || rx.exactMatch(alias) || rx.exactMatch(registeredName));
    }
    return typeFilter
           && (rx.indexIn(uri) != -1 || rx.indexIn(alias) != -1 || rx.indexIn(registeredName) != -1);
}

bool
ConversationListProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QVariant leftData = sourceModel()->data(left, sortRole());
    QVariant rightData = sourceModel()->data(right, sortRole());
    // we're assuming the sort role data type here is some integral time
    return leftData.toULongLong() < rightData.toULongLong();
}

void
ConversationListProxyModel::setTypeFilter(const profile::Type& typeFilter)
{
    beginResetModel();
    currentTypeFilter_ = typeFilter;
    endResetModel();
    updateSelection();
};
