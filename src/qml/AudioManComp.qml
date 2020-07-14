import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import Qt.labs.settings 1.1
import QtQuick.Dialogs 1.2

RowLayout {

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose an audio file")
        folder: shortcuts.home

        onAccepted: {
            mm.audioList.push(fileDialog.fileUrl)
            mm.audioListIndex = mm.audioList.length - 1
            audioListCb.model = mm.audioList
            audioListCb.currentIndex = mm.audioListIndex
        }
    }

    ComboBox {
        id: audioListCb
        model: mm.audioList
        Layout.fillWidth: true

        displayText: currentText.split('/').slice(-1)[0]

        Component.onCompleted: {
            currentIndex = mm.audioListIndex
        }

        Component.onDestruction: {
            mm.audioListIndex = currentIndex
        }

        onCurrentIndexChanged: {
            if (currentIndex > 0) {
                athan.source = mm.audioList[currentIndex]
            } else {
                athan.source = "qrc:/data/athan.ogg"
            }
        }

        onActivated: {
            mm.audioListIndex = currentIndex
        }
    }

    Button {
        text: '+'
        implicitWidth: height

        onClicked: {
            fileDialog.visible = true
        }
    }

    Button {
        text: '-'
        implicitWidth: height

        onClicked: {
            if (mm.audioListIndex > 0) {
                mm.audioList.splice(mm.audioListIndex, 1)
                mm.audioListIndex -= 1
                audioListCb.model = mm.audioList
                audioListCb.currentIndex = mm.audioListIndex
            }
        }
    }

    Button {
        icon.source: athan.isOn ? "qrc:/img/stop.png" : "qrc:/img/start.png"
        implicitWidth: height
        onClicked: {
            if (athan.isOn) {
                athan.stop()
            } else {
                athan.play()
            }
        }
    }
}
