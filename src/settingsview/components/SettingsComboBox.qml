/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
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

import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4

import "../../commoncomponents"
import "../../constant"

RowLayout {
    id: root

    property string labelText: ""
    property var comboModel
    property int fontPointSize: JamiTheme.headerFontSize
    property int heightOfLayout: 30
    property int widthOfComboBox: 50
    property int modelIndex
    property string tipText: ""
    property string role: ""

    signal indexChanged

    function setCurrentIndex(index) {
        comboBoxOfLayout.currentIndex = index
        modelIndex = index
    }

    function setEnabled(status) {
        comboBoxOfLayout.enabled = status
        label.enabled = status
    }

    ElidedTextLabel {
        id: label

        Layout.fillWidth: true
        Layout.preferredHeight: heightOfLayout
        Layout.rightMargin: JamiTheme.preferredMarginSize / 2

        eText: qsTr(labelText)
        fontSize: JamiTheme.settingsFontSize
        maxWidth: widthOfComboBox
    }

    SettingParaCombobox {
        id: comboBoxOfLayout

        Layout.preferredWidth: widthOfComboBox
        Layout.preferredHeight: JamiTheme.preferredFieldHeight

        font.pointSize: JamiTheme.buttonFontSize
        font.kerning: true

        model: comboModel

        textRole: role
        tooltipText: tipText

        onActivated: {
            root.modelIndex = index
            indexChanged()
        }
    }
}
