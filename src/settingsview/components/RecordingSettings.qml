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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import net.jami.Models 1.1
import net.jami.Adapters 1.1
import net.jami.Constants 1.1

import "../../commoncomponents"

ColumnLayout {
    id:root

    property int itemWidth
    property string recordPath: UtilsAdapter.getDirDocument()

    onRecordPathChanged: {
        if(recordPath === "") return

        if(AVModel){
            AVModel.setRecordPath(recordPath)
        }
    }

    FolderDialog {
        id: recordPathDialog

        title: JamiStrings.selectFolder
        currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)

        onAccepted: {
            var dir = UtilsAdapter.getAbsPath(folder.toString())
            recordPath = dir
        }
    }

    Timer{
        id: updateRecordQualityTimer

        interval: 500

        onTriggered: AVModel.setRecordQuality(recordQualitySlider.value * 100)
    }

    ElidedTextLabel {
        Layout.fillWidth: true

        eText: qsTr("Call Recording")
        font.pointSize: JamiTheme.headerFontSize
        maxWidth: width
    }

    ToggleSwitch {
        id: alwaysRecordingCheckBox

        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: AVModel.getAlwaysRecord()

        labelText: qsTr("Always record calls")
        fontPointSize: JamiTheme.settingsFontSize

        onSwitchToggled: AVModel.setAlwaysRecord(checked)
    }

    ToggleSwitch {
        id: recordPreviewCheckBox

        Layout.fillWidth: true
        Layout.leftMargin: JamiTheme.preferredMarginSize

        checked: AVModel.getRecordPreview()

        labelText: JamiStrings.recordCall
        fontPointSize: JamiTheme.settingsFontSize

        onSwitchToggled: AVModel.setRecordPreview(checked)
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        Text {
            Layout.fillWidth: true
            Layout.rightMargin: JamiTheme.preferredMarginSize / 2

            color: JamiTheme.textColor
            text: qsTr("Quality")
            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: recordQualityValueLabel

            Layout.alignment: Qt.AlignRight
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.rightMargin: JamiTheme.preferredMarginSize / 2

            color: JamiTheme.textColor
            text: UtilsAdapter.getRecordQualityString(AVModel.getRecordQuality() / 100)

            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }

        Slider {
            id: recordQualitySlider

            Layout.maximumWidth: itemWidth
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
            Layout.fillHeight: true

            value: AVModel.getRecordQuality() / 100

            from: 0
            to: 500
            stepSize: 1

            onMoved: {
                recordQualityValueLabel.text = UtilsAdapter.getRecordQualityString(value)
                updateRecordQualityTimer.restart()
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: JamiTheme.preferredFieldHeight
        Layout.leftMargin: JamiTheme.preferredMarginSize

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true

            text: qsTr("Save in")
            color: JamiTheme.textColor
            font.pointSize: JamiTheme.settingsFontSize
            font.kerning: true

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        MaterialButton {
            id: recordPathButton

            Layout.alignment: Qt.AlignRight

            preferredWidth: itemWidth
            preferredHeight: JamiTheme.preferredFieldHeight

            toolTipText: JamiStrings.tipRecordFolder
            text: recordPath
            iconSource: JamiResources.round_folder_24dp_svg
            color: JamiTheme.buttonTintedGrey
            hoveredColor: JamiTheme.buttonTintedGreyHovered
            pressedColor: JamiTheme.buttonTintedGreyPressed

            onClicked: recordPathDialog.open()
        }
    }
}
