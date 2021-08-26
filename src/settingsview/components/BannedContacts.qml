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

    property bool isSIP

    visible: {
        if (bannedListWidget.model.rowCount() <= 0)
            return false
        return true && !isSIP && CurrentAccount.managerUri === ""
    }

    Connections {
        target: ContactAdapter

        function onBannedStatusChanged(uri, banned) {
            if (banned) {
                bannedListWidget.model.reset()
                root.visible = true
                bannedContactsBtn.visible = true
                bannedListWidget.visible = false
            } else {
                updateAndShowBannedContactsSlot()
            }
        }
    }

    function toggleBannedContacts() {
        var bannedContactsVisible = bannedListWidget.visible
        bannedListWidget.visible = !bannedContactsVisible
        updateAndShowBannedContactsSlot()
    }

    function updateAndShowBannedContactsSlot() {
        bannedListWidget.model.reset()
        root.visible = bannedListWidget.model.rowCount() > 0
        if(bannedListWidget.model.rowCount() <= 0) {
            bannedListWidget.visible = false
            bannedContactsBtn.visible = false
        } else {
            bannedContactsBtn.visible = true
        }
    }

    RowLayout {
        id: bannedContactsBtn

        Layout.fillWidth: true

        ElidedTextLabel {
            Layout.fillWidth: true

            eText: qsTr("Banned Contacts")
            fontSize: JamiTheme.headerFontSize
            maxWidth: root.width - JamiTheme.preferredFieldHeight
                        - JamiTheme.preferredMarginSize * 4
        }

        PushButton {
            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: JamiTheme.preferredFieldHeight
            Layout.preferredHeight: JamiTheme.preferredFieldHeight

            toolTipText: JamiStrings.tipBannedContacts
            imageColor: JamiTheme.textColor

            source: bannedListWidget.visible?
                        JamiResources.expand_less_24dp_svg :
                        JamiResources.expand_more_24dp_svg

            onClicked: toggleBannedContacts()
        }
    }

    ListViewJami {
        id: bannedListWidget

        Layout.fillWidth: true
        Layout.preferredHeight: 160

        visible: false

        model: BannedListModel {
            lrcInstance: LRCInstance
        }

        delegate: ContactItemDelegate {
            id: bannedListDelegate

            width: bannedListWidget.width
            height: 74

            contactName: ContactName
            contactID: ContactID

            btnImgSource: JamiResources.round_remove_circle_24dp_svg
            btnToolTip: JamiStrings.reinstateContact

            onClicked: bannedListWidget.currentIndex = index
            onBtnContactClicked: MessagesAdapter.unbanContact(index)
        }
    }
}
