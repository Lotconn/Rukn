import QtCore
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts

import Munadi

ColumnLayout {
    RowLayout {
        ComboBox {
            id: athanListCBox

            Layout.fillWidth: true
            model: MultimediaEngine.athanList
            textRole: "name"
        }

        Button {
            icon.source: MultimediaEngine.previewPlaying ? "qrc:/qt/qml/Munadi/img/stop.png" : "qrc:/qt/qml/Munadi/img/start.png"
            implicitWidth: height

            onClicked: MultimediaEngine.previewPlaying ? MultimediaEngine.previewStop() : MultimediaEngine.previewPlay(athanListCBox.currentIndex)
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
                MultimediaEngine.removeAthan(athanListCBox.currentIndex);

                athanListCBox.currentIndex = MultimediaEngine.athanList.length - 1;
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
            for (const file of selectedFiles)
                MultimediaEngine.addAthan(file);

            athanListCBox.currentIndex = MultimediaEngine.athanList.length - 1;
        }
    }
}
