import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.4
import Munadi.RequestType 1.0

Rectangle {
    id: searchBox

    signal selectEntry(string location)
    //onSelectEntry: console.debug('sent: ', location)

    Layout.preferredWidth: parent.width
    Layout.fillWidth: true
    Layout.fillHeight: true
    focus: true

    TextField {
        id: stf
        focus: true
//        style: TextFieldStyle {
//            textColor: main.fontColour
//            //font.family: main.defaultFontFmaily
//            font.pixelSize: stf.height / 2
//            font.italic: true

////            background: Rectangle {
////                radius: 0
////                implicitWidth: 100
////                implicitHeight: parent.height
////                border.color: main.borderColour
////                border.width: 1
////            }
//        }
//            Layout.fillWidth: true
        width: parent.width
        height: parent.height
//            Layout.preferredWidth: parent.width
        placeholderText: qsTr('Type your city name')

        Connections {
            target: engine
            onApiReplyData: {
                if(type == RequestType.LocationSearch) {
                    bi.running = false;
                    if(errorNo) {
                        //main.error(qsTr('Error'), data, errorMsg);
                    } else {
                        try {
                            var json = JSON.parse(data)
                            console.debug('q: ', json.q, 'stf.text: ', stf.text, 'json.locations: ', json.locations);

                            if(json.q === stf.text) {
                                lv.model.clear();
                                json.locations.forEach(
                                            function (location) {
                                                lv.model.append(location);
                                            });
                            }
                        } catch(e) {
                            console.error(e);
                        }
                    }
                }
            }
        }

        onTextChanged: {
            bi.running = true;
            engine.apiRequest(text, RequestType.LocationSearch)
        }

        KeyNavigation.tab: lv
        KeyNavigation.down: lv

        Keys.onEscapePressed: {
            lv.model.clear();
        }

        BusyIndicator {
            id: bi
            running: false
            height: parent.height / 2
            width: height
            anchors.rightMargin: height / 2
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    ListView {
        id: lv
        anchors.top: stf.bottom
        width: parent.width
        height: model.count === 0 ? 0 :  (parent.height * 5) + (stf.height / 2)
        delegate: del
        model: ListModel {}
        clip: true

        onWidthChanged: console.debug('listview width: ', width)
        onHeightChanged: console.debug('listview height: ', height)

        Keys.onEscapePressed: {
            lv.model.clear();
            stf.focus = true
        }
        Keys.onReturnPressed: {
            searchBox.selectEntry(JSON.stringify(lv.model.get(currentIndex)));
            lv.model.clear();
            stf.text = '';
            stf.focus = true;
        }
    }

    Component {
        id: del
        Rectangle {
            Layout.fillWidth: true
            width: parent.width
            height: (ListView.view.height - stf.height / 2) / 5
            border.color: theme.borderColour
            focus: true
            color: ma.containsMouse || activeFocus ? 'lightgrey' : 'white'
            visible: true

            Text {
                anchors.centerIn: parent
                Layout.fillWidth: true
                font.pixelSize: parent.height / 2
                //Layout.fillHeight: true
                text: engine.lang() === 'en'? city_name + " - " + country_name : city_altname + " - " + country_altname;
                focus: true
            }
            MouseArea {
                id: ma
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    searchBox.selectEntry(JSON.stringify(lv.model.get(index)));
                    lv.model.clear();
                    stf.text = '';
                    stf.focus = true;
                }
            }
        }
    }
}

