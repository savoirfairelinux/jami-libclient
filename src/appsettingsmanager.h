/*!
 * Copyright (C) 2020-2021 by Savoir-faire Linux
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * \file appsettingsmanager.h
 */

#pragma once

#include "utils.h"

#include <QMetaEnum>
#include <QObject>
#include <QString>
#include <QStandardPaths>

const QString defaultDownloadPath = QStandardPaths::writableLocation(
    QStandardPaths::DownloadLocation);

// clang-format off
#define KEYS \
    X(MinimizeOnClose, true) \
    X(DownloadPath, defaultDownloadPath) \
    X(EnableNotifications, true) \
    X(EnableTypingIndicator, true) \
    X(EnableReadReceipt, true) \
    X(AllowFromUntrusted, false) \
    X(AcceptTransferBelow, 20) \
    X(AutoAcceptFiles, true) \
    X(DisplayHyperlinkPreviews, true) \
    X(EnableDarkTheme, false) \
    X(AutoUpdate, true) \
    X(NeverShowMeAgain, false)

/*
 * A class to expose settings keys in both c++ and QML.
 * Note: this using a non-constructable class instead of a
 * namespace allows for QML enum auto-completion in QtCreator.
 * This works well when there is only one enum class. Otherwise,
 * to prevent element name collision when defining multiple enums,
 * use a namespace with:
 *
 *  Q_NAMESPACE
 *  Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
 */
class Settings : public QObject
{
    Q_OBJECT
public:
    enum class Key {
#define X(key, defaultValue) key,
    KEYS
#undef X
        COUNT__
    };
    Q_ENUM(Key)
    static QString toString(Key key)
    {
        return QMetaEnum::fromType<Key>().valueToKey(
                    static_cast<int>(key));
    }
    static QVariant defaultValue(const Key key)
    {
        switch (key) {
#define X(key, defaultValue) \
        case Key::key: return defaultValue;
    KEYS
#undef X
        default: return {};
        }
    }
private:
     Settings() = delete;
};
Q_DECLARE_METATYPE(Settings::Key)
// clang-format on

class AppSettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit AppSettingsManager(QObject* parent = nullptr);
    ~AppSettingsManager() = default;

    Q_INVOKABLE QVariant getValue(const Settings::Key key);
    Q_INVOKABLE void setValue(const Settings::Key key, const QVariant& value);

private:
    QSettings* settings_;
};
