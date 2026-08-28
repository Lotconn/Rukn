import QtCore
import QtMultimedia
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts

import Munadi

ColumnLayout {
    MediaPlayer {
        id: athanPreview

        source: PrayerEngine.audioList[audioListCb.currentIndex].path

        audioOutput: AudioOutput {
            volume: 1.0
        }
    }

    RowLayout {
        ComboBox {
            id: audioListCb

            Layout.fillWidth: true
            model: PrayerEngine.audioList
            textRole: "name"
        }

        Button {
            icon.source: athanPreview.playbackState === MediaPlayer.PlayingState ? "qrc:/qt/qml/Munadi/img/stop.png" : "qrc:/qt/qml/Munadi/img/start.png"
            implicitWidth: height

            onPressed: {
                if (athanPreview.playbackState === MediaPlayer.PlayingState) {
                    athanPreview.stop();
                } else
                    athanPreview.play();
            }
        }
    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter

        Button {
            Layout.fillWidth: true
            text: qsTr("Add")

            onClicked: fileDialog.open()
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("Remove")

            onClicked: {
                PrayerEngine.removeAthan(audioListCb.currentIndex);

                audioListCb.currentIndex = PrayerEngine.audioList.length - 1;
            }
        }
    }

    FileDialog {
        id: fileDialog

        currentFolder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Audio files (*.aac *.ac3 *.alac *.amr *.flac *.m4a *.mp3 *.ogg *.opus *.wav *.wma *.weba)", "Video files (*.mp4 *.mkv *.webm *.avi *.flv *.mov *.m4v *.ogv *.wmv *.3gp *.3g2)", "All files (*)"]
        title: qsTr("Please choose an audio file")

        onAccepted: {
            for (let i = 0; i < selectedFiles.length; i++)
                PrayerEngine.addAthan(selectedFiles[i], selectedNameFilter.index);

            audioListCb.currentIndex = PrayerEngine.audioList.length - 1;
        }
    }
}
