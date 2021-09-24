/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

import QtQuick
import QtQuick.Controls

import net.jami.Models 1.1
import net.jami.Constants 1.1

// TODO: these includes should generally be resource uris
import "../../commoncomponents"
import "../../settingsview"

Rectangle {
    id: root

    signal itemSelected(int index)

    Component.onCompleted: {
        listModel.append({ 'type': SettingsView.Account, 'name': JamiStrings.accountSettingsMenuTitle,
                         'iconSource': JamiResources.account_24dp_svg})
        listModel.append({ 'type': SettingsView.General, 'name': JamiStrings.generalSettingsTitle,
                         'iconSource': JamiResources.gear_black_24dp_svg})
        listModel.append({ 'type': SettingsView.Media, 'name': JamiStrings.avSettingsMenuTitle,
                         'iconSource': JamiResources.media_black_24dp_svg})
        listModel.append({ 'type': SettingsView.Plugin, 'name': JamiStrings.pluginSettingsTitle,
                         'iconSource': JamiResources.plugin_settings_black_24dp_svg})
    }

    anchors.fill: parent
    color: JamiTheme.backgroundColor

    ButtonGroup {
        buttons: buttons.children
        onCheckedButtonChanged: itemSelected(checkedButton.menuType)
    }

    Column {
        id: buttons

        spacing: 0
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height

        Repeater {
            id: repeater

            model: ListModel { id: listModel }

            PushButton {
                property int menuType: type

                Component.onCompleted: checked = type === SettingsView.Account

                preferredHeight: 64
                preferredWidth: root.width
                preferredMargin: 24

                buttonText: name
                buttonTextFont.pointSize: JamiTheme.textFontSize + 2
                textHAlign: Text.AlignLeft

                source: iconSource
                imageColor: JamiTheme.textColor
                imageContainerHeight: 40
                imageContainerWidth: 40

                normalColor: root.color
                pressedColor: Qt.lighter(JamiTheme.pressedButtonColor, 1.25)
                checkedColor: JamiTheme.selectedColor
                hoveredColor: JamiTheme.hoverColor

                duration: 0
                checkable: true
                radius: 0
            }
        }
    }
}

