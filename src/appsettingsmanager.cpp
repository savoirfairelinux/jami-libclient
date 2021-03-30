/*!
 * Copyright (C) 2021 by Savoir-faire Linux
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
 * \file appsettingsmanager.cpp
 */

#include "appsettingsmanager.h"

AppSettingsManager::AppSettingsManager(QObject* parent)
    : QObject(parent)
    , settings_(new QSettings("jami.net", "Jami", this))
{
    for (int i = 0; i < static_cast<int>(Settings::Key::COUNT__); ++i) {
        auto key = static_cast<Settings::Key>(i);
        if (!settings_->contains(Settings::toString(key)))
            setValue(key, Settings::defaultValue(key));
    }
}

QVariant
AppSettingsManager::getValue(const Settings::Key key)
{
    auto value = settings_->value(Settings::toString(key), Settings::defaultValue(key));

    if (QString(value.typeName()) == "QString"
        && (value.toString() == "false" || value.toString() == "true"))
        return value.toBool();

    return value;
}

void
AppSettingsManager::setValue(const Settings::Key key, const QVariant& value)
{
    settings_->setValue(Settings::toString(key), value);
}
