/*
 * Copyright (C) 2020-2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Yang Wang <yang.yang@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

#include "appsettingsmanager.h"
#include "qtutils.h"
#include "qmlregister.h"

#include <QtConcurrent/QtConcurrent>

AccountAdapter::AccountAdapter(AppSettingsManager* settingsManager,
                               LRCInstance* instance,
                               QObject* parent)
    : QmlAdapterBase(instance, parent)
    , settingsManager_(settingsManager)
    , accSrcModel_(new AccountListModel(instance))
    , accModel_(new CurrentAccountFilterModel(instance, accSrcModel_.get()))
{
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_MODELS, accSrcModel_.get(), "AccountListModel");
    QML_REGISTERSINGLETONTYPE_POBJECT(NS_MODELS, accModel_.get(), "CurrentAccountFilterModel");
}

void
AccountAdapter::safeInit()
{
    connect(&lrcInstance_->accountModel(),
            &NewAccountModel::accountStatusChanged,
            this,
            &AccountAdapter::accountStatusChanged);

    connect(&lrcInstance_->accountModel(),
            &NewAccountModel::profileUpdated,
            this,
            &AccountAdapter::accountStatusChanged);
}

NewAccountModel*
AccountAdapter::getModel()
{
    return &(lrcInstance_->accountModel());
}

NewDeviceModel*
AccountAdapter::getDeviceModel()
{
    return lrcInstance_->getCurrentAccountInfo().deviceModel.get();
}

void
AccountAdapter::changeAccount(int row)
{
    auto accountList = lrcInstance_->accountModel().getAccountList();
    if (accountList.size() > row) {
        lrcInstance_->set_currentAccountId(accountList.at(row));
    }
}

void
AccountAdapter::connectFailure()
{
    Utils::oneShotConnect(&lrcInstance_->accountModel(),
                          &lrc::api::NewAccountModel::accountRemoved,
                          [this](const QString& accountId) {
                              Q_UNUSED(accountId);
                              Q_EMIT reportFailure();
                          });

    Utils::oneShotConnect(&lrcInstance_->accountModel(),
                          &lrc::api::NewAccountModel::invalidAccountDetected,
                          [this](const QString& accountId) {
                              Q_UNUSED(accountId);
                              Q_EMIT reportFailure();
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
#ifdef Q_OS_WIN
            confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
#endif
            confProps.isRendezVous = settings["isRendezVous"].toBool();
            lrcInstance_->accountModel().setAccountConfig(accountId, confProps);

            auto showBackup = isCreating
                              && !settingsManager_->getValue(Settings::Key::NeverShowMeAgain)
                                      .toBool();
            if (!registeredName.isEmpty()) {
                QObject::disconnect(registeredNameSavedConnection_);
                registeredNameSavedConnection_
                    = connect(&lrcInstance_->accountModel(),
                              &lrc::api::NewAccountModel::profileUpdated,
                              [this, showBackup, addedAccountId = accountId](
                                  const QString& accountId) {
                                  if (addedAccountId == accountId) {
                                      Q_EMIT lrcInstance_->accountListChanged();
                                      Q_EMIT accountAdded(accountId,
                                                          showBackup,
                                                          lrcInstance_->accountModel()
                                                              .getAccountList()
                                                              .indexOf(accountId));
                                      QObject::disconnect(registeredNameSavedConnection_);
                                  }
                              });

                lrcInstance_->accountModel().registerName(accountId,
                                                          settings["password"].toString(),
                                                          registeredName);
            } else {
                Q_EMIT lrcInstance_->accountListChanged();
                Q_EMIT accountAdded(accountId,
                                    showBackup,
                                    lrcInstance_->accountModel().getAccountList().indexOf(
                                        accountId));
            }
        });

    connectFailure();

    QtConcurrent::run([this, settings] {
        lrcInstance_->accountModel().createNewAccount(lrc::api::profile::Type::JAMI,
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
#ifdef Q_OS_WIN
                              confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
#endif
                              lrcInstance_->accountModel().setAccountConfig(accountId, confProps);

                              Q_EMIT lrcInstance_->accountListChanged();
                              Q_EMIT accountAdded(accountId,
                                                  false,
                                                  lrcInstance_->accountModel()
                                                      .getAccountList()
                                                      .indexOf(accountId));
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
#ifdef Q_OS_WIN
                              confProps.Ringtone.ringtonePath = Utils::GetRingtonePath();
#endif
                              lrcInstance_->accountModel().setAccountConfig(accountId, confProps);

                              Q_EMIT accountAdded(accountId,
                                                  false,
                                                  lrcInstance_->accountModel()
                                                      .getAccountList()
                                                      .indexOf(accountId));
                              Q_EMIT lrcInstance_->accountListChanged();
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
    lrcInstance_->accountModel().removeAccount(lrcInstance_->get_currentAccountId());
    Q_EMIT lrcInstance_->accountListChanged();
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
    if (!lrcInstance_->hasActiveCall(true) && lrcInstance_->renderer()->isPreviewing()) {
        lrcInstance_->renderer()->stopPreviewing();
    }
}

bool
AccountAdapter::hasVideoCall()
{
    return lrcInstance_->hasActiveCall(true);
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

bool
AccountAdapter::hasPassword()
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->get_currentAccountId());
    return confProps.archiveHasPassword;
}

void
AccountAdapter::setArchiveHasPassword(bool isHavePassword)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->get_currentAccountId());
    confProps.archiveHasPassword = isHavePassword;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->get_currentAccountId(), confProps);
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
