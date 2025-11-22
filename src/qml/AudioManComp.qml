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
        source: mm.audioList[audioListCb.currentIndex].path
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose an audio file")
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: ["Audio files (*.mp3 *.ogg *.wav)"]

        onAccepted: {
            var name = fileDialog.file.toString().split('/').slice(-1)[0]
            mm.audioList.push({
                                  "name": name,
                                  "path": fileDialog.file                              })
            audioListCb.model = mm.audioList
            audioListCb.currentIndex = mm.audioList.length - 1
        }
    }

    ComboBox {
        id: audioListCb
        model: mm.audioList
        Layout.fillWidth: true
        textRole: "name"
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
            if (audioListCb.currentIndex > 0) {
                var rmIdx = audioListCb.currentIndex
                mm.audioList.splice(rmIdx , 1)
                audioListCb.model = mm.audioList
                mm.audioList = mm.audioList

                for(var i = 0; i < 6; i++) {
                    mm.prayerConfig[i].athan_idx = 0
                }
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
