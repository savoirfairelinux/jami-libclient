import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.12
import QtGraphicalEffects 1.14
import net.jami.Models 1.0
import net.jami.Adapters 1.0
import net.jami.Enums 1.0

import "mainview"
import "wizardview"
import "commoncomponents"

ApplicationWindow {
    id: root

    AccountMigrationDialog{
        id: accountMigrationDialog

        visible: false

        onAccountMigrationFinished:{
            startClientByMainview()
        }
    }

    function close() {
        // If we're in the onboarding wizard or 'MinimizeOnClose'
        // is set, then we can quit
        if (!SettingsAdapter.getAppValue(Settings.MinimizeOnClose) ||
            !UtilsAdapter.getAccountListSize()) {
            Qt.quit()
        } else {
            // hide to the systray
            if (mainViewLoader.item)
                mainViewLoader.item.hide()
            else
                wizardView.hide()
        }
    }

    function slotNewAccountAdded() {
        if(mainViewLoader.newAddedAccountIndex !== -1)
            mainViewLoader.item.newAccountAdded(mainViewLoader.newAddedAccountIndex)
    }

    function startAccountMigration(){
        return accountMigrationDialog.startAccountMigrationOfTopStack()
    }

    function startClientByMainview(){
        setX(Screen.width / 2 - width / 2)
        setY(Screen.height / 2 - height / 2)

        if (!UtilsAdapter.getAccountListSize()) {
            wizardView.show()
        } else {
            mainViewLoader.setSource("qrc:/src/mainview/MainView.qml")
        }
    }

    Universal.theme: Universal.Light

    visible: false

    Loader {
        id: mainViewLoader

        property int newAddedAccountIndex: -1

        asynchronous: true
        visible: status == Loader.Ready
        source: ""

        Connections {
            target: mainViewLoader.item

            function onCloseApp() {
                root.close()
            }

            function onNoAccountIsAvailable() {
                mainViewLoader.setSource("")
                wizardViewForApplicationStart.changePageQML(0)
                wizardView.show()
            }
        }
    }

    Window {
        id: wizardView

        title: "Jami"

        minimumWidth: 500
        minimumHeight: 600

        WizardView {
            id: wizardViewForApplicationStart

            anchors.fill: parent

            onNeedToShowMainViewWindow: {
                mainViewLoader.newAddedAccountIndex = accountIndex
                if (mainViewLoader.source.toString() !== "qrc:/src/mainview/MainView.qml") {
                    mainViewLoader.loaded.disconnect(slotNewAccountAdded)
                    mainViewLoader.loaded.connect(slotNewAccountAdded)
                    mainViewLoader.setSource("qrc:/src/mainview/MainView.qml")
                } else {
                    slotNewAccountAdded()
                }
                wizardView.close()
            }

            onWizardViewIsClosed: parent.close()
        }

        // @disable-check M16
        onClosing: {
            if (mainViewLoader.source.toString() !== "qrc:/src/mainview/MainView.qml") {
                root.close()
            }
        }
        // @enable-check M16
    }

    Component.onCompleted: {
        if(!startAccountMigration()){
            startClientByMainview()
        }
    }

    overlay.modal: ColorOverlay {
        source: root.contentItem
        color: "transparent"

        // Color animation for overlay when pop up is shown.
        ColorAnimation on color {
            to: Qt.rgba(0, 0, 0, 0.33)
            duration: 500
        }
    }

    Connections {
        target: LRCInstance

        function restore(window) {
            window.show()
            window.raise();
            window.requestActivate()
        }

        function onRestoreAppRequested() {
            var window = mainViewLoader.item ? mainViewLoader.item : wizardView
            restore(window)
        }

        function onNotificationClicked(forceToTop) {
            var window = mainViewLoader.item ? mainViewLoader.item : wizardView
            // This is a hack to bring the window to the front which is normally done
            // with QWindow::requestActivate but is thwarted for qml windows by the
            // notification being clicked. Native solutions are preferable.
            if (forceToTop && (!window.visible
                    || window.visibility & Qt.WindowMinimized
                    || window.visibility === Qt.WindowNoState)) {
                var tmpFlags = window.flags
                window.hide()
                window.flags = Qt.WindowStaysOnTopHint
                window.flags = tmpFlags
            }
            restore(window)
        }
    }
}
