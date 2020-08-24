/*
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

#pragma once

#include "version.h"

#include <string>

#include <QClipboard>
#include <QCryptographicHash>
#include <QDir>
#include <QApplication>
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
#include <QPainterPath>

#ifdef Q_OS_WIN
#include <ciso646>
#include <windows.h>
#undef ERROR
#else
#define LPCWSTR char *
#endif

#include "api/account.h"
#include "api/contact.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"

static const QSize IMAGE_SIZE{128, 128};
static float CURRENT_SCALING_RATIO{1.0};

#ifdef BETA
static constexpr bool isBeta = true;
#else
static constexpr bool isBeta = false;
#endif

namespace Utils {

/*
 * Qml type register.
 */
#define QML_REGISTERSINGLETONTYPE(T, MAJ, MIN) \
    qmlRegisterSingletonType<T>("net.jami.Models", \
                                MAJ, \
                                MIN, \
                                #T, \
                                [](QQmlEngine *e, QJSEngine *se) -> QObject * { \
                                    Q_UNUSED(e); \
                                    Q_UNUSED(se); \
                                    T *obj = new T(); \
                                    return obj; \
                                });
#define QML_REGISTERSINGLETONTYPE_WITH_INSTANCE(T, MAJ, MIN) \
    qmlRegisterSingletonType<T>("net.jami.Models", \
                                MAJ, \
                                MIN, \
                                #T, \
                                [](QQmlEngine *e, QJSEngine *se) -> QObject * { \
                                    Q_UNUSED(e); \
                                    Q_UNUSED(se); \
                                    return &(T::instance()); \
                                });

#define QML_REGISTERSINGLETONTYPE_URL(URL, T, MAJ, MIN) \
    qmlRegisterSingletonType(QUrl(URL), "net.jami.Models", MAJ, MIN, #T);

#define QML_REGISTERTYPE(T, MAJ, MIN) qmlRegisterType<T>("net.jami.Models", MAJ, MIN, #T);

#define QML_REGISTERNAMESPACE(T, NAME, MAJ, MIN) \
    qmlRegisterUncreatableMetaObject(T, "net.jami.Models", MAJ, MIN, NAME, "")

#define QML_REGISTERUNCREATABLE(T, MAJ, MIN) \
    qmlRegisterUncreatableType<T>("net.jami.Models", \
                                  MAJ, \
                                  MIN, \
                                  #T, \
                                  "Don't try to add to a qml definition of " #T);

#define QML_REGISTERUNCREATABLE_IN_NAMESPACE(T, NAMESPACE, MAJ, MIN) \
    qmlRegisterUncreatableType<NAMESPACE::T>("net.jami.Models", \
                                             MAJ, \
                                             MIN, \
                                             #T, \
                                             "Don't try to add to a qml definition of " #T);

/*
 * System.
 */
bool CreateStartupLink(const std::wstring &wstrAppName);
void DeleteStartupLink(const std::wstring &wstrAppName);
bool CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink);
bool CheckStartupLink(const std::wstring &wstrAppName);
void removeOldVersions();
const char *WinGetEnv(const char *name);
QString GetRingtonePath();
QString GenGUID();
QString GetISODate();
void InvokeMailto(const QString &subject,
                  const QString &body,
                  const QString &attachement = QString());
void setStackWidget(QStackedWidget *stack, QWidget *widget);
void showSystemNotification(QWidget *widget,
                            const QString &message,
                            long delay = 5000,
                            const QString &triggeredAccountId = "");
void showSystemNotification(QWidget *widget,
                            const QString &sender,
                            const QString &message,
                            long delay = 5000,
                            const QString &triggeredAccountId = "");
QSize getRealSize(QScreen *screen);
void forceDeleteAsync(const QString &path);
QString getChangeLog();
QString getProjectCredits();
float getCurrentScalingRatio();
void setCurrentScalingRatio(float ratio);

/*
 * Updates.
 */
void cleanUpdateFiles();
void checkForUpdates(bool withUI, QWidget *parent = nullptr);
void applyUpdates(bool updateToBeta, QWidget *parent = nullptr);

/*
 * Names.
 */
QString bestIdForConversation(const lrc::api::conversation::Info &conv,
                              const lrc::api::ConversationModel &model);
QString bestIdForAccount(const lrc::api::account::Info &account);
QString bestNameForAccount(const lrc::api::account::Info &account);
QString bestIdForContact(const lrc::api::contact::Info &contact);
QString bestNameForContact(const lrc::api::contact::Info &contact);
QString bestNameForConversation(const lrc::api::conversation::Info &conv,
                                const lrc::api::ConversationModel &model);
/*
 * Returns empty string if only infoHash is available.
 */
QString secondBestNameForAccount(const lrc::api::account::Info &account);
lrc::api::profile::Type profileType(const lrc::api::conversation::Info &conv,
                                    const lrc::api::ConversationModel &model);

/*
 * Interactions.
 */
std::string formatTimeString(const std::time_t &timestamp);
bool isInteractionGenerated(const lrc::api::interaction::Type &interaction);
bool isContactValid(const QString &contactUid, const lrc::api::ConversationModel &model);
bool getReplyMessageBox(QWidget *widget, const QString &title, const QString &text);

/*
 * Image.
 */
QString getContactImageString(const QString &accountId, const QString &uid);
QImage getCirclePhoto(const QImage original, int sizePhoto);
QImage conversationPhoto(const QString &convUid,
                         const lrc::api::account::Info &accountInfo,
                         bool filtered = false);
QColor getAvatarColor(const QString &canonicalUri);
QImage fallbackAvatar(const QSize size,
                      const QString &canonicalUriStr,
                      const QString &letterStr = QString());
QImage fallbackAvatar(const QSize size, const std::string &alias, const std::string &uri);
QByteArray QImageToByteArray(QImage image);
QByteArray QByteArrayFromFile(const QString &filename);
QPixmap generateTintedPixmap(const QString &filename, QColor color);
QPixmap generateTintedPixmap(const QPixmap &pix, QColor color);
QImage scaleAndFrame(const QImage photo, const QSize &size = IMAGE_SIZE);
QImage accountPhoto(const lrc::api::account::Info &accountInfo, const QSize &size = IMAGE_SIZE);
QImage cropImage(const QImage &img);
QPixmap pixmapFromSvg(const QString &svg_resource, const QSize &size);
QImage setupQRCode(QString ringID, int margin);

/*
 * Rounded corner.
 */
template<typename T>
void
fillRoundRectPath(QPainter &painter,
                  const T &brushType,
                  const QRect &rectToDraw,
                  qreal cornerRadius,
                  int xTransFormOffset = 0,
                  int yTransFormOffset = 0)
{
    QBrush brush(brushType);
    brush.setTransform(QTransform::fromTranslate(rectToDraw.x() + xTransFormOffset,
                                                 rectToDraw.y() + yTransFormOffset));
    QPainterPath painterPath;
    painterPath.addRoundRect(rectToDraw, cornerRadius);
    painter.fillPath(painterPath, brush);
}

/*
 * Time.
 */
QString formattedTime(int seconds);

/*
 * Byte to human readable size.
 */
QString humanFileSize(qint64 fileSize);

/*
 * Device plug or unplug enum.
 */
enum class DevicePlugStatus { Plugged, Unplugged, Unchanged };

class OneShotDisconnectConnection : public QObject
{
    Q_OBJECT

public:
    explicit OneShotDisconnectConnection(const QObject *sender,
                                         const char *signal,
                                         QMetaObject::Connection *connection,
                                         QObject *parent = nullptr)
        : QObject(parent)
    {
        connection_ = connection;
        disconnectConnection_ = new QMetaObject::Connection;
        *disconnectConnection_ = QObject::connect(sender,
                                                  signal,
                                                  this,
                                                  SLOT(slotOneShotDisconnectConnection()));
    }
    ~OneShotDisconnectConnection()
    {
        if (!connection_) {
            delete connection_;
        }
        if (!disconnectConnection_) {
            delete disconnectConnection_;
        }
        if (!this) {
            delete this;
        }
    }

public slots:
    void
    slotOneShotDisconnectConnection()
    {
        if (connection_) {
            QObject::disconnect(*connection_);
            delete connection_;
        }
        if (disconnectConnection_) {
            QObject::disconnect(*disconnectConnection_);
            delete disconnectConnection_;
        }
        delete this;
    }

private:
    QMetaObject::Connection *connection_;
    QMetaObject::Connection *disconnectConnection_;
};

template<typename Func1, typename Func2>
void
oneShotConnect(const typename QtPrivate::FunctionPointer<Func1>::Object *sender,
               Func1 signal,
               Func2 slot)
{
    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(sender, signal, slot);
    QMetaObject::Connection *const disconnectConnection = new QMetaObject::Connection;
    *disconnectConnection = QObject::connect(sender, signal, [connection, disconnectConnection] {
        if (connection) {
            QObject::disconnect(*connection);
            delete connection;
        }
        if (disconnectConnection) {
            QObject::disconnect(*disconnectConnection);
            delete disconnectConnection;
        }
    });
}

template<typename Func1, typename Func2>
void
oneShotConnect(const typename QtPrivate::FunctionPointer<Func1>::Object *sender,
               Func1 signal,
               const typename QtPrivate::FunctionPointer<Func2>::Object *receiver,
               Func2 slot)
{
    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(sender, signal, receiver, slot);
    QMetaObject::Connection *const disconnectConnection = new QMetaObject::Connection;
    *disconnectConnection = QObject::connect(sender, signal, [connection, disconnectConnection] {
        if (connection) {
            QObject::disconnect(*connection);
            delete connection;
        }
        if (disconnectConnection) {
            QObject::disconnect(*disconnectConnection);
            delete disconnectConnection;
        }
    });
}

inline void
oneShotConnect(const QObject *sender, const char *signal, const QObject *receiver, const char *slot)
{
    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = QObject::connect(sender, signal, receiver, slot);
    OneShotDisconnectConnection *disconnectConnection = new OneShotDisconnectConnection(sender,
                                                                                        signal,
                                                                                        connection);
    Q_UNUSED(disconnectConnection)
}

template<class T>
class Blocker
{
    T *blocked;
    bool previous;

public:
    Blocker(T *blocked)
        : blocked(blocked)
        , previous(blocked->blockSignals(true))
    {}
    ~Blocker() { blocked->blockSignals(previous); }
    T *
    operator->()
    {
        return blocked;
    }
};

template<class T>
inline Blocker<T>
whileBlocking(T *blocked)
{
    return Blocker<T>(blocked);
}

template<typename T>
void
setElidedText(T *object,
              const QString &text,
              Qt::TextElideMode mode = Qt::ElideMiddle,
              int padding = 32)
{
    QFontMetrics metrics(object->font());
    QString clippedText = metrics.elidedText(text, mode, object->width() - padding);
    object->setText(clippedText);
}

template<typename E>
constexpr inline
    typename std::enable_if<std::is_enum<E>::value, typename std::underlying_type<E>::type>::type
    toUnderlyingValue(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

template<typename E, typename T>
constexpr inline
    typename std::enable_if<std::is_enum<E>::value && std::is_integral<T>::value, E>::type
    toEnum(T value) noexcept
{
    return static_cast<E>(value);
}
} // namespace Utils

class UtilsAdapter : public QObject
{
    Q_OBJECT
public:
    explicit UtilsAdapter(QObject *parent = nullptr)
        : QObject(parent)
    {
        clipboard_ = QApplication::clipboard();
    }
    ~UtilsAdapter() {}

    ///Singleton
    static UtilsAdapter &instance();

    Q_INVOKABLE const QString
    getChangeLog()
    {
        return Utils::getChangeLog();
    }

    Q_INVOKABLE const QString
    getProjectCredits()
    {
        return Utils::getProjectCredits();
    }

    Q_INVOKABLE const QString
    getVersionStr()
    {
        return QString(VERSION_STRING);
    }

    Q_INVOKABLE void
    setText(QString text)
    {
        clipboard_->setText(text, QClipboard::Clipboard);
    }

    Q_INVOKABLE const QString
    qStringFromFile(const QString &filename)
    {
        return Utils::QByteArrayFromFile(filename);
    }

    Q_INVOKABLE const QString
    getStyleSheet(const QString &name, const QString &source)
    {
        auto simplifiedCSS = source.simplified().replace("'", "\"");
        QString s = QString::fromLatin1("(function() {"
                                        "    var node = document.createElement('style');"
                                        "    node.id = '%1';"
                                        "    node.innerHTML = '%2';"
                                        "    document.head.appendChild(node);"
                                        "})()")
                        .arg(name)
                        .arg(simplifiedCSS);
        return s;
    }

    Q_INVOKABLE const QString
    getCachePath()
    {
        QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
        dataDir.cdUp();
        return dataDir.absolutePath() + "/jami";
    }
    Q_INVOKABLE bool
    createStartupLink()
    {
        return Utils::CreateStartupLink(L"Jami");
    }
    Q_INVOKABLE QString
    GetRingtonePath()
    {
        return Utils::GetRingtonePath();
    }
    Q_INVOKABLE bool
    checkStartupLink()
    {
        return Utils::CheckStartupLink(L"Jami");
    }

    Q_INVOKABLE const QString
    getContactImageString(const QString &accountId, const QString &uid)
    {
        return Utils::getContactImageString(accountId, uid);
    }

    Q_INVOKABLE void removeConversation(const QString &accountId,
                                        const QString &uid,
                                        bool banContact = false);
    Q_INVOKABLE void clearConversationHistory(const QString &accountId, const QString &uid);
    Q_INVOKABLE void setConversationFilter(const QString &filter);
    Q_INVOKABLE int getTotalUnreadMessages();
    Q_INVOKABLE int getTotalPendingRequest();
    Q_INVOKABLE const QString getBestName(const QString &accountId, const QString &uid);
    Q_INVOKABLE const QString getBestId(const QString &accountId, const QString &uid);

    Q_INVOKABLE const QString getCurrAccId();
    Q_INVOKABLE const QStringList getCurrAccList();
    Q_INVOKABLE int getAccountListSize();
    Q_INVOKABLE void setCurrentCall(const QString &accountId, const QString &convUid);
    Q_INVOKABLE void startPreviewing(bool force);
    Q_INVOKABLE void stopPreviewing();
    Q_INVOKABLE bool hasVideoCall();
    Q_INVOKABLE const QString getCallId(const QString &accountId, const QString &convUid);
    Q_INVOKABLE const QString getCallStatusStr(int statusInt);
    Q_INVOKABLE QString getStringUTF8(QString string);
    Q_INVOKABLE bool validateRegNameForm(const QString &regName);
    Q_INVOKABLE QString getRecordQualityString(int value);
    Q_INVOKABLE QString getCurrentPath();
    Q_INVOKABLE QString
    stringSimplifier(QString input)
    {
        return input.simplified();
    }

    Q_INVOKABLE QString
    toNativeSeparators(QString inputDir)
    {
        return QDir::toNativeSeparators(inputDir);
    }

    Q_INVOKABLE QString
    toFileInfoName(QString inputFileName)
    {
        QFileInfo fi(inputFileName);
        return fi.fileName();
    }

    Q_INVOKABLE QString
    toFileAbsolutepath(QString inputFileName)
    {
        QFileInfo fi(inputFileName);
        return fi.absolutePath();
    }

    Q_INVOKABLE QString
    getAbsPath(QString path)
    {
#ifdef Q_OS_WIN
        return path.replace("file:///", "").replace("\n", "").replace("\r", "");
#else
        return path.replace("file:///", "/").replace("\n", "").replace("\r", "");
#endif
    }

    Q_INVOKABLE QString
    getCroppedImageBase64FromFile(QString fileName, int size)
    {
        auto image = Utils::cropImage(QImage(fileName));
        auto croppedImage = image.scaled(size,
                                         size,
                                         Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);
        return QString::fromLatin1(Utils::QImageToByteArray(croppedImage).toBase64().data());
    }

    Q_INVOKABLE bool checkShowPluginsButton();

private:
    QClipboard *clipboard_;
};
Q_DECLARE_METATYPE(UtilsAdapter *)
