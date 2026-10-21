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
        font.pixelSize: Font.pixelSize * parent.scale
        font.pointSize: compass.height / 20
        text: qsTr("Qibla Direction")
    }

    Rectangle {
        id: compass

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        anchors.top: qiblaLabel.bottom
        border.color: theme.fontColour
        border.width: 4
        height: width
        radius: width + 45.0
        width: Math.min(parent.width, parent.height) / 1.5

        Component.onCompleted: console.log(parent.width + ", " + parent.height)

        Image {
            id: arrow

            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            //width: 20
            height: parent.height / 2
            rotation: engine.ff() === "m" ? 0 : engine.getQibla()
            source: "qrc:/qt/qml/Munadi/img/arrow.png"

            Behavior on rotation {
                NumberAnimation {
                    duration: 5000
                    easing.type: Easing.OutElastic
                }
            }

            Connections {
                target: engine

                onMinuteElapsed: {
                    var n = engine.getQibla();
                    n = n > -90 && n < 90 ? -n : n;
                    arrow.rotation = n;
                }
            }
        }
    }

    //        RotationSensor {
    //            id: sensor
    //            active: engine.ff() === "m" ? true : false
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
        font.pixelSize: qiblaLabel.font.pixelSize
        //text: sensor.hasZ && engine.ff() === "m" ? qsTr("Please wait ...") : qsTr("%1° with respect to North").arg(engine.getQibla().toFixed(0));
        text: qsTr("%1° with respect to North").arg(engine.getQibla().toFixed(0))
    }
}
