/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

import QtQuick 2.15
import QtQuick.Layouts 1.15

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id: root

    property int itemWidth

    function updateAccountInfo() {
        usernameSIP.textField = SettingsAdapter.getAccountConfig_Username()
        hostnameSIP.textField = SettingsAdapter.getAccountConfig_Hostname()
        passSIPlineEdit.textField = SettingsAdapter.getAccountConfig_Password()
        proxySIP.textField = SettingsAdapter.getAccountConfig_RouteSet()
    }

    SettingsMaterialLineEdit {
        id: usernameSIP

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        titleField: JamiStrings.username
        itemWidth: root.itemWidth
        onEditFinished: SettingsAdapter.setAccountConfig_Username(textField)
    }

    SettingsMaterialLineEdit {
        id: hostnameSIP

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        titleField: JamiStrings.server
        itemWidth: root.itemWidth
        onEditFinished: SettingsAdapter.setAccountConfig_Hostname(textField)
    }

    SettingsMaterialLineEdit {
        id: proxySIP

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        titleField: JamiStrings.proxy
        itemWidth: root.itemWidth
        onEditFinished: SettingsAdapter.setAccountConfig_RouteSet(textField)
    }

    SettingsMaterialLineEdit {
        id: passSIPlineEdit

        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        titleField: JamiStrings.password
        itemWidth: root.itemWidth
        onEditFinished: SettingsAdapter.setAccountConfig_Password(textField)
        echoMode: TextInput.Password
    }
}
