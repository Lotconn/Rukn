import QtCore
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts

import App

ColumnLayout {
    RowLayout {
        ComboBox {
            id: adhanListCBox

            Layout.fillWidth: true
            model: MultimediaEngine.adhanList
            textRole: "name"
        }

        Button {
            icon.source: MultimediaEngine.previewPlaying ? "qrc:/qt/qml/App/img/stop.png" : "qrc:/qt/qml/App/img/start.png"
            implicitWidth: height

            onClicked: MultimediaEngine.previewPlaying ? MultimediaEngine.previewStop() : MultimediaEngine.previewPlay(adhanListCBox.currentIndex)
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
                MultimediaEngine.removeAdhan(adhanListCBox.currentIndex);

                adhanListCBox.currentIndex = MultimediaEngine.adhanList.length - 1;
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
                MultimediaEngine.addAdhan(file);

            adhanListCBox.currentIndex = MultimediaEngine.adhanList.length - 1;
        }
    }
}
