import QtCore
import QtMultimedia
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts

import Lotconn.FileMan
import Munadi

ColumnLayout {
    MediaPlayer {
        id: athanPreview

        source: mm.audioList[audioListCb.currentIndex].path

        audioOutput: AudioOutput {
            volume: 1.0
        }
    }

    FileDialog {
        id: fileDialog

        currentFolder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Audio files (*.aac *.ac3 *.alac *.amr *.flac *.m4a *.mp3 *.ogg *.opus *.wav *.wma *.weba)", "Video files (*.mp4 *.mkv *.webm *.avi *.flv *.mov *.m4v *.ogv *.wmv *.3gp *.3g2)", "All files (*)"]
        title: qsTr("Please choose an audio file")

        onAccepted: {
            const category = selectedNameFilter.index == 0 ? FileMan.Categories.Audio : selectedNameFilter.index == 1 ? FileMan.Categories.Video : (FileMan.Categories.Audio | FileMan.Categories.Video);
            for (let i = 0; i < selectedFiles.length; i++) {
                const fileUrl = new URL(selectedFiles[i]);
                const result = FileMan.addFile(fileUrl, category);

                if (result.status !== 0 && result.status !== 17) { // errno 17 = EEXIST
                    console.error("Fileman::addFile() failed with error:", result.status);
                    continue;
                }

                const fileName = FileMan.getFileName(fileUrl);
                const savedPath = result.location;

                mm.audioList.push({
                    "name": fileName,
                    "path": savedPath
                });
                audioListCb.model = mm.audioList;
                audioListCb.currentIndex = mm.audioList.length - 1;
            }
        }
    }

    RowLayout {
        ComboBox {
            id: audioListCb

            Layout.fillWidth: true
            model: mm.audioList
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
                if (audioListCb.currentIndex <= 2)
                    return;
                const rmIdx = audioListCb.currentIndex;
                const result = FileMan.rmFile(new URL(mm.audioList[rmIdx].path));
                if (result.status_l !== 0 && result.status_l !== 2) { // errno 2 is ENOENT
                    console.error("Fileman::rmFile() failed with error:", result.status_l);
                    return;
                }

                let newList = mm.audioList.slice();
                newList.splice(rmIdx, 1);
                mm.audioList = newList;
                audioListCb.model = mm.audioList;

                PrayerEngine.removeAthanIndex(rmIdx);
            }
        }
    }
}
