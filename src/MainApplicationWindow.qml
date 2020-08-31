import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Universal 2.12
import QtGraphicalEffects 1.14
import net.jami.Models 1.0

import "mainview"
import "wizardview"
import "commoncomponents"

ApplicationWindow {
    id: mainApplicationWindow

    AccountMigrationDialog{
        id: accountMigrationDialog

        visible: false

        onAccountMigrationFinished:{
            startClientByMainview()
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

        if (!ClientWrapper.utilsAdaptor.getAccountListSize()) {
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
                Qt.quit()
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

        minimumWidth: 400
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

            onWizardViewIsClosed: {
                if (mainViewLoader.source.toString() !== "qrc:/src/mainview/MainView.qml") {
                    Qt.quit()
                }
            }
        }
    }


    Component.onCompleted: {
        if(!startAccountMigration()){
            startClientByMainview()
        }
    }

    overlay.modal: ColorOverlay {
        source: mainApplicationWindow.contentItem
        color: "transparent"


        /*
         * Color animation for overlay when pop up is shown.
         */
        ColorAnimation on color {
            to: Qt.rgba(0, 0, 0, 0.33)
            duration: 500
        }
    }

    Connections {
        target: ClientWrapper.lrcInstance
        onRestoreAppRequested: {
            if (mainViewLoader.item)
                mainViewLoader.item.show()
            else
                wizardView.show()
        }
    }
}
