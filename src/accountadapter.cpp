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

AccountAdapter::AccountAdapter(LRCInstance* instance, QObject* parent)
    : QmlAdapterBase(instance, parent)
{}

void
AccountAdapter::safeInit()
{
    connect(lrcInstance_,
            &LRCInstance::currentAccountChanged,
            this,
            &AccountAdapter::onCurrentAccountChanged);

    deselectConversation();

    auto accountId = lrcInstance_->getCurrAccId();
    setProperties(accountId);
    connectAccount(accountId);
}

lrc::api::NewAccountModel*
AccountAdapter::getModel()
{
    return &(lrcInstance_->accountModel());
}

lrc::api::NewDeviceModel*
AccountAdapter::getDeviceModel()
{
    return lrcInstance_->getCurrentAccountInfo().deviceModel.get();
}

void
AccountAdapter::accountChanged(int index)
{
    deselectConversation(); // Hack UI
    auto accountList = lrcInstance_->accountModel().getAccountList();
    if (accountList.size() > index) {
        lrcInstance_->setSelectedAccountId(accountList.at(index));
    }
}

void
AccountAdapter::connectFailure()
{
    Utils::oneShotConnect(&lrcInstance_->accountModel(),
                          &lrc::api::NewAccountModel::accountRemoved,
                          [this](const QString& accountId) {
                              Q_UNUSED(accountId);
                              emit reportFailure();
                          });
    Utils::oneShotConnect(&lrcInstance_->accountModel(),
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
        &lrcInstance_->accountModel(),
        &lrc::api::NewAccountModel::accountAdded,
        [this, registeredName, settings, isCreating](const QString& accountId) {
            auto confProps = lrcInstance_->accountModel().getAccountConfig(accountId);
            confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
            confProps.isRendezVous = settings["isRendezVous"].toBool();
            lrcInstance_->accountModel().setAccountConfig(accountId, confProps);

            auto showBackup = isCreating
                              && !AppSettingsManager::getValue(Settings::Key::NeverShowMeAgain)
                                      .toBool();
            if (!registeredName.isEmpty()) {
                QObject::disconnect(registeredNameSavedConnection_);
                registeredNameSavedConnection_ = connect(
                    &lrcInstance_->accountModel(),
                    &lrc::api::NewAccountModel::profileUpdated,
                    [this, showBackup, addedAccountId = accountId](const QString& accountId) {
                        if (addedAccountId == accountId) {
                            emit lrcInstance_->accountListChanged();
                            emit accountAdded(accountId,
                                              showBackup,
                                              lrcInstance_->accountModel().getAccountList().indexOf(
                                                  accountId));
                            QObject::disconnect(registeredNameSavedConnection_);
                        }
                    });

                lrcInstance_->accountModel().registerName(accountId,
                                                          settings["password"].toString(),
                                                          registeredName);
            } else {
                emit lrcInstance_->accountListChanged();
                emit accountAdded(accountId,
                                  showBackup,
                                  lrcInstance_->accountModel().getAccountList().indexOf(accountId));
            }
        });

    connectFailure();

    QtConcurrent::run([this, settings] {
        lrcInstance_->accountModel().createNewAccount(lrc::api::profile::Type::RING,
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
    Utils::oneShotConnect(&lrcInstance_->accountModel(),
                          &lrc::api::NewAccountModel::accountAdded,
                          [this, settings](const QString& accountId) {
                              auto confProps = lrcInstance_->accountModel().getAccountConfig(
                                  accountId);
                              // set SIP details
                              confProps.hostname = settings["hostname"].toString();
                              confProps.username = settings["username"].toString();
                              confProps.password = settings["password"].toString();
                              confProps.routeset = settings["proxy"].toString();
                              confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
                              lrcInstance_->accountModel().setAccountConfig(accountId, confProps);

                              emit lrcInstance_->accountListChanged();
                              emit accountAdded(accountId,
                                                false,
                                                lrcInstance_->accountModel().getAccountList().indexOf(
                                                    accountId));
                          });

    connectFailure();

    QtConcurrent::run([this, settings] {
        lrcInstance_->accountModel().createNewAccount(lrc::api::profile::Type::SIP,
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
    Utils::oneShotConnect(&lrcInstance_->accountModel(),
                          &lrc::api::NewAccountModel::accountAdded,
                          [this](const QString& accountId) {
                              if (!lrcInstance_->accountModel().getAccountList().size())
                                  return;

                              auto confProps = lrcInstance_->accountModel().getAccountConfig(
                                  accountId);
                              confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
                              lrcInstance_->accountModel().setAccountConfig(accountId, confProps);

                              emit accountAdded(accountId,
                                                false,
                                                lrcInstance_->accountModel().getAccountList().indexOf(
                                                    accountId));
                              emit lrcInstance_->accountListChanged();
                          });

    connectFailure();

    QtConcurrent::run([this, settings] {
        lrcInstance_->accountModel().connectToAccountManager(settings["username"].toString(),
                                                             settings["password"].toString(),
                                                             settings["manager"].toString());
    });
}

void
AccountAdapter::deleteCurrentAccount()
{
    lrcInstance_->accountModel().removeAccount(lrcInstance_->getCurrAccId());
    emit lrcInstance_->accountListChanged();
}

bool
AccountAdapter::savePassword(const QString& accountId,
                             const QString& oldPassword,
                             const QString& newPassword)
{
    return lrcInstance_->accountModel().changeAccountPassword(accountId, oldPassword, newPassword);
}

void
AccountAdapter::startPreviewing(bool force)
{
    lrcInstance_->renderer()->startPreviewing(force);
}

void
AccountAdapter::stopPreviewing()
{
    if (!lrcInstance_->hasVideoCall() && lrcInstance_->renderer()->isPreviewing()) {
        lrcInstance_->renderer()->stopPreviewing();
    }
}

bool
AccountAdapter::hasVideoCall()
{
    return lrcInstance_->hasVideoCall();
}

bool
AccountAdapter::isPreviewing()
{
    return lrcInstance_->renderer()->isPreviewing();
}

void
AccountAdapter::setCurrAccDisplayName(const QString& text)
{
    lrcInstance_->setCurrAccDisplayName(text);
}

void
AccountAdapter::setSelectedConvId(const QString& convId)
{
    lrcInstance_->setSelectedConvId(convId);
}

lrc::api::profile::Type
AccountAdapter::getCurrentAccountType()
{
    return lrcInstance_->getCurrentAccountInfo().profileInfo.type;
}

void
AccountAdapter::setCurrAccAvatar(bool fromFile, const QString& source)
{
    QtConcurrent::run([this, fromFile, source]() {
        QPixmap image;
        bool success;
        if (fromFile)
            success = image.load(source);
        else
            success = image.loadFromData(Utils::base64StringToByteArray(source));

        if (success)
            lrcInstance_->setCurrAccAvatar(image);
    });
}

void
AccountAdapter::onCurrentAccountChanged()
{
    auto accountId = lrcInstance_->getCurrAccId();
    setProperties(accountId);
    connectAccount(accountId);
}

bool
AccountAdapter::hasPassword()
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(lrcInstance_->getCurrAccId());
    return confProps.archiveHasPassword;
}

void
AccountAdapter::setArchiveHasPassword(bool isHavePassword)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(lrcInstance_->getCurrAccId());
    confProps.archiveHasPassword = isHavePassword;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrAccId(), confProps);
}
bool
AccountAdapter::exportToFile(const QString& accountId,
                             const QString& path,
                             const QString& password) const
{
    return lrcInstance_->accountModel().exportToFile(accountId, path, password);
}

void
AccountAdapter::setArchivePasswordAsync(const QString& accountID, const QString& password)
{
    QtConcurrent::run([this, accountID, password] {
        auto config = lrcInstance_->accountModel().getAccountConfig(accountID);
        config.archivePassword = password;
        lrcInstance_->accountModel().setAccountConfig(accountID, config);
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
    if (lrcInstance_->getCurrentConvUid().isEmpty()) {
        return;
    }

    auto currentConversationModel = lrcInstance_->getCurrentConversationModel();

    if (currentConversationModel == nullptr) {
        return;
    }

    lrcInstance_->setSelectedConvId();
}

void
AccountAdapter::connectAccount(const QString& accountId)
{
    try {
        auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId);

        QObject::disconnect(accountStatusChangedConnection_);
        QObject::disconnect(accountProfileUpdatedConnection_);
        QObject::disconnect(contactAddedConnection_);
        QObject::disconnect(addedToConferenceConnection_);
        QObject::disconnect(contactUnbannedConnection_);

        accountStatusChangedConnection_
            = QObject::connect(accInfo.accountModel,
                               &lrc::api::NewAccountModel::accountStatusChanged,
                               [this](const QString& accountId) {
                                   emit accountStatusChanged(accountId);
                               });

        accountProfileUpdatedConnection_
            = QObject::connect(accInfo.accountModel,
                               &lrc::api::NewAccountModel::profileUpdated,
                               [this](const QString& accountId) {
                                   emit accountStatusChanged(accountId);
                               });

        contactAddedConnection_
            = QObject::connect(accInfo.contactModel.get(),
                               &lrc::api::ContactModel::contactAdded,
                               [this, accountId](const QString& contactUri) {
                                   const auto& convInfo = lrcInstance_->getConversationFromConvUid(
                                       lrcInstance_->getCurrentConvUid());
                                   if (convInfo.uid.isEmpty()) {
                                       return;
                                   }
                                   auto& accInfo = lrcInstance_->accountModel().getAccountInfo(
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
                               [this](const QString& callId, const QString& confId) {
                                   Q_UNUSED(callId);
                                   lrcInstance_->renderer()->addDistantRenderer(confId);
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
    auto accountType = lrcInstance_->getAccountInfo(accountId).profileInfo.type;
    setProperty("currentAccountType", lrc::api::profile::to_string(accountType));
    emit deviceModelChanged();
}
