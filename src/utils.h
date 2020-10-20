/*!
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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

#pragma once

#include <QCryptographicHash>
#include <QDir>

#include <QImage>
#include <QItemDelegate>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QQmlEngine>
#include <QSettings>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QString>
#include <QTextDocument>
#include <QtGlobal>

#include <string>

#ifdef Q_OS_WIN
#include <ciso646>
#include <windows.h>
#undef ERROR
#else
#define LPCWSTR char*
#endif

#include "api/account.h"
#include "api/contact.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"

namespace Utils {

/*
 * App/System
 */
bool CreateStartupLink(const std::wstring& wstrAppName);
void DeleteStartupLink(const std::wstring& wstrAppName);
bool CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink);
bool CheckStartupLink(const std::wstring& wstrAppName);
const char* WinGetEnv(const char* name);
QString GetRingtonePath();
QString GenGUID();
QString GetISODate();
void showNotification(const QString& message,
                      const QString& from,
                      const QString& accountId,
                      const QString& convUid,
                      std::function<void()> const& onClicked);
QSize getRealSize(QScreen* screen);
void forceDeleteAsync(const QString& path);
QString getChangeLog();
QString getProjectCredits();
void removeOldVersions();

/*
 * LRC helpers
 */
lrc::api::profile::Type profileType(const lrc::api::conversation::Info& conv,
                                    const lrc::api::ConversationModel& model);
std::string formatTimeString(const std::time_t& timestamp);
bool isInteractionGenerated(const lrc::api::interaction::Type& interaction);
bool isContactValid(const QString& contactUid, const lrc::api::ConversationModel& model);
bool getReplyMessageBox(QWidget* widget, const QString& title, const QString& text);

/*
 * Image manipulation
 */
static const QSize defaultAvatarSize {128, 128};
QImage contactPhotoFromBase64(const QByteArray& data, const QString& type);
QImage contactPhoto(const QString& contactUri, const QSize& size = defaultAvatarSize);
QImage getCirclePhoto(const QImage original, int sizePhoto);
QColor getAvatarColor(const QString& canonicalUri);
QImage fallbackAvatar(const QString& canonicalUriStr,
                      const QString& letterStr = QString(),
                      const QSize& size = defaultAvatarSize);
QImage fallbackAvatar(const std::string& alias,
                      const std::string& uri,
                      const QSize& size = defaultAvatarSize);
QByteArray QImageToByteArray(QImage image);
QByteArray QByteArrayFromFile(const QString& filename);
QPixmap generateTintedPixmap(const QString& filename, QColor color);
QPixmap generateTintedPixmap(const QPixmap& pix, QColor color);
QImage scaleAndFrame(const QImage photo, const QSize& size = defaultAvatarSize);
QImage accountPhoto(const lrc::api::account::Info& accountInfo,
                    const QSize& size = defaultAvatarSize);
QImage cropImage(const QImage& img);
QPixmap pixmapFromSvg(const QString& svg_resource, const QSize& size);
QImage setupQRCode(QString ringID, int margin);
bool isImage(const QString& fileExt);
QString generateUid();

/*
 * Misc
 */
QString formattedTime(int seconds);
QString humanFileSize(qint64 fileSize);

} // namespace Utils
