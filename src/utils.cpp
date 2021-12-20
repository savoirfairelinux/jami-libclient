/*!
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com
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

#include "utils.h"

#include "jamiavatartheme.h"
#include "lrcinstance.h"

#include <qrencode.h>

#include <QApplication>
#include <QBitmap>
#include <QErrorMessage>
#include <QFile>
#include <QMessageBox>
#include <QObject>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScreen>
#include <QDateTime>
#include <QSvgRenderer>
#include <QTranslator>
#include <QtConcurrent/QtConcurrent>
#include <QUuid>

#ifdef Q_OS_WIN
#include <lmcons.h>
#include <shlguid.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <windows.h>
#endif

bool
Utils::CreateStartupLink(const std::wstring& wstrAppName)
{
#ifdef Q_OS_WIN
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);

    std::wstring programPath(szPath);

    TCHAR startupPath[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath);

    std::wstring linkPath(startupPath);
    linkPath += std::wstring(TEXT("\\") + wstrAppName + TEXT(".lnk"));

    return Utils::CreateLink(programPath.c_str(), linkPath.c_str());
#else
    Q_UNUSED(wstrAppName)
    QString desktopPath;
    /* cmake should set JAMI_INSTALL_PREFIX, otherwise it checks the following dirs
     *  - /usr/<data dir>
     *  - /usr/local/<data dir>
     *  - default install data dir
     */

#ifdef JAMI_INSTALL_PREFIX
    desktopPath = JAMI_INSTALL_PREFIX;
    desktopPath += "/jami-qt/jami-qt.desktop";
#else
    desktopPath = "share/jami-qt/jami-qt.desktop";
    QStringList paths = {"/usr/" + desktopPath,
                         "/usr/local/" + desktopPath,
                         QDir::currentPath() + "/../../install/client-qt/" + desktopPath};
    for (QString filename : paths) {
        if (QFile::exists(filename)) {
            desktopPath = filename;
            break;
        }
    }
#endif

    if (desktopPath.isEmpty() || !(QFile::exists(desktopPath))) {
        qDebug() << "Could not locate .desktop file at" << desktopPath;
        return false;
    }

    qDebug() << "Linking autostart file from" << desktopPath;

    QString desktopFile = QStandardPaths::locate(QStandardPaths::ConfigLocation,
                                                 "autostart/jami-qt.desktop");
    if (!desktopFile.isEmpty()) {
        QFileInfo symlinkInfo(desktopFile);
        if (symlinkInfo.isSymLink()) {
            if (symlinkInfo.symLinkTarget() == desktopPath) {
                qDebug() << desktopFile << "already points to" << desktopPath;
                return true;
            } else {
                qDebug() << desktopFile << "exists but does not point to" << desktopPath;
                QFile::remove(desktopFile);
            }
        }
    } else {
        QString autoStartDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                               + "/autostart";

        if (!QDir(autoStartDir).exists()) {
            if (QDir().mkdir(autoStartDir)) {
                qDebug() << "Created autostart directory:" << autoStartDir;
            } else {
                qWarning() << "Could not create autostart directory:" << autoStartDir;
                return false;
            }
        }
        desktopFile = autoStartDir + "/jami-qt.desktop";
    }

    QFile srcFile(desktopPath);
    bool result = srcFile.link(desktopFile);
    qDebug() << desktopFile
             << (result ? "-> " + desktopPath + " successfully created" : "could not be created");
    return result;
#endif
}

bool
Utils::CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink)
{
#ifdef Q_OS_WIN
    HRESULT hres;
    IShellLink* psl;

    hres = CoCreateInstance(CLSID_ShellLink,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellLink,
                            (LPVOID*) &psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;
        psl->SetPath(lpszPathObj);
        psl->SetArguments(TEXT("--minimized"));

        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*) &ppf);
        if (SUCCEEDED(hres)) {
            hres = ppf->Save(lpszPathLink, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
#else
    Q_UNUSED(lpszPathObj)
    Q_UNUSED(lpszPathLink)
    return true;
#endif
}

void
Utils::DeleteStartupLink(const std::wstring& wstrAppName)
{
#ifdef Q_OS_WIN
    TCHAR startupPath[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath);

    std::wstring linkPath(startupPath);
    linkPath += std::wstring(TEXT("\\") + wstrAppName + TEXT(".lnk"));

    DeleteFile(linkPath.c_str());

#else
    Q_UNUSED(wstrAppName)
    QString desktopFile = QStandardPaths::locate(QStandardPaths::ConfigLocation,
                                                 "autostart/jami-qt.desktop");
    if (!desktopFile.isEmpty()) {
        try {
            QFile::remove(desktopFile);
            qDebug() << "Autostart disabled," << desktopFile << "removed";
        } catch (...) {
            qDebug() << "Could not remove" << desktopFile;
        }
    } else {
        qDebug() << desktopFile << "does not exist";
    }
#endif
}

bool
Utils::CheckStartupLink(const std::wstring& wstrAppName)
{
#ifdef Q_OS_WIN
    TCHAR startupPath[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath);

    std::wstring linkPath(startupPath);
    linkPath += std::wstring(TEXT("\\") + wstrAppName + TEXT(".lnk"));
    return PathFileExists(linkPath.c_str());
#else
    Q_UNUSED(wstrAppName)
    return (!QStandardPaths::locate(QStandardPaths::ConfigLocation, "autostart/jami-qt.desktop")
                 .isEmpty());
#endif
}

const char*
Utils::WinGetEnv(const char* name)
{
#ifdef Q_OS_WIN
    const DWORD buffSize = 65535;
    static char buffer[buffSize];
    if (GetEnvironmentVariableA(name, buffer, buffSize)) {
        return buffer;
    } else {
        return 0;
    }
#else
    Q_UNUSED(name)
    return 0;
#endif
}

void
Utils::removeOldVersions()
{
#ifdef Q_OS_WIN
    /*
     * As per: https://git.jami.net/savoirfairelinux/ring-client-windows/issues/429
     * NB: As only the 64-bit version of this application is distributed, we will only
     * remove 1. the configuration reg keys for Ring-x64, 2. the startup links for Ring,
     * 3. the winsparkle reg keys. The NSIS uninstall reg keys for Jami-x64 are removed
     * by the MSI installer.
     * Uninstallation of Ring, either 32 or 64 bit, is left to the user.
     * The current version of Jami will attempt to kill Ring.exe upon start if a startup
     * link is found.
     */
    QString node64 = "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node";
    QString hkcuSoftwareKey = "HKEY_CURRENT_USER\\Software\\";
    QString uninstKey = "\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
    QString company = "Savoir-Faire Linux";

    /*
     * 1. Configuration reg keys for Ring-x64.
     */
    QSettings(hkcuSoftwareKey + "jami.net\\Ring", QSettings::NativeFormat).remove("");
    QSettings(hkcuSoftwareKey + "ring.cx", QSettings::NativeFormat).remove("");

    /*
     * 2. Unset Ring as a startup application.
     */
    if (Utils::CheckStartupLink(TEXT("Ring"))) {
        qDebug() << "Found startup link for Ring. Removing it and killing Ring.exe.";
        Utils::DeleteStartupLink(TEXT("Ring"));
        QProcess process;
        process.start("taskkill",
                      QStringList() << "/im"
                                    << "Ring.exe"
                                    << "/f");
        process.waitForFinished();
    }

    /*
     * 3. Remove registry entries for winsparkle(both Jami-x64 and Ring-x64).
     */
    QSettings(hkcuSoftwareKey + company, QSettings::NativeFormat).remove("");
#else
    return;
#endif
}

QString
Utils::GetRingtonePath()
{
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + "\\ringtones\\default.opus";
#else
    return QString("/usr/share/ring/ringtones/default.opus");
#endif
}

QString
Utils::GenGUID()
{
#ifdef Q_OS_WIN
    GUID gidReference;
    wchar_t* str;
    HRESULT hCreateGuid = CoCreateGuid(&gidReference);
    if (hCreateGuid == S_OK) {
        StringFromCLSID(gidReference, &str);
        auto gStr = QString::fromWCharArray(str);
        return gStr.remove("{").remove("}").toLower();
    } else
        return QString();
#else
    return QString("");
#endif
}

QString
Utils::GetISODate()
{
#ifdef Q_OS_WIN
    SYSTEMTIME lt;
    GetSystemTime(&lt);
    return QString("%1-%2-%3T%4:%5:%6Z")
        .arg(lt.wYear)
        .arg(lt.wMonth, 2, 10, QChar('0'))
        .arg(lt.wDay, 2, 10, QChar('0'))
        .arg(lt.wHour, 2, 10, QChar('0'))
        .arg(lt.wMinute, 2, 10, QChar('0'))
        .arg(lt.wSecond, 2, 10, QChar('0'));
#else
    return QString();
#endif
}

QImage
Utils::accountPhoto(LRCInstance* instance, const QString& accountId, const QSize& size)
{
    QImage photo;
    try {
        auto& accInfo = instance->accountModel().getAccountInfo(
            accountId.isEmpty() ? instance->get_currentAccountId() : accountId);
        if (!accInfo.profileInfo.avatar.isEmpty()) {
            photo = imageFromBase64String(accInfo.profileInfo.avatar);
        } else {
            auto bestName = instance->accountModel().bestNameForAccount(accInfo.id);
            QString name = bestName == accInfo.profileInfo.uri ? QString() : bestName;
            QString prefix = accInfo.profileInfo.type == profile::Type::JAMI ? "jami:" : "sip:";
            photo = fallbackAvatar(prefix + accInfo.profileInfo.uri, name, size);
        }
    } catch (const std::exception& e) {
        qDebug() << e.what() << "; Using default avatar";
        photo = fallbackAvatar(QString(), QString(), size);
    }
    return Utils::scaleAndFrame(photo, size);
}

QImage
Utils::contactPhoto(LRCInstance* instance,
                    const QString& contactUri,
                    const QSize& size,
                    const QString& accountId)
{
    QImage photo;
    try {
        auto& accInfo = instance->accountModel().getAccountInfo(
            accountId.isEmpty() ? instance->get_currentAccountId() : accountId);
        auto contactInfo = accInfo.contactModel->getContact(contactUri);
        auto contactPhoto = accInfo.contactModel->avatar(contactUri);
        auto bestName = accInfo.contactModel->bestNameForContact(contactUri);
        if (accInfo.profileInfo.type == profile::Type::SIP
            && contactInfo.profileInfo.type == profile::Type::TEMPORARY) {
            photo = Utils::fallbackAvatar(QString(), QString());
        } else if (contactInfo.profileInfo.type == profile::Type::TEMPORARY
                   && contactInfo.profileInfo.uri.isEmpty()) {
            photo = Utils::fallbackAvatar(QString(), QString());
        } else if (!contactPhoto.isEmpty()) {
            photo = imageFromBase64String(contactPhoto);
            if (photo.isNull()) {
                auto avatarName = contactInfo.profileInfo.uri == bestName ? QString() : bestName;
                photo = Utils::fallbackAvatar("jami:" + contactInfo.profileInfo.uri, avatarName);
            }
        } else {
            auto avatarName = contactInfo.profileInfo.uri == bestName ? QString() : bestName;
            photo = Utils::fallbackAvatar("jami:" + contactInfo.profileInfo.uri, avatarName);
        }
    } catch (const std::exception& e) {
        qDebug() << e.what() << "; Using default avatar";
        photo = fallbackAvatar("jami:" + contactUri, QString(), size);
    }
    return Utils::scaleAndFrame(photo, size);
}

QImage
Utils::conversationAvatar(LRCInstance* instance,
                          const QString& convId,
                          const QSize& size,
                          const QString& accountId)
{
    QImage avatar(size, QImage::Format_ARGB32_Premultiplied);
    avatar.fill(Qt::transparent);
    QPainter painter(&avatar);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    try {
        auto& accInfo = instance->accountModel().getAccountInfo(
            accountId.isEmpty() ? instance->get_currentAccountId() : accountId);
        auto* convModel = accInfo.conversationModel.get();
        Q_FOREACH (const auto peerUri, convModel->peersForConversation(convId)) {
            auto peerAvatar = Utils::contactPhoto(instance, peerUri, size);
            painter.drawImage(avatar.rect(), peerAvatar);
        }
    } catch (const std::exception& e) {
        qDebug() << Q_FUNC_INFO << e.what();
    }
    return avatar;
}

QImage
Utils::imageFromBase64String(const QString& str, bool circleCrop)
{
    return imageFromBase64Data(Utils::base64StringToByteArray(str), circleCrop);
}

QImage
Utils::imageFromBase64Data(const QByteArray& data, bool circleCrop)
{
    QImage img;

    if (img.loadFromData(data)) {
        if (circleCrop) {
            return Utils::getCirclePhoto(img, img.size().width());
        }
        return img;
    }

    qWarning() << Q_FUNC_INFO << "Image loading failed";
    return {};
}

QImage
Utils::getCirclePhoto(const QImage original, int sizePhoto)
{
    QImage target(sizePhoto, sizePhoto, QImage::Format_ARGB32_Premultiplied);
    target.fill(Qt::transparent);

    QPainter painter(&target);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setBrush(QBrush(Qt::white));
    auto scaledPhoto = original
                           .scaled(sizePhoto,
                                   sizePhoto,
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation)
                           .convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int margin = 0;
    if (scaledPhoto.width() > sizePhoto) {
        margin = (scaledPhoto.width() - sizePhoto) / 2;
    }
    painter.drawEllipse(0, 0, sizePhoto, sizePhoto);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.drawImage(0, 0, scaledPhoto, margin, 0);
    return target;
}

QSize
Utils::getRealSize(QScreen* screen)
{
#ifdef Q_OS_WIN
    DEVMODE dmThisScreen;
    ZeroMemory(&dmThisScreen, sizeof(dmThisScreen));
    EnumDisplaySettings((const wchar_t*) screen->name().utf16(),
                        ENUM_CURRENT_SETTINGS,
                        (DEVMODE*) &dmThisScreen);
    return QSize(dmThisScreen.dmPelsWidth, dmThisScreen.dmPelsHeight);
#else
    Q_UNUSED(screen)
    return {};
#endif
}

void
Utils::forceDeleteAsync(const QString& path)
{
    /*
     * Keep deleting file until the process holding it let go,
     * or the file itself does not exist anymore.
     */
    auto futureResult = QtConcurrent::run([path] {
        QFile file(path);
        if (!QFile::exists(path))
            return;
        int retries {0};
        while (!file.remove() && retries < 5) {
            qDebug().noquote() << "\n" << file.errorString() << "\n";
            QThread::msleep(10);
            ++retries;
        }
    });
}

QString
Utils::getProjectCredits()
{
    QString credits;
    QFile projectCreditsFile(":/misc/projectcredits.html");
    if (!projectCreditsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << " Project Credits failed to load";
        return {};
    }
    QTextStream in(&projectCreditsFile);
    while (!in.atEnd()) {
        QString currentLine = in.readLine();
        if (currentLine.contains("Created by:")) {
            credits += "<h3 align=\"center\" style=\" margin-top:0px; "
                       + QString("margin-bottom:0px; margin-left:0px; margin-right:0px; ")
                       + "-qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">"
                       + QObject::tr("Created by:") + "</span></h3>";
            continue;
        } else if (currentLine.contains("Artwork by:")) {
            credits
                += "<h3 align=\"center\" style=\" margin-top:0px; margin-bottom:0px; "
                   + QString(
                       "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">")
                   + "<span style=\" font-weight:600;\">" + QObject::tr("Artwork by:")
                   + "</span></h3>";
            continue;
        }
        credits += currentLine;
    }
    credits += "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; "
               + QString(
                   "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">")
               + QObject::tr("Based on the SFLPhone project") + "</p>";

    return credits;
}

inline QString
removeEndlines(const QString& str)
{
    QString trimmed(str);
    trimmed.remove(QChar('\n'));
    trimmed.remove(QChar('\r'));
    return trimmed;
}

lrc::api::profile::Type
Utils::profileType(const lrc::api::conversation::Info& conv,
                   const lrc::api::ConversationModel& model)
{
    try {
        auto contact = model.owner.contactModel->getContact(conv.participants[0]);
        return contact.profileInfo.type;
    } catch (const std::out_of_range& e) {
        qDebug() << e.what();
        return lrc::api::profile::Type::INVALID;
    }
}

QString
Utils::formatTimeString(const std::time_t& timeStamp)
{
    auto currentTimeStamp = QDateTime::fromSecsSinceEpoch(timeStamp);
    auto now = QDateTime::currentDateTime();
    auto timeStampDMY = currentTimeStamp.toString("dd/MM/yy");
    if (timeStampDMY == now.toString("dd/MM/yy")) {
        return currentTimeStamp.toString("hh:mm");
    } else {
        return timeStampDMY;
    }
}

bool
Utils::isInteractionGenerated(const lrc::api::interaction::Type& type)
{
    return type == lrc::api::interaction::Type::CALL
           || type == lrc::api::interaction::Type::CONTACT;
}

bool
Utils::isContactValid(const QString& contactUid, const lrc::api::ConversationModel& model)
{
    try {
        const auto contact = model.owner.contactModel->getContact(contactUid);
        return (contact.profileInfo.type == lrc::api::profile::Type::PENDING
                || contact.profileInfo.type == lrc::api::profile::Type::TEMPORARY
                || contact.profileInfo.type == lrc::api::profile::Type::JAMI
                || contact.profileInfo.type == lrc::api::profile::Type::SIP)
               && !contact.profileInfo.uri.isEmpty();
    } catch (const std::out_of_range& e) {
        qDebug() << e.what();
        return false;
    }
}

bool
Utils::getReplyMessageBox(QWidget* widget, const QString& title, const QString& text)
{
    if (QMessageBox::question(widget, title, text, QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes)
        return true;
    return false;
}

QColor
Utils::getAvatarColor(const QString& canonicalUri)
{
    if (canonicalUri.isEmpty()) {
        return JamiAvatarTheme::defaultAvatarColor_;
    }
    auto h = QString(
        QCryptographicHash::hash(canonicalUri.toLocal8Bit(), QCryptographicHash::Md5).toHex());
    if (h.isEmpty() || h.isNull()) {
        return JamiAvatarTheme::defaultAvatarColor_;
    }
    auto colorIndex = std::string("0123456789abcdef").find(h.at(0).toLatin1());
    return JamiAvatarTheme::avatarColors_[colorIndex];
}

/*!
 * Generate a QImage representing a default user avatar, when the user doesn't provide it.
 * If the name passed is empty, then the default avatar picture will be displayed instead
 * of a letter.
 *
 * @param canonicalUri uri containing the account type prefix used to obtain the bgcolor
 * @param name the string used to acquire the letter centered in the avatar
 * @param size the dimensions of the desired image
 */
QImage
Utils::fallbackAvatar(const QString& canonicalUri, const QString& name, const QSize& size)
{
    auto sizeToUse = size.height() >= defaultAvatarSize.height() ? size : defaultAvatarSize;

    QImage avatar(sizeToUse, QImage::Format_ARGB32);
    avatar.fill(Qt::transparent);

    QPainter painter(&avatar);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::transparent);

    // background circle
    painter.setBrush(getAvatarColor(canonicalUri).lighter(110));
    painter.drawEllipse(avatar.rect());

    // if a letter was passed, then we paint a letter in the circle,
    // otherwise we draw the default avatar icon
    QString trimmedName(name);
    if (!trimmedName.remove(QRegularExpression("[\\n\\t\\r]")).isEmpty()) {
        auto unicode = trimmedName.toUcs4().at(0);
        if (unicode >= 0x1F000 && unicode <= 0x1FFFF) {
            // emoticon
            auto letter = QString::fromUcs4(reinterpret_cast<char32_t*>(&unicode), 1);
            QFont font(QString("Segoe UI Emoji").split(QLatin1Char(',')),
                       avatar.height() / 2.66667,
                       QFont::Medium);
            painter.setFont(font);
            QRect emojiRect(avatar.rect());
            emojiRect.moveTop(-6);
            painter.drawText(emojiRect, letter, QTextOption(Qt::AlignCenter));
        } else if (unicode >= 0x0000 && unicode <= 0x00FF) {
            // basic Latin
            auto letter = trimmedName.at(0).toUpper();
            QFont font(QString("Arial").split(QLatin1Char(',')),
                       avatar.height() / 2.66667,
                       QFont::Medium);
            painter.setFont(font);
            painter.setPen(Qt::white);
            painter.drawText(avatar.rect(), QString(letter), QTextOption(Qt::AlignCenter));
        } else {
            auto letter = QString::fromUcs4(reinterpret_cast<char32_t*>(&unicode), 1);
            QFont font(QString("Arial").split(QLatin1Char(',')),
                       avatar.height() / 2.66667,
                       QFont::Medium);
            painter.setFont(font);
            painter.setPen(Qt::white);
            painter.drawText(avatar.rect(), QString(letter), QTextOption(Qt::AlignCenter));
        }
    } else {
        QRect overlayRect = avatar.rect();
        qreal margin = (0.05 * overlayRect.width());
        overlayRect.moveLeft(overlayRect.left() + margin * 0.5);
        overlayRect.moveTop(overlayRect.top() + margin * 0.5);
        overlayRect.setWidth(overlayRect.width() - margin);
        overlayRect.setHeight(overlayRect.height() - margin);
        painter.drawPixmap(overlayRect, QPixmap(":/images/default_avatar_overlay.svg"));
    }

    return avatar.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QImage
Utils::fallbackAvatar(const std::string& alias, const std::string& uri, const QSize& size)
{
    return fallbackAvatar(QString::fromStdString(uri), QString::fromStdString(alias), size);
}

QByteArray
Utils::QImageToByteArray(QImage image)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return ba;
}

QString
Utils::byteArrayToBase64String(QByteArray byteArray)
{
    return QString::fromLatin1(byteArray.toBase64().data());
}

QByteArray
Utils::base64StringToByteArray(QString base64)
{
    return QByteArray::fromBase64(base64.toLatin1());
}

QImage
Utils::cropImage(const QImage& img)
{
    auto w = img.width();
    auto h = img.height();
    if (w > h) {
        return img.copy({(w - h) / 2, 0, h, h});
    }
    return img.copy({0, (h - w) / 2, w, w});
}

QPixmap
Utils::pixmapFromSvg(const QString& svg_resource, const QSize& size)
{
    QSvgRenderer svgRenderer(svg_resource);
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter pixPainter(&pixmap);
    svgRenderer.render(&pixPainter);
    return pixmap;
}

QImage
Utils::setupQRCode(QString ringID, int margin)
{
    auto qrcode = QRcode_encodeString(ringID.toStdString().c_str(),
                                      0,            // Let the version be decided by libqrencode
                                      QR_ECLEVEL_L, // Lowest level of error correction
                                      QR_MODE_8,    // 8-bit data mode
                                      1);
    if (not qrcode) {
        qWarning() << "Failed to generate QR code";
        return QImage();
    }

    int qrwidth = qrcode->width + margin * 2;
    QImage result(QSize(qrwidth, qrwidth), QImage::Format_Mono);
    QPainter painter;
    painter.begin(&result);
    painter.setClipRect(QRect(0, 0, qrwidth, qrwidth));
    painter.setPen(QPen(Qt::black, 0.1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.setBrush(Qt::black);
    painter.fillRect(QRect(0, 0, qrwidth, qrwidth), Qt::white);
    unsigned char* p;
    p = qrcode->data;
    for (int y = 0; y < qrcode->width; y++) {
        unsigned char* row = (p + (y * qrcode->width));
        for (int x = 0; x < qrcode->width; x++) {
            if (*(row + x) & 0x1) {
                painter.drawRect(margin + x, margin + y, 1, 1);
            }
        }
    }
    painter.end();
    QRcode_free(qrcode);
    return result;
}

QString
Utils::formattedTime(int duration)
{
    if (duration == 0)
        return {};
    std::string formattedString;
    auto minutes = duration / 60;
    auto seconds = duration % 60;
    if (minutes > 0) {
        formattedString += std::to_string(minutes) + ":";
        if (formattedString.length() == 2) {
            formattedString = "0" + formattedString;
        }
    } else {
        formattedString += "00:";
    }
    if (seconds < 10)
        formattedString += "0";
    formattedString += std::to_string(seconds);
    return QString::fromStdString(formattedString);
}

QByteArray
Utils::QByteArrayFromFile(const QString& filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    } else {
        qDebug() << "QByteArrayFromFile: can't open file";
        return QByteArray();
    }
}

QPixmap
Utils::generateTintedPixmap(const QString& filename, QColor color)
{
    QPixmap px(filename);
    QImage tmpImage = px.toImage();
    for (int y = 0; y < tmpImage.height(); y++) {
        for (int x = 0; x < tmpImage.width(); x++) {
            color.setAlpha(tmpImage.pixelColor(x, y).alpha());
            tmpImage.setPixelColor(x, y, color);
        }
    }
    return QPixmap::fromImage(tmpImage);
}

QPixmap
Utils::generateTintedPixmap(const QPixmap& pix, QColor color)
{
    QPixmap px = pix;
    QImage tmpImage = px.toImage();
    for (int y = 0; y < tmpImage.height(); y++) {
        for (int x = 0; x < tmpImage.width(); x++) {
            color.setAlpha(tmpImage.pixelColor(x, y).alpha());
            tmpImage.setPixelColor(x, y, color);
        }
    }
    return QPixmap::fromImage(tmpImage);
}

QImage
Utils::scaleAndFrame(const QImage photo, const QSize& size)
{
    return photo.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QString
Utils::humanFileSize(qint64 fileSize)
{
    float fileSizeF = static_cast<float>(fileSize);
    float thresh = 1024;

    if (abs(fileSizeF) < thresh) {
        return QString::number(fileSizeF) + " B";
    }
    QString units[] = {"KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    int unit_position = -1;
    do {
        fileSizeF /= thresh;
        ++unit_position;
    } while (abs(fileSizeF) >= thresh && unit_position < units->size() - 1);
    /*
     * Round up to two decimal.
     */
    fileSizeF = roundf(fileSizeF * 100) / 100;
    return QString::number(fileSizeF) + " " + units[unit_position];
}

bool
Utils::isImage(const QString& fileExt)
{
    if (fileExt == "png" || fileExt == "jpg" || fileExt == "jpeg")
        return true;
    return false;
}

QString
Utils::generateUid()
{
    return QUuid::createUuid().toString(QUuid::Id128);
}
