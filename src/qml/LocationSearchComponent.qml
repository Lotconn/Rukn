import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

Item {
    id: locationSearch

    width: parent.width// - theme.margin * 2
    anchors.horizontalCenter: parent.horizontalCenter
    Layout.fillHeight: true
    implicitHeight: cl.height

    property bool detailsVisible: false
    property string prevQuery
    property var location

    function search(query) {
        prevQuery = query
        locationsComboBox.model.clear()
        detailsVisible = false
        locationService.search(query)
    }

    // When used on overlay mode, use white background
    Rectangle {
        anchors.fill: parent
        anchors.margins: -10
    }

    LocationService {
        id: locationService

        onSearchResults: {
            locationsComboBox.model.clear()

            for(var i = 0; i < locations.count; i++) {
                locationsComboBox.model.append({ text: locations.get(i).address.text, location: locations.get(i) })
            }
        }
    }

    ColumnLayout {
        id: cl

        spacing: theme.margin

        width: parent.width

        RowLayout {

            spacing: parent.spacing

            TextField {
                id: tf
                Layout.fillWidth: true
                selectByMouse: true
                placeholderText: qsTr("Enter city name")

                onAccepted: {
                    locationSearch.search(text)
                }
            }

            Button {
                text: qsTr("Search")
                onClicked: {
                    locationSearch.search(tf.text)
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
            enabled: model.count > 0
            visible: enabled
            textRole: "text"
            Layout.fillWidth: true

            model: ListModel {}

            onModelChanged: {
                timezonesComboBox.model.clear()
            }

            onCurrentIndexChanged: {

                timezonesComboBox.model.clear()
                detailsVisible = false

                if(model.count < 1) return

                location = model.get(currentIndex).location

                if(location) {

                    var timezones = engine.timeZoneFromCC(location.address.countryCode)

                    if(timezones.length > 1) {
                        timezones.forEach( function(tz) { timezonesComboBox.model.append({text: tz}) })
                        mm.latitude = 0
                    } else {

                        var address = location.address.text
                        var comma = address.indexOf(',')
                        var addresHead = address.substring(0, comma)
                        var addressTail = address.substring(comma + 1).trim()

                        mm.timeZone = timezones[0]
                        mm.cityName = addresHead
                        mm.countryName = addressTail
                        mm.latitude = location.coordinate.latitude
                        mm.longitude = location.coordinate.longitude

                        //locationSearch.detailsVisible = true

                        mm.invalidate()
                    }
                }
            }
        }

        ComboBox {
            id: timezonesComboBox

            enabled: model.count > 0
            visible: enabled
            Layout.fillWidth: true
            textRole: "text"

            model: ListModel {}

            onCurrentIndexChanged: {

                if(model.count < 1) return

                var address = location.address.text
                var comma = address.indexOf(',')
                var addresHead = address.substring(0, comma)
                var addressTail = address.substring(comma + 1).trim()

                mm.timeZone = model.get(currentIndex).text
                mm.cityName = addresHead
                mm.countryName = addressTail
                mm.latitude = location.coordinate.latitude
                mm.longitude = location.coordinate.longitude

                //locationSearch.detailsVisible = true

                mm.invalidate()
            }
        }

        GridLayout {
            visible: locationSearch.detailsVisible
            columns: 2
            Text {
                font.family: theme.defaultFontFamily; color: theme.defaultFontColour
                text: qsTr("Location")
            }
            TextField {
                id: locationTextField

                text: mm.cityName + ", " + mm.stateName + ", " + mm.countryName
                font.family: theme.defaultFontFamily
                Layout.fillWidth: true
            }
            Text {
                font.family: theme.defaultFontFamily; color: theme.defaultFontColour
                text: qsTr("Timezone")
            }
            TextField {
                id: timezoneTextField

                text: mm.timeZone
                font.family: theme.defaultFontFamily
                Layout.fillWidth: true
            }
            Text {
                font.family: theme.defaultFontFamily; color: theme.defaultFontColour
                text: qsTr("Latitude")
            }
            TextField {
                id: latitudeTextField

                text: mm.latitude.toString()
                font.family: theme.defaultFontFamily
            }
            Text {
                font.family: theme.defaultFontFamily; color: theme.defaultFontColour
                text: qsTr("Longitude")
            }
            TextField {
                id: longitudeTextField

                text: mm.longitude.toString()
                font.family: theme.defaultFontFamily
            }
        }
    }
}
