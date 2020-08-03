/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.14

Label {
    id: lookupStatusLabel

    property int labelWidth : 30
    property int labelHeight : 30

    Layout.maximumWidth: labelWidth
    Layout.preferredWidth: labelWidth
    Layout.minimumWidth: labelWidth

    Layout.maximumHeight: labelHeight
    Layout.preferredHeight: labelHeight
    Layout.minimumHeight: labelHeight

    property string lookupStatusState: "Blank"

    // whenever look up status state change, the backgroud weill be set to a cooresponding choice
    onLookupStatusStateChanged: {
        switch (lookupStatusState) {
        case "Blank":
            background = Qt.createQmlObject(
                        "import QtQuick 2.14; Rectangle { anchors.fill: parent; color: \"transparent\"; }",
                        lookupStatusLabel)
            break
        case "Invalid":
            background = Qt.createQmlObject("import QtQuick 2.14;
import \"qrc:/src/constant/\";
Rectangle {
anchors.fill: parent;
Image {
anchors.fill: parent;
source: \"image://tintedPixmap/\" + (\"qrc:/images/icons/baseline-error_outline-24px.svg\").replace(\"qrc:/images/icons/\",\"\") + \"+\" + JamiTheme.urgentOrange_;
mipmap: true;
}
}", lookupStatusLabel)
            break
        case "Taken":
            background = Qt.createQmlObject("import QtQuick 2.14;
import \"qrc:/src/constant/\";
Rectangle {
anchors.fill: parent;
Image{
anchors.fill: parent;
source: \"image://tintedPixmap/\" + (\"qrc:/images/icons/baseline-close-24px.svg\").replace(\"qrc:/images/icons/\",\"\") + \"+\" + JamiTheme.red_;
mipmap: true;
}
}", lookupStatusLabel)
            break
        case "Free":
            background = Qt.createQmlObject("import QtQuick 2.14;
import \"qrc:/src/constant/\";
Rectangle {
anchors.fill: parent;
Image {
anchors.fill: parent;
source: \"image://tintedPixmap/\"+ (\"qrc:/images/icons/baseline-done-24px.svg\").replace(\"qrc:/images/icons/\",\"\") + \"+\" + JamiTheme.presenceGreen_;
mipmap: true;
}
}", lookupStatusLabel)
            break
        case "Searching":
            background = Qt.createQmlObject("import QtQuick 2.14;
import \"qrc:/src/constant/\";
Rectangle {
anchors.fill: parent;
AnimatedImage {
anchors.fill: parent
source: \"qrc:/images/jami_rolling_spinner.gif\";

playing: true
paused: false
fillMode: Image.PreserveAspectFit
mipmap: true
}
}", lookupStatusLabel)
            break
        }
    }
}
