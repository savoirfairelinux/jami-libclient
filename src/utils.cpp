/*
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Aline Gondim Santos   <aline.gondimsantos@savoirfairelinux.com>
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

#ifdef Q_OS_WIN
#include <lmcons.h>
#include <shlguid.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <windows.h>
#endif

#include "globalsystemtray.h"
#include "jamiavatartheme.h"
#include "lrcinstance.h"
#include "pixbufmanipulator.h"
#include "version.h"

#include <globalinstances.h>
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
#include <QStackedWidget>
#include <QSvgRenderer>
#include <QTranslator>
#include <QtConcurrent/QtConcurrent>

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
    return true;
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
    return true;
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
    return QString("/usr/local");
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

void
Utils::InvokeMailto(const QString& subject, const QString& body, const QString& attachement)
{
#ifdef Q_OS_WIN
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"mailto", 0, KEY_READ, &hKey);
    if (lRes != ERROR_FILE_NOT_FOUND) {
        auto addr = QString("mailto:?subject=%1&body=%2").arg(subject).arg(body);
        if (not attachement.isEmpty())
            addr += QString("&attachement=%1").arg(attachement);
        ShellExecute(nullptr, L"open", addr.toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
    } else {
        QErrorMessage errorMessage;
        errorMessage.showMessage(QObject::tr("No default mail client found"));
    }
#endif
}

QString
Utils::getContactImageString(const QString& accountId, const QString& uid)
{
    return QString::fromLatin1(
        Utils::QImageToByteArray(
            Utils::conversationPhoto(uid, LRCInstance::getAccountInfo(accountId)))
            .toBase64()
            .data());
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

void
Utils::setStackWidget(QStackedWidget* stack, QWidget* widget)
{
    if (stack->indexOf(widget) != -1 && stack->currentWidget() != widget) {
        stack->setCurrentWidget(widget);
    }
}

void
Utils::showSystemNotification(QWidget* widget,
                              const QString& message,
                              long delay,
                              const QString& triggeredAccountId)
{
    if (!AppSettingsManager::getValue(Settings::Key::EnableNotifications).toBool()) {
        qWarning() << "Notifications are disabled";
        return;
    }
    GlobalSystemTray::instance().setTriggeredAccountId(triggeredAccountId);
    GlobalSystemTray::instance().showMessage(message, "", QIcon(":images/jami.png"));
    QApplication::alert(widget, delay);
}

void
Utils::showSystemNotification(QWidget* widget,
                              const QString& sender,
                              const QString& message,
                              long delay,
                              const QString& triggeredAccountId)
{
    if (!AppSettingsManager::getValue(Settings::Key::EnableNotifications).toBool()) {
        qWarning() << "Notifications are disabled";
        return;
    }
    GlobalSystemTray::instance().setTriggeredAccountId(triggeredAccountId);
    GlobalSystemTray::instance().showMessage(sender, message, QIcon(":images/jami.png"));
    QApplication::alert(widget, delay);
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
    QtConcurrent::run([path] {
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

UtilsAdapter&
UtilsAdapter::instance()
{
    static auto instance = new UtilsAdapter;
    return *instance;
}

QString
Utils::getChangeLog()
{
    QString logs;
    QFile changeLogFile(":/changelog.html");
    if (!changeLogFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << " Change log file failed to load";
        return {};
    }
    QTextStream in(&changeLogFile);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        logs += in.readLine();
    }
    return logs;
}

QString
Utils::getProjectCredits()
{
    QString credits;
    QFile projectCreditsFile(":/projectcredits.html");
    if (!projectCreditsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << " Project Credits failed to load";
        return {};
    }
    QTextStream in(&projectCreditsFile);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        QString currentLine = in.readLine();
        if (credits.isEmpty()) {
            credits += "<h3 align=\"center\" style=\" margin-top:0px; "
                       + QString("margin-bottom:0px; margin-left:0px; margin-right:0px; ")
                       + "-qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">"
                       + UtilsAdapter::tr("Created by:") + "</span></h3>";
        } else if (currentLine.contains("Marianne Forget")) {
            credits
                += "<h3 align=\"center\" style=\" margin-top:0px; margin-bottom:0px; "
                   + QString(
                       "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">")
                   + "<span style=\" font-weight:600;\">" + UtilsAdapter::tr("Artwork by:")
                   + "</span></h3>";
        }
        credits += currentLine;
    }
    credits += "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; "
               + QString(
                   "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">")
               + UtilsAdapter::tr("Based on the SFLPhone project") + "</p>";

    return credits;
}

void
Utils::cleanUpdateFiles()
{
    /*
     * Delete all logs and msi in the %TEMP% directory before launching.
     */
    QString dir = QString(Utils::WinGetEnv("TEMP"));
    QDir log_dir(dir, {"jami*.log"});
    for (const QString& filename : log_dir.entryList()) {
        log_dir.remove(filename);
    }
    QDir msi_dir(dir, {"jami*.msi"});
    for (const QString& filename : msi_dir.entryList()) {
        msi_dir.remove(filename);
    }
    QDir version_dir(dir, {"version"});
    for (const QString& filename : version_dir.entryList()) {
        version_dir.remove(filename);
    }
}

void
Utils::checkForUpdates(bool withUI, QWidget* parent)
{
    Q_UNUSED(withUI)
    Q_UNUSED(parent)
    /*
     * TODO: check update logic.
     */
}

void
Utils::applyUpdates(bool updateToBeta, QWidget* parent)
{
    Q_UNUSED(updateToBeta)
    Q_UNUSED(parent)
    /*
     * TODO: update logic.
     */
}

inline QString
removeEndlines(const QString& str)
{
    QString trimmed(str);
    trimmed.remove(QChar('\n'));
    trimmed.remove(QChar('\r'));
    return trimmed;
}

QString
Utils::bestIdForConversation(const lrc::api::conversation::Info& conv,
                             const lrc::api::ConversationModel& model)
{
    auto contact = model.owner.contactModel->getContact(conv.participants[0]);
    if (!contact.registeredName.isEmpty()) {
        return removeEndlines(contact.registeredName);
    }
    return removeEndlines(contact.profileInfo.uri);
}

QString
Utils::bestIdForAccount(const lrc::api::account::Info& account)
{
    if (!account.registeredName.isEmpty()) {
        return removeEndlines(account.registeredName);
    }
    return removeEndlines(account.profileInfo.uri);
}

QString
Utils::bestNameForAccount(const lrc::api::account::Info& account)
{
    if (account.profileInfo.alias.isEmpty()) {
        return bestIdForAccount(account);
    }
    return account.profileInfo.alias;
}

QString
Utils::bestIdForContact(const lrc::api::contact::Info& contact)
{
    if (!contact.registeredName.isEmpty()) {
        return removeEndlines(contact.registeredName);
    }
    return removeEndlines(contact.profileInfo.uri);
}

QString
Utils::bestNameForContact(const lrc::api::contact::Info& contact)
{
    auto alias = removeEndlines(contact.profileInfo.alias);
    if (alias.length() == 0) {
        return bestIdForContact(contact);
    }
    return alias;
}

QString
Utils::bestNameForConversation(const lrc::api::conversation::Info& conv,
                               const lrc::api::ConversationModel& model)
{
    try {
        auto contact = model.owner.contactModel->getContact(conv.participants[0]);
        auto alias = removeEndlines(contact.profileInfo.alias);
        if (alias.length() == 0) {
            return bestIdForConversation(conv, model);
        }
        return alias;
    } catch (...) {
    }
    return {};
}

/*
 * Returns empty string if only infoHash is available, second best identifier otherwise.
 */
QString
Utils::secondBestNameForAccount(const lrc::api::account::Info& account)
{
    auto alias = removeEndlines(account.profileInfo.alias);
    auto registeredName = removeEndlines(account.registeredName);
    auto infoHash = account.profileInfo.uri;

    if (!alias.length() == 0) {
        /*
         * If alias exists.
         */
        if (!registeredName.length() == 0) {
            /*
             * If registeredName exists.
             */
            return registeredName;
        } else {
            return infoHash;
        }
    } else {
        if (!registeredName.length() == 0) {
            /*
             * If registeredName exists.
             */
            return infoHash;
        } else {
            return "";
        }
    }
}

lrc::api::profile::Type
Utils::profileType(const lrc::api::conversation::Info& conv,
                   const lrc::api::ConversationModel& model)
{
    try {
        auto contact = model.owner.contactModel->getContact(conv.participants[0]);
        return contact.profileInfo.type;
    } catch (...) {
        return lrc::api::profile::Type::INVALID;
    }
}

std::string
Utils::formatTimeString(const std::time_t& timestamp)
{
    std::time_t now = std::time(nullptr);
    char interactionDay[64];
    char nowDay[64];
    std::strftime(interactionDay, sizeof(interactionDay), "%D", std::localtime(&timestamp));
    std::strftime(nowDay, sizeof(nowDay), "%D", std::localtime(&now));
    if (std::string(interactionDay) == std::string(nowDay)) {
        char interactionTime[64];
        std::strftime(interactionTime, sizeof(interactionTime), "%R", std::localtime(&timestamp));
        return interactionTime;
    } else {
        return interactionDay;
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
    const auto contact = model.owner.contactModel->getContact(contactUid);
    return (contact.profileInfo.type == lrc::api::profile::Type::PENDING
            || contact.profileInfo.type == lrc::api::profile::Type::TEMPORARY
            || contact.profileInfo.type == lrc::api::profile::Type::RING
            || contact.profileInfo.type == lrc::api::profile::Type::SIP)
           && !contact.profileInfo.uri.isEmpty();
}

bool
Utils::getReplyMessageBox(QWidget* widget, const QString& title, const QString& text)
{
    if (QMessageBox::question(widget, title, text, QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes)
        return true;
    return false;
}

QImage
Utils::conversationPhoto(const QString& convUid,
                         const lrc::api::account::Info& accountInfo,
                         bool filtered)
{
    auto* convModel = LRCInstance::getCurrentConversationModel();
    const auto convInfo = convModel->getConversationForUID(convUid);
    if (!convInfo.uid.isEmpty()) {
        return GlobalInstances::pixmapManipulator()
            .decorationRole(convInfo, accountInfo)
            .value<QImage>();
    }
    return QImage();
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

/* Generate a QImage representing a dummy user avatar, when user doesn't provide it.
 * Current rendering is a flat colored circle with a centered letter.
 * The color of the letter is computed from the circle color to be visible whaterver be the circle
 * color.
 */
QImage
Utils::fallbackAvatar(const QSize size, const QString& canonicalUriStr, const QString& letterStr)
{
    /*
     * We start with a transparent avatar.
     */
    QImage avatar(size, QImage::Format_ARGB32);
    avatar.fill(Qt::transparent);

    /*
     * We pick a color based on the passed character.
     */
    QColor avColor = getAvatarColor(canonicalUriStr);

    /*
     * We draw a circle with this color.
     */
    QPainter painter(&avatar);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::transparent);
    painter.setBrush(avColor.lighter(110));
    painter.drawEllipse(avatar.rect());

    /*
     * If a letter was passed, then we paint a letter in the circle,
     * otherwise we draw the default avatar icon.
     */
    QString letterStrCleaned(letterStr);
    letterStrCleaned.remove(QRegExp("[\\n\\t\\r]"));
    if (!letterStr.isEmpty()) {
        auto unicode = letterStr.toUcs4().at(0);
        if (unicode >= 0x1F000 && unicode <= 0x1FFFF) {
            /*
             * Is Emoticon.
             */
            auto letter = QString::fromUcs4(&unicode, 1);
            QFont font(QStringLiteral("Segoe UI Emoji"), avatar.height() / 2.66667, QFont::Medium);
            painter.setFont(font);
            QRect emojiRect(avatar.rect());
            emojiRect.moveTop(-6);
            painter.drawText(emojiRect, letter, QTextOption(Qt::AlignCenter));
        } else if (unicode >= 0x0000 && unicode <= 0x00FF) {
            /*
             * Is Basic Latin.
             */
            auto letter = letterStr.at(0).toUpper();
            QFont font("Arial", avatar.height() / 2.66667, QFont::Medium);
            painter.setFont(font);
            painter.setPen(Qt::white);
            painter.drawText(avatar.rect(), QString(letter), QTextOption(Qt::AlignCenter));
        } else {
            auto letter = QString::fromUcs4(&unicode, 1);
            QFont font("Arial", avatar.height() / 2.66667, QFont::Medium);
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

    return avatar;
}

QImage
Utils::fallbackAvatar(const QSize size, const std::string& alias, const std::string& uri)
{
    return fallbackAvatar(size, QString::fromStdString(uri), QString::fromStdString(alias));
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

QImage
Utils::cropImage(const QImage& img)
{
    QRect rect;
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
    auto rcode = QRcode_encodeString(ringID.toStdString().c_str(),
                                     0,            // Let the version be decided by libqrencode
                                     QR_ECLEVEL_L, // Lowest level of error correction
                                     QR_MODE_8,    // 8-bit data mode
                                     1);
    if (not rcode) {
        qWarning() << "Failed to generate QR code: " << strerror(errno);
        return QImage();
    }

    int qrwidth = rcode->width + margin * 2;
    QImage result(QSize(qrwidth, qrwidth), QImage::Format_Mono);
    QPainter painter;
    painter.begin(&result);
    painter.setClipRect(QRect(0, 0, qrwidth, qrwidth));
    painter.setPen(QPen(Qt::black, 0.1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.setBrush(Qt::black);
    painter.fillRect(QRect(0, 0, qrwidth, qrwidth), Qt::white);
    unsigned char* p;
    p = rcode->data;
    for (int y = 0; y < rcode->width; y++) {
        unsigned char* row = (p + (y * rcode->width));
        for (int x = 0; x < rcode->width; x++) {
            if (*(row + x) & 0x1) {
                painter.drawRect(margin + x, margin + y, 1, 1);
            }
        }
    }
    painter.end();
    QRcode_free(rcode);
    return result;
}

float
Utils::getCurrentScalingRatio()
{
    return CURRENT_SCALING_RATIO;
}

void
Utils::setCurrentScalingRatio(float ratio)
{
    CURRENT_SCALING_RATIO = ratio;
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
        qDebug() << "can't open file";
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

QImage
Utils::accountPhoto(const lrc::api::account::Info& accountInfo, const QSize& size)
{
    QImage photo;
    if (!accountInfo.profileInfo.avatar.isEmpty()) {
        QByteArray ba = accountInfo.profileInfo.avatar.toLocal8Bit();
        photo = GlobalInstances::pixmapManipulator().personPhoto(ba, nullptr).value<QImage>();
    } else {
        auto bestId = bestIdForAccount(accountInfo);
        auto bestName = bestNameForAccount(accountInfo);
        QString letterStr = bestId == bestName ? QString() : bestName;
        QString prefix = accountInfo.profileInfo.type == lrc::api::profile::Type::RING ? "ring:"
                                                                                       : "sip:";
        photo = fallbackAvatar(size, prefix + accountInfo.profileInfo.uri, letterStr);
    }
    return scaleAndFrame(photo, size);
}

QString
Utils::humanFileSize(qint64 fileSize)
{
    float fileSizeF = static_cast<float>(fileSize);
    float thresh = 1024;

    if (abs(fileSizeF) < thresh) {
        return QString::number(fileSizeF) + " B";
    }
    QString units[] = {"kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
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

const QString
UtilsAdapter::getBestName(const QString& accountId, const QString& uid)
{
    auto* convModel = LRCInstance::getAccountInfo(accountId).conversationModel.get();
    return Utils::bestNameForConversation(convModel->getConversationForUID(uid), *convModel);
}

const QString
UtilsAdapter::getBestId(const QString& accountId, const QString& uid)
{
    auto* convModel = LRCInstance::getAccountInfo(accountId).conversationModel.get();
    return Utils::bestIdForConversation(convModel->getConversationForUID(uid), *convModel);
}

int
UtilsAdapter::getTotalUnreadMessages()
{
    int totalUnreadMessages {0};
    if (LRCInstance::getCurrentAccountInfo().profileInfo.type != lrc::api::profile::Type::SIP) {
        auto* convModel = LRCInstance::getCurrentConversationModel();
        auto ringConversations = convModel->getFilteredConversations(lrc::api::profile::Type::RING);
        std::for_each(ringConversations.begin(),
                      ringConversations.end(),
                      [&totalUnreadMessages](const auto& conversation) {
                          totalUnreadMessages += conversation.unreadMessages;
                      });
    }
    return totalUnreadMessages;
}

int
UtilsAdapter::getTotalPendingRequest()
{
    auto& accountInfo = LRCInstance::getCurrentAccountInfo();
    return accountInfo.contactModel->pendingRequestCount();
}

void
UtilsAdapter::setConversationFilter(const QString& filter)
{
    LRCInstance::getCurrentConversationModel()->setFilter(filter);
}

void
UtilsAdapter::clearConversationHistory(const QString& accountId, const QString& uid)
{
    LRCInstance::getAccountInfo(accountId).conversationModel->clearHistory(uid);
}

void
UtilsAdapter::removeConversation(const QString& accountId, const QString& uid, bool banContact)
{
    LRCInstance::getAccountInfo(accountId).conversationModel->removeConversation(uid, banContact);
}

const QString
UtilsAdapter::getCurrAccId()
{
    return LRCInstance::getCurrAccId();
}

const QString
UtilsAdapter::getCurrConvId()
{
    return LRCInstance::getCurrentConvUid();
}

void
UtilsAdapter::makePermanentCurrentConv()
{
    LRCInstance::getCurrentConversationModel()->makePermanent(LRCInstance::getCurrentConvUid());
}

const QStringList
UtilsAdapter::getCurrAccList()
{
    return LRCInstance::accountModel().getAccountList();
}

int
UtilsAdapter::getAccountListSize()
{
    return getCurrAccList().size();
}

void
UtilsAdapter::setCurrentCall(const QString& accountId, const QString& convUid)
{
    auto& accInfo = LRCInstance::getAccountInfo(accountId);
    const auto convInfo = accInfo.conversationModel->getConversationForUID(convUid);
    accInfo.callModel->setCurrentCall(convInfo.callId);
}

void
UtilsAdapter::startPreviewing(bool force)
{
    LRCInstance::renderer()->startPreviewing(force);
}

void
UtilsAdapter::stopPreviewing()
{
    if (!LRCInstance::hasVideoCall()) {
        LRCInstance::renderer()->stopPreviewing();
    }
}

bool
UtilsAdapter::hasVideoCall()
{
    return LRCInstance::hasVideoCall();
}

const QString
UtilsAdapter::getCallId(const QString& accountId, const QString& convUid)
{
    auto& accInfo = LRCInstance::getAccountInfo(accountId);
    const auto convInfo = accInfo.conversationModel->getConversationForUID(convUid);

    if (convInfo.uid.isEmpty()) {
        return "";
    }

    auto call = LRCInstance::getCallInfoForConversation(convInfo, false);
    if (!call) {
        return "";
    }

    return call->id;
}

const QString
UtilsAdapter::getCallStatusStr(int statusInt)
{
    const auto status = static_cast<lrc::api::call::Status>(statusInt);
    return lrc::api::call::to_string(status);
}

QString
UtilsAdapter::getStringUTF8(QString string)
{
    return string.toUtf8();
}

QString
UtilsAdapter::getRecordQualityString(int value)
{
    return value ? QString::number(static_cast<float>(value) / 100, 'f', 1) + " Mbps" : "Default";
}

QString
UtilsAdapter::getCurrentPath()
{
    return QDir::currentPath();
}

bool
UtilsAdapter::checkShowPluginsButton()
{
    return LRCInstance::pluginModel().getPluginsEnabled()
           && (LRCInstance::pluginModel().listLoadedPlugins().size() > 0);
}

bool
UtilsAdapter::isImage(const QString& fileExt) const
{
    if (fileExt == "png" || fileExt == "jpg" || fileExt == "jpeg")
        return true;
    return false;
}
