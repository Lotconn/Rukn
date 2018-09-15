import QtQuick 2.5
import QtQuick.Controls 1.4
import Qt.labs.controls 1.0
import QtQuick.Layouts 1.2
import Qt.labs.settings 1.0
import Munadi.RequestType 1.0

Rectangle {
    border.color: main.borderColour

    function daysInMonth(month) {
        var today = new Date();
        return new Date(today.getFullYear(), month, 0).getDate();
    }

    ScrollView {
        id: sv
        anchors.fill: parent
        flickableItem.flickableDirection: Flickable.VerticalFlick

        ColumnLayout {
            spacing: 10
            width: sv.width - 30
            x: 10   //left padding didn't work

            RowLayout {} // Top padding

            Component {
                id: ls
                Settings {
                    category: 'Location'
                    property int city_gid
                    property int country_gid
                    property string name
                }
            }

            Loader {
                id: lsl
                sourceComponent: ls
            }

            SettingTitle {
                text: qsTr('Select or add a Mosque to get started')
            }

            RowLayout {
                ComboBox {
                    id: mosquesList
                    Layout.fillWidth: true
                    model: ListModel {}
                    textRole: 'text'
                    enabled: model.count !== 0

                    onCurrentIndexChanged: {
                        tt.sourceComponent = null;
                        console.debug('onCurrentIndexChanged: ', currentIndex);
                        if(model.count === 0) {
                            mosqueEditLoader.sourceComponent =  null
                        } else if(currentIndex === 0) {
                            mosqueEditLoader.sourceComponent =  null
                            mosqueEditLoader.sourceComponent =  mosqueEdit
                        } else if(currentIndex > 0) {
                            mosqueEditLoader.sourceComponent =  null
                            mosqueEditLoader.sourceComponent =  tableEdit
                        }
                    }
                }
                Button {
                    id: btnMosqueEdit
                    text: qsTr('Edit Mosque Details')
                    visible: mosquesList.currentIndex > 0 ? true : false
                    onClicked: {
                        tt.sourceComponent = null;  // Don't edit when table being edited!
                        mosqueEditLoader.sourceComponent =  null;
                        mosqueEditLoader.sourceComponent =  mosqueEdit;
                    }
                }
                Button {
                    id: btnRefreshMosques
                    Layout.preferredWidth: height

                    onClicked: {
                        main.mosqueCountShown = true;
                        mosqueEditLoader.sourceComponent = null;
                        lsl.sourceComponent = null;
                        lsl.sourceComponent = ls;
                        engine.apiRequest(lsl.item.city_gid, RequestType.MosquesList);
                        mosquesList.model.clear();
                        mosquesList.currentIndex = 0;
                        mosquesList.model.append({text: qsTr('Please wait ...')});
                        mosquesList.enabled = false;
                    }

                    Image {
                        source: "qrc:/img/refresh.png"
                        anchors.centerIn: parent
                        sourceSize.height: parent.height / 2
                    }
                }
            }

            Connections {
                target: engine
                onApiReplyData: {
                    if(errorNo) {
                        //main.error(qsTr('Error'), data, errorMsg);

                        if(type == RequestType.TableUpload) {
                            mosqueEditLoader.item.upReqCompleted(false);
                        } else if(type == RequestType.TableDownload) {
                            mosqueEditLoader.item.downReqCompleted(false);
                        }

                        return
                    }

                    if(type === RequestType.MosquesList) {
                        if(errorNo) {
                            //main.error(qsTr('Error'), data, errorMsg);
                        } else {
                            //console.debug('Mosques: ', data);
                            mosquesList.model.clear();
                            mosquesList.model.append({
                                            text: qsTr('Add New Mosque'),
                                            m_id: -1,
                                            name: '',
                                            address: '',
                                            postcode: '',
                                            city_gid: lsl.item.city_gid,
                                            country_gid: lsl.item.country_gid
                                        })

                            try {
                                var json = JSON.parse(data);
                                json.mosques.forEach(function(mosque) {
                                    mosque.text = mosque.name + ', ' + mosque.address + ', ' + mosque.postcode;
                                    mosquesList.model.append(mosque);
                                    //console.debug('Mosque: ', JSON.stringify(mosque));
                                });
                            } catch(e) {
                                console.error(e);
                            }

                            mosquesList.currentIndex = -1;
                            mosquesList.enabled = mosquesList.model.count !== 0;
                        }
                    } else if(type === RequestType.MosqueAdd || type === RequestType.MosqueUpdate) {
                        //main.success(qsTr('Thank you'), data);
                        mosqueEditLoader.sourceComponent = null;
                        btnRefreshMosques.clicked();
                    } else if(type === RequestType.TableUpload) {
                        //main.success(qsTr('Thank you'), data);
                        mosqueEditLoader.item.upReqCompleted(true);
                    } else if(type === RequestType.TableDownload) {
                        tt.item.downloadDone(data);
                        mosqueEditLoader.item.downReqCompleted(true);
                    }
                }
            }

            Component {
                id: mosqueEdit
                ColumnLayout {

                    TextField {
                        id: mosqueName
                        placeholderText: qsTr('Mosque Name')
                        Layout.fillWidth: true
                        text: mosquesList.model.get(mosquesList.currentIndex).name
                    }
                    TextField {
                        id: mosqueAddress
                        placeholderText: qsTr('Mosque Address')
                        Layout.fillWidth: true
                        text: mosquesList.model.get(mosquesList.currentIndex).address
                    }
                    TextField {
                        Layout.fillWidth: true
                        text: lsl.item.name
                        readOnly: true
                    }
                    RowLayout {
                        TextField {
                            id: mosquePostCode
                            placeholderText: qsTr('Mosque Postcode/Zipcode')
                            Layout.fillWidth: true
                            text: mosquesList.model.get(mosquesList.currentIndex).postcode
                        }
                        Button {
                            id: btnAddUpdate
                            text: mosquesList.currentIndex === 0? qsTr('Add') : qsTr('Update')
                            onClicked: {
                                mosquesList.model.get(mosquesList.currentIndex).name = mosqueName.text;
                                mosquesList.model.get(mosquesList.currentIndex).address = mosqueAddress.text;
                                mosquesList.model.get(mosquesList.currentIndex).postcode = mosquePostCode.text;

                                if(mosquesList.model.get(mosquesList.currentIndex).name === ''
                                        || mosquesList.model.get(mosquesList.currentIndex).address === ''
                                        || mosquesList.model.get(mosquesList.currentIndex).postcode === '') {
                                    //main.error(qsTr('Attention'), qsTr('Please fill in all blanks.'));
                                } else {
                                    enabled = false;

                                    var filter = function(k,v) { return (k==='objectName' || k==='text') ? undefined : v };
                                    var msq = JSON.stringify(mosquesList.model.get(mosquesList.currentIndex), filter)
                                    //console.debug('Mosque: ', msq)

                                    if(mosquesList.model.get(mosquesList.currentIndex).m_id === -1) {
                                        engine.apiRequest(msq, RequestType.MosqueAdd);
                                    } else {
                                        engine.apiRequest(msq, RequestType.MosqueUpdate);
                                    }
                                }
                            }
                        }

                        Button {
                            text: qsTr('Cancel');
                            visible: mosquesList.currentIndex > 0;
                            onClicked: {
                                mosqueEditLoader.sourceComponent =  null;
                                mosqueEditLoader.sourceComponent =  tableEdit;
                            }
                        }
                    }
                }
            }

            Loader {
                id: mosqueEditLoader
                sourceComponent: null
                Layout.fillWidth: true
            }

            Component {
                id: tableEdit
                RowLayout {
                    property alias currentIndex: month.currentIndex

                    function downReqCompleted(success) {
                        if(success) {
                            btnTableAddUpdate.state = 'upload'
                        } else {
                            btnTableAddUpdate.state = 'download'
                        }
                    }

                    function upReqCompleted(success) {
                        if(success) {
                            btnTableAddUpdate.state = 'download'
                            tt.sourceComponent = null;
                        } else {
                            btnTableAddUpdate.state = 'upload'
                        }
                    }

                    ComboBox {
                        id: month
                        Component.onCompleted: {
                            var date = new Date();

                            // Add months
                            months.append( { text: qsTr('January') } );
                            months.append( { text: qsTr('February') } );
                            months.append( { text: qsTr('March') } );
                            months.append( { text: qsTr('April') } );
                            months.append( { text: qsTr('May') } );
                            months.append( { text: qsTr('June') } );
                            months.append( { text: qsTr('July') } );
                            months.append( { text: qsTr('August') } );
                            months.append( { text: qsTr('September') } );
                            months.append( { text: qsTr('October') } );
                            months.append( { text: qsTr('November') } );
                            months.append( { text: qsTr('December') } );

                            month.currentIndex = date.getMonth();
                        }

                        model: ListModel {
                            id: months
                        }
                        onCurrentIndexChanged: {
                            tt.sourceComponent = null;
                            btnTableAddUpdate.state = 'download'
                        }
                    }

                    Button {
                        id: btnTableAddUpdate
                        Layout.fillWidth: true
                        state: tt.sourceComponent === null ? 'download' : 'upload'
                        onClicked: {
                            state = 'loading'

                            if(tt.sourceComponent === null) {
                                tt.sourceComponent = null;
                                tt.sourceComponent = table;
                                tt.item.download();
                            } else {
                                tt.item.upload();
                            }
                        }

                        states: [
                            State {
                                name: "download"
                                PropertyChanges {
                                    target: btnTableAddUpdate
                                    text: qsTr('Edit Timetable')
                                }
                            },
                            State {
                                name: "upload"
                                PropertyChanges {
                                    target: btnTableAddUpdate
                                    text: qsTr('Save Timetable')
                                }
                            },
                            State {
                                name: "loading"
                                PropertyChanges {
                                    target: btnTableAddUpdate
                                    text: qsTr('Please wait ...')
                                    enabled: false
                                }
                            }
                        ]
                    }

                    Button {
                        text: qsTr('Cancel');
                        visible: btnTableAddUpdate.state === 'upload';
                        onClicked: {
                            tt.sourceComponent = null;
                            btnTableAddUpdate.state = 'download';
                        }
                    }
                }
            }

            Loader {
                id: tt
                sourceComponent: null
                Layout.fillWidth: true
            }

            Component {
                id: table
                ColumnLayout {
                    function upload() {
                        engine.clearCache();    // Clear cached tables, so we get fresh modified copy
                        var arr = [];

                        for(var i = 0; i < rt.model.count; i++) {
                            if(rt.model.get(i).fi === '' || rt.model.get(i).di === '' || rt.model.get(i).ai === ''
                                    || rt.model.get(i).mi === '' || rt.model.get(i).ii === '') {
                                //main.error(qsTr('Attention'), qsTr('Please fill in all blanks.'));
                                console.debug('Blank @: ', i);
                                mosqueEditLoader.item.upReqCompleted(false);
                                return;
                            } else {
                                arr.push(rt.model.get(i));
                            }
                        }
                        var entries = JSON.stringify(arr, function(k,v) {
                            if(k==="objectName")
                                return;
                            return v;
                        });
                        var timetable = {entries: entries, month: mosqueEditLoader.item.currentIndex + 1
                            , mosque_id: mosquesList.model.get(mosquesList.currentIndex).m_id}
                        engine.apiRequest(JSON.stringify(timetable), RequestType.TableUpload);
                    }

                    function download() {
                        var request = {month: mosqueEditLoader.item.currentIndex + 1
                            , mosque_id: mosquesList.model.get(mosquesList.currentIndex).m_id}

                        engine.apiRequest(JSON.stringify(request), RequestType.TableDownload);
                    }

                    function downloadDone(data) {
                        var timetable = JSON.parse(data)
                        console.debug('entires: ', timetable.entries);
                        var date = new Date();
                        var days = daysInMonth(mosqueEditLoader.item.currentIndex + 1);
                        var msg = {action: 'appendCurrentTime', model: dayListModel, days: days, entries: timetable.entries};
                        worker.sendMessage(msg);
                    }

                    WorkerScript {
                        id: worker
                        source: "workerscript.js"
                        onMessage: {
                            console.debug(messageObject.reply);
                            mosqueEditLoader.item.downReqCompleted(true);
                        }
                    }

                    Repeater {
                        id: rt
                        property int labelWidth: 0
                        property int entryWidth
                        property variant weekday

                        function getWeekday(month, day) {
                            var weekday = new Array(7);
                            weekday[0]=  "Sunday";
                            weekday[1] = "Monday";
                            weekday[2] = "Tuesday";
                            weekday[3] = "Wednesday";
                            weekday[4] = "Thursday";
                            weekday[5] = "Friday";
                            weekday[6] = "Saturday";

                            var date = new Date((new Date()).getFullYear(), month, day);
                            var retStr = weekday[date.getDay()] + ", " + day;
                            switch(day) {
                            case 1: return retStr + "st";
                            case 2: return retStr + "nd";
                            case 3: return retStr + "rd";
                            default: return retStr + "th"
                            }
                        }

                        model: ListModel {
                            id: dayListModel
                        }

                        RowLayout{
                            id: dayRow
                            property variant val: RegExpValidator { regExp: /^(\+\d\d?|(0[0-9]|0[0-9]|1[0-9]|2[0-3]):[0-5][0-9])$/ }

                            Text {
                                id: dateLabel
                                property bool friday: text.charAt(0) == 'F';
                                font.family: main.defaultFontFamily
                                color: main.defaultFontColour
                                Layout.minimumWidth: rt.labelWidth
                                text: rt.getWeekday(mosqueEditLoader.item.currentIndex, index + 1) + "  "
                                font.bold: friday
                                onWidthChanged: {
                                    rt.labelWidth = Math.max(rt.labelWidth, width);
                                }
                            }

                            TextField { id: fa; Layout.fillWidth: true; text: model.fa; placeholderText: qsTr("Fajr Start")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("fa", text) : fillRest("fa", false) } validator: val }
                            TextField { id: sr; Layout.fillWidth: true; text: model.sr; placeholderText: qsTr("Sunrise")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("sr", text) : fillRest("sr", false) } validator: val }
                            TextField { id: da; Layout.fillWidth: true; text: model.da; placeholderText: qsTr("Duhr Start")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("da", text) : fillRest("da", false) } validator: val }
                            TextField { id: aa; Layout.fillWidth: true; text: model.aa; placeholderText: qsTr("Asr Start")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("aa", text) : fillRest("aa", false) } validator: val }
                            TextField { id: ma; Layout.fillWidth: true; text: model.ma; placeholderText: qsTr("Mgrb Start")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("ma", text) : fillRest("ma", false) } validator: val }
                            TextField { id: ia; Layout.fillWidth: true; text: model.ia; placeholderText: qsTr("Isha Start")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("ia", text) : fillRest("ia", false) } validator: val }

                            Rectangle {
                                width: fa.width / 2
                            }


                            TextField { id: fi; Layout.fillWidth: true; text: model.fi; placeholderText: qsTr("Fajr Iqama")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("fi", text) : fillRest("fi", false) } validator: val }
                            TextField { id: di; Layout.fillWidth: true; text: model.di; placeholderText: dateLabel.friday ? qsTr('Jum\'aa') : qsTr("Duhr Iqama")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("di", text) : fillRest("di", false) } validator: val }
                            TextField { id: ai; Layout.fillWidth: true; text: model.ai; placeholderText: qsTr("Asr Iqama")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("ai", text) : fillRest("ai", false) } validator: val }
                            TextField { id: mi; Layout.fillWidth: true; text: model.mi; placeholderText: qsTr("Mgrb Iqama")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("mi", text) : fillRest("mi", false) } validator: val }
                            TextField { id: ii; Layout.fillWidth: true; text: model.ii; placeholderText: qsTr("Isha Iqama")
                                onAccepted: { text.charAt(0) === '+' ? fillRest("ii", text) : fillRest("ii", false) } validator: val }

                            Binding { target: dayListModel.get(index); property: "fa"; value: fa.text }
                            Binding { target: dayListModel.get(index); property: "sr"; value: sr.text }
                            Binding { target: dayListModel.get(index); property: "da"; value: da.text }
                            Binding { target: dayListModel.get(index); property: "aa"; value: aa.text }
                            Binding { target: dayListModel.get(index); property: "ma"; value: ma.text }
                            Binding { target: dayListModel.get(index); property: "ia"; value: ia.text }

                            Binding { target: dayListModel.get(index); property: "fi"; value: fi.text }
                            Binding { target: dayListModel.get(index); property: "di"; value: di.text }
                            Binding { target: dayListModel.get(index); property: "ai"; value: ai.text }
                            Binding { target: dayListModel.get(index); property: "mi"; value: mi.text }
                            Binding { target: dayListModel.get(index); property: "ii"; value: ii.text }

                            function fillRest(salat, offset) {
                                var date = new Date();
                                var dim = daysInMonth(mosqueEditLoader.item.currentIndex + 1);
                                console.log("Days in month: ", dim);

                                var last = offset ? dim : dim - 1;

                                for(var i = index; i < last; i++) {
                                    var time = new Date();

                                    switch(salat) {

                                    // Below for Athan

                                    case "fa":
                                        if(offset) {
                                            time = engine.getPrayerCalc(0, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).fa = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).fa = dayListModel.get(i).fa;
                                        }
                                        break;
                                    case "sr":
                                        if(offset) {
                                            time = engine.getPrayerCalc(1, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).sr = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).sr = dayListModel.get(i).sr;
                                        }
                                        break;
                                    case "da":
                                        if(offset) {
                                            time = engine.getPrayerCalc(2, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).da = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).da = dayListModel.get(i).da;
                                        }
                                        break;
                                    case "aa":
                                        if(offset) {
                                            time = engine.getPrayerCalc(3, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).aa = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).aa = dayListModel.get(i).aa;
                                        }
                                        break;
                                    case "ma":
                                        if(offset) {
                                            time = engine.getPrayerCalc(4, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).ma = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).ma = dayListModel.get(i).ma;
                                        }
                                        break;
                                    case "ia":
                                        if(offset) {
                                            time = engine.getPrayerCalc(5, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).ia = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).ia = dayListModel.get(i).ia;
                                        }
                                        break;

                                    // Below for Iqama

                                    case "fi":
                                        if(offset) {
                                            time = engine.getPrayerCalc(0, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).fi = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).fi = dayListModel.get(i).fi;
                                        }
                                        break;
                                    case "di":
                                        if(offset) {
                                            time = engine.getPrayerCalc(2, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).di = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).di = dayListModel.get(i).di;
                                        }
                                        break;
                                    case "ai":
                                        if(offset) {
                                            time = engine.getPrayerCalc(3, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).ai = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).ai = dayListModel.get(i).ai;
                                        }
                                        break;
                                    case "mi":
                                        if(offset) {
                                            time = engine.getPrayerCalc(4, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).mi = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).mi = dayListModel.get(i).mi;
                                        }
                                        break;
                                    case "ii":
                                        if(offset) {
                                            time = engine.getPrayerCalc(5, new Date(date.getFullYear(), mosqueEditLoader.item.currentIndex, i + 1));
                                            time.setMinutes(time.getMinutes() + parseInt(offset));
                                            dayListModel.get(i).ii = Qt.formatTime(time, "hh:mm");
                                        } else {
                                            dayListModel.get(i + 1).ii = dayListModel.get(i).ii;
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                }
            }
            RowLayout {} // Bottom padding
        }
    }
}
