import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtMultimedia 5.0

RowLayout {

    MediaPlayer {
        id: athanPreview
        volume: 1.0
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose an audio file")
        folder: shortcuts.home

        onAccepted: {
            var name = fileUrl.toString().split('/').slice(-1)[0]
            mm.audioList.push({
                                  "name": name,
                                  "path": fileDialog.fileUrl
                              })
            mm.audioListIndex = mm.audioList.length - 1
            audioListCb.model = mm.audioList
            audioListCb.currentIndex = mm.audioListIndex
        }
    }

    ComboBox {
        id: audioListCb
        model: mm.audioList
        Layout.fillWidth: true
        textRole: "name"

        Component.onCompleted: {
            currentIndex = mm.audioListIndex
        }

        Component.onDestruction: {
            mm.audioListIndex = currentIndex
        }

        onCurrentIndexChanged: {
            athanPreview.source = mm.audioList[currentIndex]["path"]
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
        // Two comp below can be replaced with source.icon
        // they're here because of Snap issues !!! Remove after core20 is out
        Image {
            mirror: mainPage.mirrored
            source: athanPreview.playbackState === MediaPlayer.PlayingState
                    ? "qrc:/img/stop.png" : "qrc:/img/start.png"
            anchors.margins: parent.height / 3
            anchors.fill: parent
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (athanPreview.playbackState === MediaPlayer.PlayingState) {
                    athanPreview.stop()
                } else {
                    athanPreview.play()
                }
            }
        }
        implicitWidth: height
    }
}
