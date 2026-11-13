import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import App

Item {
    Layout.fillHeight: true
    anchors.horizontalCenter: parent.horizontalCenter
    implicitHeight: cl.height
    width: parent.width

    // When used on overlay mode, use white background
    Item {
        anchors.fill: parent
        anchors.margins: -10
    }

    ColumnLayout {
        id: cl

        LayoutMirroring.childrenInherit: true
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        spacing: theme.margin
        width: parent.width

        RowLayout {
            spacing: parent.spacing

            TextField {
                id: tf

                Layout.fillWidth: true
                focus: true
                placeholderText: qsTr("Enter city name")
                selectByMouse: true

                Component.onCompleted: forceActiveFocus()
                onAccepted: LocationEngine.search(text)
            }

            Button {
                text: qsTr("Search")

                onClicked: LocationEngine.search(tf.text)
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            indeterminate: true
            visible: LocationEngine.isLoading
        }

        ComboBox {
            id: locationsComboBox

            Layout.fillWidth: true
            enabled: count > 0
            model: LocationEngine.locationResults
            visible: enabled

            onActivated: index => {
                // if (count < 1)
                //     return;

                LocationEngine.locationResultsIndex = index;
            }
            onModelChanged: {
                Qt.callLater(() => {
                    if (enabled && visible) {
                        forceActiveFocus();
                        popup.open();
                    }
                });
            }
            popup.onClosed: {
                if (activeFocus)
                    tf.forceActiveFocus();
            }
        }
    }
}
