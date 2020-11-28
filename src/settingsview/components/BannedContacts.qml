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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4
import Qt.labs.platform 1.1
import net.jami.Models 1.0
import net.jami.Adapters 1.0

import "../../commoncomponents"

ColumnLayout {
    id:root

    property bool isSIP

    visible: {
        if (bannedListWidget.model.rowCount() <= 0)
            return false
        return true && !isSIP && SettingsAdapter.getAccountConfig_Manageruri() === ""
    }

    Connections {
        target: MessagesAdapter

        function onContactBanned() {
            bannedListWidget.model.reset()
            root.visible = true
            bannedContactsBtn.visible = true
            bannedListWidget.visible = false
        }
    }

    Connections {
        target: AccountAdapter

        function onContactUnbanned() {
            updateAndShowBannedContactsSlot()
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

    function unban(index) {
        SettingsAdapter.unbanContact(index)
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
                        "qrc:/images/icons/expand_less-24px.svg" :
                        "qrc:/images/icons//expand_more-24px.svg"

            onClicked: toggleBannedContacts()
        }
    }

    ListViewJami {
        id: bannedListWidget

        Layout.fillWidth: true
        Layout.preferredHeight: 160

        visible: false

        model: BannedListModel {}

        delegate: BannedItemDelegate {
            id: bannedListDelegate

            width: bannedListWidget.width
            height: 74

            contactName : ContactName
            contactID: ContactID

            onClicked: bannedListWidget.currentIndex = index

            onBtnReAddContactClicked: unban(index)
        }
    }
}
