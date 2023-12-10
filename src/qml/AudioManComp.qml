import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.platform
import QtMultimedia

RowLayout {

    MediaPlayer {
        id: athanPreview
        audioOutput: AudioOutput {
            volume: 1.0
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose an audio file")
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)

        onAccepted: {
            console.log("fff", fileDialog.file)
            var name = fileDialog.file.toString().split('/').slice(-1)[0]
            mm.audioList.push({
                                  "name": name,
                                  "path": fileDialog.file                              })
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
            fileDialog.open()
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
        icon.source: athanPreview.playbackState === MediaPlayer.PlayingState
                                 ? "qrc:/img/stop.png" : "qrc:/img/start.png"
        onPressed: {
            if (athanPreview.playbackState === MediaPlayer.PlayingState) {
                athanPreview.stop()
            } else {
                athanPreview.play()
            }
        }
        implicitWidth: height
    }
}
