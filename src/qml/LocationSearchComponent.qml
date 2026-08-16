import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Item {
    id: locationSearch

    property var location
    property string prevQuery

    function search(query) {
        prevQuery = query;
        locationsComboBox.model.clear();
        locationService.search(query);
    }

    Layout.fillHeight: true
    anchors.horizontalCenter: parent.horizontalCenter
    implicitHeight: cl.height
    width: parent.width

    // When used on overlay mode, use white background
    Item {
        anchors.fill: parent
        anchors.margins: -10
    }

    LocationService {
        id: locationService

        onSearchResults: locations => {
            locationsComboBox.model.clear();

            for (var i = 0; i < locations.count; i++) {
                locationsComboBox.model.append({
                    "text": locations.get(i).address.text,
                    "location": locations.get(i)
                });
            }
        }
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
                onAccepted: {
                    locationSearch.search(text);
                }
            }

            Button {
                text: qsTr("Search")

                onClicked: {
                    locationSearch.search(tf.text);
                }
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            indeterminate: true
            visible: locationService.loading
        }

        ComboBox {
            id: locationsComboBox

            Layout.fillWidth: true
            enabled: model.count > 0
            textRole: "text"
            visible: enabled

            model: ListModel {}

            onCurrentIndexChanged: {
                if (model.count < 1)
                    return;
                location = model.get(currentIndex).location;
                var address = location.address.text.split(',').map(s => s.trim());

                mm.cityName = address.length > 0 ? address[0] : "";
                mm.countryName = address.length > 1 ? address[address.length - 1] : "";
                mm.latitude = location.coordinate.latitude;
                mm.longitude = location.coordinate.longitude;
                mm.invalidate();
            }
            onEnabledChanged: {
                Qt.callLater(() => {
                    if (enabled && visible) {
                        forceActiveFocus();
                        popup.open();
                    }
                });
            }
            onModelChanged: {
                timezonesComboBox.model.clear();
            }
            popup.onClosed: {
                if (activeFocus)
                    tf.forceActiveFocus();
            }
        }
    }
}
