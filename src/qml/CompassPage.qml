import QtQuick 2.5
//import QtSensors 5.3
import QtQuick.Controls 2.0

Page {
    Text {
        id: qiblaLabel
        anchors.top: parent.top

        anchors.margins: 20
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("Qibla Direction")
        font.pixelSize: Font.pixelSize * parent.scale
        font.pointSize: compass.height / 20
        color: theme.fontColour

    }

    Rectangle {
        id: compass
        anchors.margins: 20
        anchors.top: qiblaLabel.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        radius: width + 45.0

        width: Math.min(parent.width, parent.height) / 1.5
        height: width

        border.color: theme.fontColour
        border.width: 4

        Component.onCompleted: console.log(
                                   parent.width + ", " + parent.height)

        Image {
            id: arrow
            anchors.centerIn: parent
            //width: 20
            height: parent.height / 2
            fillMode: Image.PreserveAspectFit
            rotation: engine.ff() === "m" ? 0 : engine.getQibla()

            source: "qrc:/img/arrow.png"

            Behavior on rotation {
                NumberAnimation {
                    duration: 5000
                    easing.type: Easing.OutElastic
                }
            }

            Connections {
                target: engine

                onMinuteElapsed: {
                    var n = engine.getQibla()
                    n = n > -90 && n < 90 ? -n : n
                    arrow.rotation = n
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
        anchors.top: compass.bottom
        //anchors.bottom: hijriDate.top
        anchors.margins: 20
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: qiblaLabel.font.pixelSize
        //text: sensor.hasZ && engine.ff() === "m" ? qsTr("Please wait ...") : qsTr("%1° with respect to North").arg(engine.getQibla().toFixed(0));
        text: qsTr("%1° with respect to North").arg(engine.getQibla(
                                                        ).toFixed(0))
        color: theme.fontColour

    }
}
