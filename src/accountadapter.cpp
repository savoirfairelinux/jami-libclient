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

#undef REGISTERED
#include "../daemon/src/dring/account_const.h"

AccountAdapter::AccountAdapter(QObject *parent)
    : QmlAdapterBase(parent)
{}

AccountAdapter::~AccountAdapter() {}

AccountAdapter &
AccountAdapter::instance()
{
    static auto instance = new AccountAdapter;
    return *instance;
}

void
AccountAdapter::initQmlObject()
{
    setSelectedAccount(LRCInstance::getCurrAccId());
}

void
AccountAdapter::accountChanged(int index)
{
    auto accountList = LRCInstance::accountModel().getAccountList();
    if (accountList.size() > index)
        setSelectedAccount(accountList.at(index));
}

void
AccountAdapter::connectFailure()
{
    Utils::oneShotConnect(&LRCInstance::accountModel(),
                          &lrc::api::NewAccountModel::accountRemoved,
                          [this](const QString &accountId) {
                              Q_UNUSED(accountId);

                              emit reportFailure();
                          });
    Utils::oneShotConnect(&LRCInstance::accountModel(),
                          &lrc::api::NewAccountModel::invalidAccountDetected,
                          [this](const QString &accountId) {
                              Q_UNUSED(accountId);

                              emit reportFailure();
                          });
}

void
AccountAdapter::createJamiAccount(QString registeredName,
                                  const QVariantMap &settings,
                                  QString photoBoothImgBase64,
                                  bool isCreating)
{
    Utils::oneShotConnect(
        &LRCInstance::accountModel(),
        &lrc::api::NewAccountModel::accountAdded,
        [this, registeredName, settings, isCreating, photoBoothImgBase64](const QString &accountId) {
            QSettings qSettings("jami.net", "Jami");
            if (not qSettings.contains(SettingsKey::neverShowMeAgain)) {
                qSettings.setValue(SettingsKey::neverShowMeAgain, false);
            }
            auto showBackup = isCreating && !settings.value(SettingsKey::neverShowMeAgain).toBool();

            if (!registeredName.isEmpty()) {
                Utils::oneShotConnect(&LRCInstance::accountModel(),
                                      &lrc::api::NewAccountModel::nameRegistrationEnded,
                                      [this, showBackup](const QString &accountId) {
                                          emit accountAdded(showBackup,
                                                            LRCInstance::accountModel()
                                                                .getAccountList()
                                                                .indexOf(accountId));
                                      });
                LRCInstance::accountModel().registerName(accountId,
                                                         settings["password"].toString(),
                                                         registeredName);
            } else {
                emit accountAdded(showBackup,
                                  LRCInstance::accountModel().getAccountList().indexOf(accountId));
            }
            // set up avatar pixmap from photobooth
            QImage avatarImg;
            const bool ret = avatarImg.loadFromData(
                QByteArray::fromBase64(photoBoothImgBase64.toLatin1()));
            if (!ret) {
                qDebug() << "JAMI account creation BASE64 image loading failed";
            } else {
                LRCInstance::setAvatarForAccount(QPixmap::fromImage(avatarImg), accountId);
            }
        });

    connectFailure();

    QtConcurrent::run([this, settings] {
        QMap<QString, QString> additionalAccountConfig;
        additionalAccountConfig.insert(DRing::Account::ConfProperties::Ringtone::PATH,
                                       Utils::GetRingtonePath());

        LRCInstance::accountModel().createNewAccount(lrc::api::profile::Type::RING,
                                                     settings["alias"].toString(),
                                                     settings["archivePath"].toString(),
                                                     settings["password"].toString(),
                                                     settings["archivePin"].toString(),
                                                     "",
                                                     additionalAccountConfig);
    });
}

void
AccountAdapter::createSIPAccount(const QVariantMap &settings, QString photoBoothImgBase64)
{
    Utils::oneShotConnect(&LRCInstance::accountModel(),
                          &lrc::api::NewAccountModel::accountAdded,
                          [this, settings, photoBoothImgBase64](const QString &accountId) {
                              auto confProps = LRCInstance::accountModel().getAccountConfig(
                                  accountId);
                              // set SIP details
                              confProps.hostname = settings["hostname"].toString();
                              confProps.username = settings["username"].toString();
                              confProps.password = settings["password"].toString();
                              confProps.proxyServer = settings["proxy"].toString();
                              LRCInstance::accountModel().setAccountConfig(accountId, confProps);

                              // set up photobooth avatar to SIP avatar
                              QImage avatarImg;
                              const bool ret = avatarImg.loadFromData(
                                  QByteArray::fromBase64(photoBoothImgBase64.toLatin1()));
                              if (!ret) {
                                  qDebug() << "SIP account creation BASE64 image loading failed";
                              } else {
                                  LRCInstance::setAvatarForAccount(QPixmap::fromImage(avatarImg),
                                                                   accountId);
                              }

                              emit accountAdded(false,
                                                LRCInstance::accountModel().getAccountList().indexOf(
                                                    accountId));
                          });

    connectFailure();

    QtConcurrent::run([this, settings] {
        QMap<QString, QString> additionalAccountConfig;
        additionalAccountConfig.insert(DRing::Account::ConfProperties::Ringtone::PATH,
                                       Utils::GetRingtonePath());

        LRCInstance::accountModel().createNewAccount(lrc::api::profile::Type::SIP,
                                                     settings["alias"].toString(),
                                                     settings["archivePath"].toString(),
                                                     "",
                                                     "",
                                                     settings["username"].toString(),
                                                     additionalAccountConfig);
        QThread::sleep(2);
        emit LRCInstance::instance().accountListChanged();
    });
}

void
AccountAdapter::createJAMSAccount(const QVariantMap &settings)
{
    Utils::oneShotConnect(&LRCInstance::accountModel(),
                          &lrc::api::NewAccountModel::accountAdded,
                          [this](const QString &accountId) {
                              Q_UNUSED(accountId)
                              if (!LRCInstance::accountModel().getAccountList().size())
                                  return;
                              emit accountAdded(false,
                                                LRCInstance::accountModel().getAccountList().indexOf(
                                                    accountId));
                              emit LRCInstance::instance().accountListChanged();
                          });

    connectFailure();

    QtConcurrent::run([this, settings] {
        QMap<QString, QString> additionalAccountConfig;
        additionalAccountConfig.insert(DRing::Account::ConfProperties::Ringtone::PATH,
                                       Utils::GetRingtonePath());

        LRCInstance::accountModel().connectToAccountManager(settings["username"].toString(),
                                                            settings["password"].toString(),
                                                            settings["manager"].toString(),
                                                            additionalAccountConfig);
    });
}

void
AccountAdapter::deleteCurrentAccount()
{
    LRCInstance::accountModel().removeAccount(LRCInstance::getCurrAccId());
}

bool
AccountAdapter::savePassword(const QString accountId,
                             const QString oldPassword,
                             const QString newPassword)
{
    return LRCInstance::accountModel().changeAccountPassword(accountId, oldPassword, newPassword);
}

void
AccountAdapter::startAudioMeter(bool async)
{
    LRCInstance::startAudioMeter(async);
}

void
AccountAdapter::stopAudioMeter(bool async)
{
    LRCInstance::stopAudioMeter(async);
}

void
AccountAdapter::startPreviewing(bool force, bool async)
{
    LRCInstance::renderer()->startPreviewing(force, async);
}

void
AccountAdapter::stopPreviewing(bool async)
{
    if (!LRCInstance::hasVideoCall() && LRCInstance::renderer()->isPreviewing()) {
        LRCInstance::renderer()->stopPreviewing(async);
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
AccountAdapter::setCurrAccDisplayName(QString text)
{
    LRCInstance::setCurrAccDisplayName(text);
}

void
AccountAdapter::setSelectedAccountId(QString accountId)
{
    LRCInstance::setSelectedAccountId(accountId);
}

void
AccountAdapter::setSelectedConvId(QString accountId)
{
    LRCInstance::setSelectedConvId(accountId);
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
AccountAdapter::exportToFile(const QString &accountId,
                             const QString &path,
                             const QString &password) const
{
    return LRCInstance::accountModel().exportToFile(accountId, path, password);
}

void
AccountAdapter::setArchivePasswordAsync(const QString &accountID, const QString &password)
{
    QtConcurrent::run([this, accountID, password] {
        auto config = LRCInstance::accountModel().getAccountConfig(accountID);
        config.archivePassword = password;
        LRCInstance::accountModel().setAccountConfig(accountID, config);
    });
}

void
AccountAdapter::settingsNeverShowAgain(bool checked)
{
    QSettings settings("jami.net", "Jami");
    settings.setValue(SettingsKey::neverShowMeAgain, checked);
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
AccountAdapter::setSelectedAccount(const QString &accountId)
{
    LRCInstance::setSelectedAccountId(accountId);

    connectAccount(accountId);
    emit accountSignalsReconnect(accountId);

    backToWelcomePage();
}

void
AccountAdapter::backToWelcomePage()
{
    deselectConversation();
    QMetaObject::invokeMethod(qmlObj_, "backToWelcomePage");
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

    currentConversationModel->selectConversation("");
    LRCInstance::setSelectedConvId();
}

void
AccountAdapter::connectAccount(const QString &accountId)
{
    try {
        auto &accInfo = LRCInstance::accountModel().getAccountInfo(accountId);

        QObject::disconnect(accountStatusChangedConnection_);
        QObject::disconnect(contactAddedConnection_);

        accountStatusChangedConnection_
            = QObject::connect(accInfo.accountModel,
                               &lrc::api::NewAccountModel::accountStatusChanged,
                               [this] { emit accountStatusChanged(); });

        contactAddedConnection_
            = QObject::connect(accInfo.contactModel.get(),
                               &lrc::api::ContactModel::contactAdded,
                               [this, accountId](const QString &contactUri) {
                                   auto &accInfo = LRCInstance::accountModel().getAccountInfo(
                                       accountId);
                                   auto* convModel = LRCInstance::getCurrentConversationModel();
                                   const auto conversation = convModel->getConversationForUID(
                                       LRCInstance::getCurrentConvUid());
                                   if (conversation.uid.isEmpty()) {
                                       return;
                                   }
                                   if (contactUri
                                       == accInfo.contactModel
                                              ->getContact(conversation.participants.at(0))
                                              .profileInfo.uri) {
                                       /*
                                        * Update conversation.
                                        */
                                       emit updateConversationForAddedContact();
                                   }
                               });
        QObject::disconnect(addedToConferenceConnection_);
        addedToConferenceConnection_
            = QObject::connect(accInfo.callModel.get(),
                               &NewCallModel::callAddedToConference,
                               [this](const QString &callId, const QString &confId) {
                                   Q_UNUSED(callId);
                                   LRCInstance::renderer()->addDistantRenderer(confId);
                               });
    } catch (...) {
        qWarning() << "Couldn't get account: " << accountId;
    }
}
