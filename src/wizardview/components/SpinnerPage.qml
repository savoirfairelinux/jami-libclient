/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang <yang.wang@savoirfairelinux.com>
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
 */

import QtQuick 2.14
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.14

import "../../constant"

ColumnLayout {
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 6

    property bool successState: true
    property string progressLabelEditText: "Generating your Jami account"

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 40
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
    Label {
        id: spinnerLabel
        Layout.alignment: Qt.AlignHCenter
        Layout.minimumWidth: 200
        Layout.minimumHeight: 200

        Layout.maximumWidth: 16777215
        Layout.maximumHeight: 16777215

        property string spinnerDisplyState: successState ? "spinnerLabel_Regular" : "spinnerLabel_Failure"
        onSpinnerDisplyStateChanged: {
            switch (spinnerDisplyState) {
            case "spinnerLabel_Regular":
                background = Qt.createQmlObject("import QtQuick 2.14;
AnimatedImage {
source: \"qrc:/images/jami_eclipse_spinner.gif\"

playing: true
paused: false
fillMode: Image.PreserveAspectFit
mipmap: true
}", spinnerLabel)
                break
            case "spinnerLabel_Failure":
                background = Qt.createQmlObject("import QtQuick 2.14;
import \"qrc:/src/constant/\";
Image {
anchors.fill: parent;
source:\"image://tintedPixmap/\" + (\"qrc:/images/icons/baseline-error_outline-24px.svg\").replace(\"qrc:/images/icons/\", \"\") + \"+\" + JamiTheme.urgentOrange_;
mipmap: true;}", spinnerLabel)
                break
            }
        }
    }
    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 40
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
    Label {
        id: progressLabel
        Layout.alignment: Qt.AlignHCenter
        text: successState ? progressLabelEditText : "Error creating account"
        font.pointSize: 11
        font.kerning: true

        property string progressLabelState: successState ? "color_success" : "color_fail"
        onProgressLabelStateChanged: {
            switch (progressLabelState) {
            case "color_success":
                background = Qt.createQmlObject(
                            "import QtQuick 2.14; Rectangle { anchors.fill: parent; color: \"transparent\"; }",
                            progressLabel)
                break
            case "color_fail":
                background = Qt.createQmlObject(
                            "import QtQuick 2.14; Rectangle { anchors.fill: parent; color: \"red\"; }",
                            progressLabel)
                break
            }
        }
    }
    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.minimumHeight: 20
        Layout.maximumHeight: 20
        Layout.preferredHeight: 20
        Layout.fillWidth: true
        Layout.fillHeight: false
    }
}
