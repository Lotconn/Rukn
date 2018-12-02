import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import Qt.labs.settings 1.0

Rectangle {
    id: table
    Layout.column: main.mirror === true ? 1 : 0
    color: main.color
    width: 1; height: 1
    Layout.fillWidth: true
    Layout.fillHeight: true

    property int mosqueId    

    onMosqueIdChanged: {
        //console.debug('mosqueId changed to: ', mosqueId)
        settingsCache.setFavMosqueId(mosqueId);
    }

    Component.onCompleted: {
        updatePrayerTimes();
        updateTimeLeft();
    }

    ColumnLayout {
        id: cl
        anchors.fill: parent
        spacing: main.margin

        Component {
            id: miscSettingsComp
            Settings {
                category: 'Location'
                property string name
                property string favMosque
                property string favMosques
                property int favMosqueIndex
                property int favMosqueId

                Component.onCompleted: {
                    mosqueId = favMosqueId;
                }
            }
        }

        Loader {
            id: miscSettings
            sourceComponent: null
        }

        Connections {
            target: main
            onSettingsChanged: {
                console.debug('onSettingsChanged');
                miscSettings.sourceComponent = null;
                miscSettings.sourceComponent = miscSettingsComp;

                favMosquesCB.model.clear();

                try {
                    var favMosquesArr = JSON.parse(miscSettings.item.favMosques);
                    favMosquesArr.forEach(function(favMosque) { favMosquesCB.model.append(JSON.parse(favMosque)); });
                } catch(e) {
                    console.error(e);
                }

                if(miscSettings.item.favMosqueIndex < favMosquesCB.model.count) {
                    favMosquesCB.currentIndex = miscSettings.item.favMosqueIndex;
                }
                else {
                    favMosquesCB.currentIndex = favMosquesCB.model.count - 1;
                }

                updatePrayerTimes();
                updateTimeLeft();
            }
        }



        Prayer {
            id: sunrise
            prayerName: qsTr("Sunrise")
        }

        Prayer {
            id: fajr
            prayerName: qsTr("Fajr")
        }

        Prayer {
            id: duhr
            function isFri() { return (new Date()).getDay() == 5 }
            prayerName: isFri() ? qsTr("Juma") : qsTr("Duhr");
        }

        Prayer {
            id: asr
            prayerName: qsTr("Asr")
        }

        Prayer {
            id: magrib
            prayerName: qsTr("Mgrb")
        }

        Prayer {
            id: isha
            prayerName: qsTr("Isha")
        }

        Rectangle {
            border.color: main.borderColour
            id: tlc
            Layout.fillWidth: true
            Layout.fillHeight: true
            //radius: height

            Text {
                id: timeLeft;
                anchors.centerIn: parent
                font.pixelSize: parent.height / 3
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                color: main.fontColour
                font.family: main.defaultFontFamily
            }

            Image {
                id: athanStopBtn
                visible: false
                height: timeLeft.height
                anchors.right: parent.right
                anchors.rightMargin: width
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/img/stop.png"
                fillMode: Image.PreserveAspectFit

                Connections {
                    target: engine
                    onAthanStarted: {
                        athanStopBtn.visible = true;
                    }
                    onAthanStopped: {
                        athanStopBtn.visible = false;
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    engine.stopAthan();
                    //engine.playAthan();
                }
            }
        }

        Connections {
            target: engine
            onAthanStarted: {
                console.log("onAthanStarted");
                updateTimeLeft();
            }
            onAthanStopped: {
                console.log("onAthanStopped");
                updateTimeLeft();
            }
            onMinuteElapsed: {
                console.log("onUpdateTimeLeft")
                updatePrayerTimes();
                updateTimeLeft();
            }
        }
    }

    function updatePrayerTimes(clear) {

        //console.debug('updatePrayerTimes: clear=', clear)

        updateTimeLeft();

        sunrise.iqamaTime = '';

        if(clear) {

            //console.debug('CLEAR CLEAR CLEAR CLEAR CLEAR CLEAR CLEAR CLEAR ')

            fajr.iqamaTime = ''
            duhr.iqamaTime = ''
            asr.iqamaTime = ''
            magrib.iqamaTime = ''
            isha.iqamaTime = ''

            return;
        }

        if(favMosquesCB.currentIndex > -1) {

            //console.debug('favMosquesCB.currentIndex > -1')

            fajr.athanTime      = Qt.formatTime(engine.getPrayerEntry(0, "fa", mosqueId), "h:mm");
            sunrise.athanTime   = Qt.formatTime(engine.getPrayerEntry(1, "sr", mosqueId), "h:mm");
            duhr.athanTime      = Qt.formatTime(engine.getPrayerEntry(2, "da", mosqueId), "h:mm");
            asr.athanTime       = Qt.formatTime(engine.getPrayerEntry(3, "aa", mosqueId), "h:mm");
            magrib.athanTime    = Qt.formatTime(engine.getPrayerEntry(4, "ma", mosqueId), "h:mm");
            isha.athanTime      = Qt.formatTime(engine.getPrayerEntry(5, "ia", mosqueId), "h:mm");

            fajr.iqamaTime      = Qt.formatTime(engine.getPrayerEntry(0, "fi", mosqueId), "h:mm");
            duhr.iqamaTime      = Qt.formatTime(engine.getPrayerEntry(2, "di", mosqueId), "h:mm");
            asr.iqamaTime       = Qt.formatTime(engine.getPrayerEntry(3, "ai", mosqueId), "h:mm");
            magrib.iqamaTime    = Qt.formatTime(engine.getPrayerEntry(4 ,"mi", mosqueId), "h:mm");
            isha.iqamaTime      = Qt.formatTime(engine.getPrayerEntry(5 ,"ii", mosqueId), "h:mm");

        } else {

            //console.debug('favMosquesCB.currentIndex > -1 NOT')

            fajr.athanTime      = Qt.formatTime(engine.getPrayerCalc(0), "h:mm");
            sunrise.athanTime   = Qt.formatTime(engine.getPrayerCalc(1), "h:mm");
            duhr.athanTime      = Qt.formatTime(engine.getPrayerCalc(2), "h:mm");
            asr.athanTime       = Qt.formatTime(engine.getPrayerCalc(3), "h:mm");
            magrib.athanTime    = Qt.formatTime(engine.getPrayerCalc(4), "h:mm");
            isha.athanTime      = Qt.formatTime(engine.getPrayerCalc(5), "h:mm");
        }

        updateTimeLeft();
    }

    function updateTimeLeft() {

        //console.log("updateTimeLeft")

        if(engine.isAthanPlaying()) {
            timeLeft.text = qsTr("%1 Athan").arg(engine.getCurrPrayerLabel());
        }
        else {
            var prayerLabel = engine.getNextPrayerLabel();

            if(prayerLabel === "") {
                tlc.visible = false;
            } else {
                tlc.visible = true;
                timeLeft.text = qsTr(prayerLabel) + qsTr(" in ") + engine.getTimeDifference();
            }
        }
    }
}
