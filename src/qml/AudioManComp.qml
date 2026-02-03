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
        nameFilters: [
            // List of practically available audio files.
            "Audio files (*.aac *.ac3 *.alac *.amr *.flac *.m4a *.mp3 *.ogg *.opus *.wav *.wma *.weba)",
            "All files (*)"
        ]

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
            if (audioListCb.currentIndex > 2) {
                var rmIdx = audioListCb.currentIndex
                mm.audioList.splice(rmIdx , 1)
                audioListCb.model = mm.audioList
                mm.audioList = mm.audioList

                if ( mm.prayerConfig[0].athan_idx == rmIdx) mm.prayerConfig[0].athan_idx = 2;
                if ( mm.prayerConfig[2].athan_idx == rmIdx) mm.prayerConfig[2].athan_idx = 0;
                if ( mm.prayerConfig[3].athan_idx == rmIdx) mm.prayerConfig[3].athan_idx = 0;
                if ( mm.prayerConfig[4].athan_idx == rmIdx) mm.prayerConfig[4].athan_idx = 1;
                if ( mm.prayerConfig[5].athan_idx == rmIdx) mm.prayerConfig[5].athan_idx = 0;
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
