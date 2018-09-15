import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

ComboBox {

    id: favMosquesCB
    textRole: 'name'
    visible: true;// model.count === 0 ? false : true
    font.pixelSize: height / 2

    background: Item {
        implicitWidth: 120
        implicitHeight: 40

        Rectangle {
            width: parent.width
            height: parent.height
            opacity: favMosquesCB.enabled ? 1.0 : 0.2
            //color: favMosquesCB.pressed || favMosquesCB.popup.visible ? "#585A5C" : "#353637"
            border.color: theme.borderColour
        }

        /*Image {
            x: parent.width - width - favMosquesCB.rightPadding
            y: (parent.height - height) / 2
            //source: "qrc:/qt-project.org/imports/Qt/labs/controls/images/drop-indicator.png"
            source: "qrc:/img/star.png"
        }*/

//        Image {
//            source: "qrc:/img/star.png"
//            anchors.verticalCenter: parent.verticalCenter
//            anchors.right: parent.right
//            anchors.rightMargin: height / 2
//            y: (parent.width - width)
//            sourceSize.height: parent.height / 4
//        }
    }

    contentItem: ColumnLayout {

        width: favMosquesCB.width
        height: favMosquesCB.width / 10


        Text {
            anchors.horizontalCenter:  parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: height / 2
            text:  "Mosque A Name"
            font.family: theme.defaultFontFamily
            font.pixelSize: parent.height / 3
            color: theme.fontColour
        }
        Text {
            anchors.horizontalCenter:  parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: height / 2
            text:  "Mosque A Address, Postcode"
            font.family: theme.defaultFontFamily
            font.pixelSize: parent.height / 4
            font.italic: true
            color: "grey"   //main.defaultFontColour
        }


//        Text {
//            id: locOrMsqName
//            text: favMosquesCB.displayText
//            Layout.fillWidth: true
//            font: favMosquesCB.font
//            color: theme.fontColour
//            horizontalAlignment: Text.AlignHCenter
//            verticalAlignment: Text.AlignVCenter
//            elide: Text.ElideRight
//            //rightPadding: 18 + control.spacing
//        }

        /*Text {
            Layout.fillWidth: true
            text: favMosquesCB.model.get(1).m_id
            //model.get(currentIndex).m_id;
            //font: favMosquesCB.font
            font.italic: true
            color: main.fontColour
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            //rightPadding: 18 + control.spacing
        }*/
    }

    model: ListModel {
        ListElement {
            name: "Mosque A Name"
            address: "Mosque A Address"
            postcode: "Mosque A Postcode"
            verified: true
        }
        ListElement {
            name: "Mosque B Name"
            address: "Mosque B Address"
            postcode: "Mosque B Postcode"
            verified: false
        }
    }
    delegate: ItemDelegate {
        id: mosqueOverlay
        //anchors.fill: parent
        //border.color: main.borderColour
        //checkable: false

        focus: true

        width: favMosquesCB.width
        height: favMosquesCB.width / 10


        Text {
            anchors.horizontalCenter:  parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: height / 2
            text:  name //favMosquesCB.editText
            font.family: theme.defaultFontFamily
            font.pixelSize: parent.height / 3
            color: theme.defaultFontColour
        }
        Text {
            anchors.horizontalCenter:  parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: height / 2
            text:  address + ", " + postcode //favMosquesCB.editText
            font.family: theme.defaultFontFamily
            font.pixelSize: parent.height / 4
            font.italic: true
            color: "grey"   //theme.defaultFontColour
        }

        Image {
            visible: favMosquesCB.currentIndex == index
            source: "qrc:/img/star.png"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: height / 2
            y: (parent.width - width)
            sourceSize.height: parent.height / 4
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            x: parent.height / 5
            text:  verified == true ? "Verfied" : "Not verified"
            font.family: theme.defaultFontFamily
            font.pixelSize: parent.height / 4
            font.italic: true
            color: "grey"   //main.defaultFontColour
        }

//                Connections {
//                    target: engine
//                    onApiReplyData: {
//                        //busyIndicator.running = false;
//                    }
//                }
    }

    function downloadTimetable() {
        if( favMosquesCB.currentIndex > -1) {
            try {
                if(model.get(currentIndex).m_id !== undefined) {

                    miscSettings.item.favMosque = JSON.stringify(model.get(currentIndex));
                    miscSettings.item.favMosqueIndex = currentIndex;
                    miscSettings.item.favMosqueId = model.get(currentIndex).m_id;

                    mosqueId = model.get(currentIndex).m_id;

                    var date = new Date();
                    var request = {month: date.getMonth() + 1, mosque_id: model.get(currentIndex).m_id}
                    engine.apiRequest(JSON.stringify(request), RequestType.TableGet);

                    return;
                }
            } catch(e) {
                console.error(e);
            }
        }

        miscSettings.item.favMosque = '';
        miscSettings.item.favMosqueId = -1;
        mosqueId = -1;
    }

    onCurrentIndexChanged: {
        console.debug('InfoPage: ', 'onCurrentIndexChanged: ', currentIndex);
        //updatePrayerTimes(true);
        timer.start();
    }

//    Timer {
//        id: timer
//        interval: 1
//        repeat: true
//        onTriggered: {
//            //busyIndicator.running = true;

//            if(miscSettings.sourceComponent !== null) {
//                stop();
//                parent.downloadTimetable();
//            } else {
//                interval = interval * 2;
//            }
//        }
//    }

    Connections {
        target: engine
        onApiReplyData: {
            if(type === RequestType.TableGet) {
                //console.debug('Mosques: ', data);

                //busyIndicator.running = false;

                if(errorNo) {
                    //main.error(qsTr('Error'), data, errorMsg);
                } else {
                    //updatePrayerTimes();
                    engine.refreshSettingsCache();
                }
            }
        }
    }
}
