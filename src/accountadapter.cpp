/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Anthony L�onard <anthony.leonard@savoirfairelinux.com
 * Author: Olivier Soldano <olivier.soldano@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

#include "accountadapter.h"

#include "qtutils.h"

#include <QtConcurrent/QtConcurrent>

AccountAdapter::AccountAdapter(QObject* parent)
    : QmlAdapterBase(parent)
{}

void
AccountAdapter::safeInit()
{
    connect(&LRCInstance::instance(),
            &LRCInstance::currentAccountChanged,
            this,
            &AccountAdapter::onCurrentAccountChanged);

    deselectConversation();

    auto accountId = LRCInstance::getCurrAccId();
    setProperties(accountId);
    connectAccount(accountId);
}

lrc::api::NewAccountModel*
AccountAdapter::getModel()
{
    return &(LRCInstance::accountModel());
}

lrc::api::NewDeviceModel*
AccountAdapter::getDeviceModel()
{
    return LRCInstance::getCurrentAccountInfo().deviceModel.get();
}

void
AccountAdapter::accountChanged(int index)
{
    deselectConversation(); // Hack UI
    auto accountList = LRCInstance::accountModel().getAccountList();
    if (accountList.size() > index) {
        LRCInstance::setSelectedAccountId(accountList.at(index));
    }
}

void
AccountAdapter::connectFailure()
{
    Utils::oneShotConnect(&LRCInstance::accountModel(),
                          &lrc::api::NewAccountModel::accountRemoved,
                          [this](const QString& accountId) {
                              Q_UNUSED(accountId);
                              emit reportFailure();
                          });
    Utils::oneShotConnect(&LRCInstance::accountModel(),
                          &lrc::api::NewAccountModel::invalidAccountDetected,
                          [this](const QString& accountId) {
                              Q_UNUSED(accountId);
                              emit reportFailure();
                          });
}

void
AccountAdapter::createJamiAccount(QString registeredName,
                                  const QVariantMap& settings,
                                  bool isCreating)
{
    Utils::oneShotConnect(
        &LRCInstance::accountModel(),
        &lrc::api::NewAccountModel::accountAdded,
        [this, registeredName, settings, isCreating](const QString& accountId) {
            auto confProps = LRCInstance::accountModel().getAccountConfig(accountId);
            confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
            confProps.isRendezVous = settings["isRendezVous"].toBool();
            LRCInstance::accountModel().setAccountConfig(accountId, confProps);

            auto showBackup = isCreating
                              && !AppSettingsManager::getValue(Settings::Key::NeverShowMeAgain)
                                      .toBool();
            if (!registeredName.isEmpty()) {
                QObject::disconnect(registeredNameSavedConnection_);
                registeredNameSavedConnection_ = connect(
                    &LRCInstance::accountModel(),
                    &lrc::api::NewAccountModel::profileUpdated,
                    [this, showBackup, addedAccountId = accountId](const QString& accountId) {
                        if (addedAccountId == accountId) {
                            emit LRCInstance::instance().accountListChanged();
                            emit accountAdded(accountId,
                                              showBackup,
                                              LRCInstance::accountModel().getAccountList().indexOf(
                                                  accountId));
                            QObject::disconnect(registeredNameSavedConnection_);
                        }
                    });

                LRCInstance::accountModel().registerName(accountId,
                                                         settings["password"].toString(),
                                                         registeredName);
            } else {
                emit LRCInstance::instance().accountListChanged();
                emit accountAdded(accountId,
                                  showBackup,
                                  LRCInstance::accountModel().getAccountList().indexOf(accountId));
            }
        });

    connectFailure();

    QtConcurrent::run([settings] {
        LRCInstance::accountModel().createNewAccount(lrc::api::profile::Type::RING,
                                                     settings["alias"].toString(),
                                                     settings["archivePath"].toString(),
                                                     settings["password"].toString(),
                                                     settings["archivePin"].toString(),
                                                     "");
    });
}

void
AccountAdapter::createSIPAccount(const QVariantMap& settings)
{
    Utils::oneShotConnect(&LRCInstance::accountModel(),
                          &lrc::api::NewAccountModel::accountAdded,
                          [this, settings](const QString& accountId) {
                              auto confProps = LRCInstance::accountModel().getAccountConfig(
                                  accountId);
                              // set SIP details
                              confProps.hostname = settings["hostname"].toString();
                              confProps.username = settings["username"].toString();
                              confProps.password = settings["password"].toString();
                              confProps.routeset = settings["proxy"].toString();
                              confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
                              LRCInstance::accountModel().setAccountConfig(accountId, confProps);

                              emit LRCInstance::instance().accountListChanged();
                              emit accountAdded(accountId,
                                                false,
                                                LRCInstance::accountModel().getAccountList().indexOf(
                                                    accountId));
                          });

    connectFailure();

    QtConcurrent::run([settings] {
        LRCInstance::accountModel().createNewAccount(lrc::api::profile::Type::SIP,
                                                     settings["alias"].toString(),
                                                     settings["archivePath"].toString(),
                                                     "",
                                                     "",
                                                     settings["username"].toString(),
                                                     {});
    });
}

void
AccountAdapter::createJAMSAccount(const QVariantMap& settings)
{
    Utils::oneShotConnect(&LRCInstance::accountModel(),
                          &lrc::api::NewAccountModel::accountAdded,
                          [this](const QString& accountId) {
                              if (!LRCInstance::accountModel().getAccountList().size())
                                  return;

                              auto confProps = LRCInstance::accountModel().getAccountConfig(
                                  accountId);
                              confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
                              LRCInstance::accountModel().setAccountConfig(accountId, confProps);

                              emit accountAdded(accountId,
                                                false,
                                                LRCInstance::accountModel().getAccountList().indexOf(
                                                    accountId));
                              emit LRCInstance::instance().accountListChanged();
                          });

    connectFailure();

    QtConcurrent::run([settings] {
        LRCInstance::accountModel().connectToAccountManager(settings["username"].toString(),
                                                            settings["password"].toString(),
                                                            settings["manager"].toString());
    });
}

void
AccountAdapter::deleteCurrentAccount()
{
    LRCInstance::accountModel().removeAccount(LRCInstance::getCurrAccId());
    emit LRCInstance::instance().accountListChanged();
}

bool
AccountAdapter::savePassword(const QString& accountId,
                             const QString& oldPassword,
                             const QString& newPassword)
{
    return LRCInstance::accountModel().changeAccountPassword(accountId, oldPassword, newPassword);
}

void
AccountAdapter::startPreviewing(bool force)
{
    LRCInstance::renderer()->startPreviewing(force);
}

void
AccountAdapter::stopPreviewing()
{
    if (!LRCInstance::hasVideoCall() && LRCInstance::renderer()->isPreviewing()) {
        LRCInstance::renderer()->stopPreviewing();
    }
}

bool
AccountAdapter::hasVideoCall()
{
    return LRCInstance::hasVideoCall();
}

bool
AccountAdapter::isPreviewing()
{
    return LRCInstance::renderer()->isPreviewing();
}

void
AccountAdapter::setCurrAccDisplayName(const QString& text)
{
    LRCInstance::setCurrAccDisplayName(text);
}

void
AccountAdapter::setSelectedConvId(const QString& convId)
{
    LRCInstance::setSelectedConvId(convId);
}

lrc::api::profile::Type
AccountAdapter::getCurrentAccountType()
{
    return LRCInstance::getCurrentAccountInfo().profileInfo.type;
}

void
AccountAdapter::setCurrAccAvatar(bool fromFile, const QString& source)
{
    QtConcurrent::run([fromFile, source]() {
        QPixmap image;
        bool success;
        if (fromFile)
            success = image.load(source);
        else
            success = image.loadFromData(Utils::base64StringToByteArray(source));

        if (success)
            LRCInstance::setCurrAccAvatar(image);
    });
}

void
AccountAdapter::onCurrentAccountChanged()
{
    auto accountId = LRCInstance::getCurrAccId();
    setProperties(accountId);
    connectAccount(accountId);
}

bool
AccountAdapter::hasPassword()
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    return confProps.archiveHasPassword;
}

void
AccountAdapter::setArchiveHasPassword(bool isHavePassword)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.archiveHasPassword = isHavePassword;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}
bool
AccountAdapter::exportToFile(const QString& accountId,
                             const QString& path,
                             const QString& password) const
{
    return LRCInstance::accountModel().exportToFile(accountId, path, password);
}

void
AccountAdapter::setArchivePasswordAsync(const QString& accountID, const QString& password)
{
    QtConcurrent::run([accountID, password] {
        auto config = LRCInstance::accountModel().getAccountConfig(accountID);
        config.archivePassword = password;
        LRCInstance::accountModel().setAccountConfig(accountID, config);
    });
}

void
AccountAdapter::passwordSetStatusMessageBox(bool success, QString title, QString infoToDisplay)
{
    if (success) {
        QMessageBox::information(0, title, infoToDisplay);
    } else {
        QMessageBox::critical(0, title, infoToDisplay);
    }
}

void
AccountAdapter::deselectConversation()
{
    if (LRCInstance::getCurrentConvUid().isEmpty()) {
        return;
    }

    auto currentConversationModel = LRCInstance::getCurrentConversationModel();

    if (currentConversationModel == nullptr) {
        return;
    }

    LRCInstance::setSelectedConvId();
}

void
AccountAdapter::connectAccount(const QString& accountId)
{
    try {
        auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId);

        QObject::disconnect(accountStatusChangedConnection_);
        QObject::disconnect(contactAddedConnection_);
        QObject::disconnect(addedToConferenceConnection_);
        QObject::disconnect(contactUnbannedConnection_);

        accountStatusChangedConnection_
            = QObject::connect(accInfo.accountModel,
                               &lrc::api::NewAccountModel::accountStatusChanged,
                               [this](const QString& accountId) {
                                   emit accountStatusChanged(accountId);
                               });

        contactAddedConnection_
            = QObject::connect(accInfo.contactModel.get(),
                               &lrc::api::ContactModel::contactAdded,
                               [this, accountId](const QString& contactUri) {
                                   const auto& convInfo = LRCInstance::getConversationFromConvUid(
                                       LRCInstance::getCurrentConvUid());
                                   if (convInfo.uid.isEmpty()) {
                                       return;
                                   }
                                   auto& accInfo = LRCInstance::accountModel().getAccountInfo(
                                       accountId);
                                   if (contactUri
                                       == accInfo.contactModel
                                              ->getContact(convInfo.participants.at(0))
                                              .profileInfo.uri) {
                                       /*
                                        * Update conversation.
                                        */
                                       emit updateConversationForAddedContact();
                                   }
                               });

        addedToConferenceConnection_
            = QObject::connect(accInfo.callModel.get(),
                               &NewCallModel::callAddedToConference,
                               [](const QString& callId, const QString& confId) {
                                   Q_UNUSED(callId);
                                   LRCInstance::renderer()->addDistantRenderer(confId);
                               });

        contactUnbannedConnection_ = QObject::connect(accInfo.contactModel.get(),
                                                      &lrc::api::ContactModel::bannedStatusChanged,
                                                      [this](const QString& contactUri,
                                                             bool banned) {
                                                          if (!banned)
                                                              emit contactUnbanned();
                                                      });
    } catch (...) {
        qWarning() << "Couldn't get account: " << accountId;
    }
}

void
AccountAdapter::setProperties(const QString& accountId)
{
    setProperty("currentAccountId", accountId);
    auto accountType = LRCInstance::getAccountInfo(accountId).profileInfo.type;
    setProperty("currentAccountType", lrc::api::profile::to_string(accountType));
    emit deviceModelChanged();
}
