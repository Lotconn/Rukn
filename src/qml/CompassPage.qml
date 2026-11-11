import QtQuick
//import QtSensors 5.3
import QtQuick.Controls

Page {
    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft

    Text {
        id: qiblaLabel

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        anchors.top: parent.top
        color: theme.fontColour
        font.pointSize: Math.max(1, compass.height / 20)
        text: qsTr("Qibla Direction")
    }

    Rectangle {
        id: compass

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        anchors.top: qiblaLabel.bottom
        border.color: theme.fontColour
        border.width: 4
        implicitHeight: implicitWidth
        implicitWidth: Math.min(parent.width, parent.height) / 1.5
        radius: implicitWidth + 45.0

        // Component.onCompleted: console.log(parent.width + ", " + parent.height)

        Image {
            id: arrow

            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            height: parent.height / 2
            rotation: PrayerEngine.qibla
            source: "qrc:/qt/qml/App/img/arrow.png"

            Behavior on rotation {
                NumberAnimation {
                    duration: 5000
                    easing.type: Easing.OutElastic
                }
            }
        }
    }

    //        RotationSensor {
    //            id: sensor
    //            active: UiEngine.getFormFactor() === "mobile" ? true : false
    //            onReadingChanged: {
    //                settingsCache.refresh();
    //                qiblaInfo.text = "";
    //                var n = engine.getQibla();
    //                var z = hasZ ? reading.z : 0;
    //                //n = n > -90 && n < 90 ? -n : n;
    //                //z = z > -90 && z < 90 ? -z : z;
    //                arrow.rotation = n + z;
    //                //console.debug("Qibla Angle: " + arrow.rotation);
    //                //console.debug("X: " + reading.x + ", Y: " + reading.y + ", Z: " + reading.z);
    //            }
    //        }
    //    }
    Text {
        id: qiblaInfo

        anchors.horizontalCenter: parent.horizontalCenter
        //anchors.bottom: hijriDate.top
        anchors.margins: 20
        anchors.top: compass.bottom
        color: theme.fontColour
        font.pointSize: qiblaLabel.font.pointSize
        //text: sensor.hasZ && UiEngine.getFormFactor() === "mobile" ? qsTr("Please wait ...") : qsTr("%1° with respect to North").arg(engine.getQibla().toFixed(0));
        text: qsTr("%1° with respect to North").arg(PrayerEngine.qibla.toFixed(2))
    }
}
