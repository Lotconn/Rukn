import QtCore
import QtMultimedia
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts

import HasanEsa.FileMan

ColumnLayout {

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
    currentFolder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
    fileMode: FileDialog.OpenFiles
    nameFilters: [
      "Audio files (*.aac *.ac3 *.alac *.amr *.flac *.m4a *.mp3 *.ogg *.opus *.wav *.wma *.weba)",
      "Video files (*.mp4 *.mkv *.webm *.avi *.flv *.mov *.m4v *.ogv *.wmv *.3gp *.3g2)",
      "All files (*)"
    ]

    onAccepted: {
      var category  = selectedNameFilter.index == 0 ? FileMan.Audio
                    : selectedNameFilter.index == 1 ? FileMan.Video
                    : (FileMan.Audio | FileMan.Video)
      for (var i = 0; i < selectedFiles.length; i++) {
        var fileUrl = new URL(selectedFiles[i])
        var result = FileMan.addFile(fileUrl, category)

        if (result.error !== 0 && result.error !== 17) { // errno 17 = EEXIST
          console.error("Fileman::addFile() failed with error:", result.error)
          continue
        }

        var fileName = FileMan.getFileName(fileUrl)
        var savedPath = result.path

        mm.audioList.push({"name": fileName, "path": savedPath})
        audioListCb.model = mm.audioList
        audioListCb.currentIndex = mm.audioList.length - 1
      }
    }
  }

  RowLayout {

    ComboBox {
      id: audioListCb
      model: mm.audioList
      Layout.fillWidth: true
      textRole: "name"
    }

    Button {
      icon.source: athanPreview.playbackState === MediaPlayer.PlayingState
                                            ? "qrc:/qt/qml/Munadi/img/stop.png"
                                            : "qrc:/qt/qml/Munadi/img/start.png"
      onPressed: {
        if (athanPreview.playbackState === MediaPlayer.PlayingState) {
            athanPreview.stop()
        } else athanPreview.play()
      }
      implicitWidth: height
    }
  }

  RowLayout {

      Layout.alignment: Qt.AlignHCenter

      Button {
        text: qsTr("Add")
        Layout.fillWidth: true
        onClicked: fileDialog.open()
      }

      Button {
        text: qsTr("Remove")
        Layout.fillWidth: true
        onClicked: {
          if (audioListCb.currentIndex <= 2) return

            var rmIdx = audioListCb.currentIndex
            var status = FileMan.rmFile(mm.audioList[rmIdx].name,
                                        FileMan.Audio | FileMan.Video)
            if (status !== 0 && status !== 2) { // errno 2 is ENOENT
              console.error("Fileman::rmFile() failed with error:", status)
              return
            }

            var newList = mm.audioList.slice()
            newList.splice(rmIdx, 1)
            mm.audioList = newList
            audioListCb.model = mm.audioList

            var newConfig = JSON.parse(JSON.stringify(mm.prayerConfig))
            if (newConfig[0].athan_idx === rmIdx) newConfig[0].athan_idx = 2;
            if (newConfig[2].athan_idx === rmIdx) newConfig[2].athan_idx = 0;
            if (newConfig[3].athan_idx === rmIdx) newConfig[3].athan_idx = 0;
            if (newConfig[4].athan_idx === rmIdx) newConfig[4].athan_idx = 1;
            if (newConfig[5].athan_idx === rmIdx) newConfig[5].athan_idx = 0;

            for (var i = 0; i < newConfig.length; ++i) {
              if (newConfig[i].athan_idx > rmIdx) newConfig[i].athan_idx--
            }
            mm.prayerConfig = newConfig
        }
    }
  }
}