import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

Page {

    PageBorder {

        Flickable {
            anchors.fill: parent
            focus: true
            clip: true

            Keys.onUpPressed: scrollBar.decrease()
            Keys.onDownPressed: scrollBar.increase()

            ScrollBar.vertical: ScrollBar { id: scrollBar }

            contentWidth: parent.width
            contentHeight: content.height + theme.margin * 2


            ColumnLayout {
                id: content

                spacing: 10
                width: parent.width - 60
                //height: parent.height
                //anchors.fill: parent
                anchors.margins: 10

                anchors.horizontalCenter: parent.horizontalCenter

                RowLayout {} // Top padding

                SettingTitle {
                    text: qsTr('Select a topic')
                }

                ComboBox {
                    id: help
                    //color: theme.defaultFontColour
                    model: helpContent
                    Layout.fillWidth: true
                    textRole: "text"
                }
                ListModel {
                    id: helpContent

                    ListElement {
                        text: qsTr('About Munadi?')
                        answer: qsTr("
Munadi is a simple Athan app, with an aim to bring Athan and Iqama time of any location throughout the world.\n
The project aims to target, by priority order, Linux, Windows, Android, and Raspberry Pi.\n
Stay tuned for updates, by regularly visiting munadi.org.\n")
                    }
                    ListElement {
                        text: qsTr('How do I add my current location?')
                        answer: qsTr('
Click the link at top which says "Click here to change location", then you will see a box where you can type your city name.\n
Hit enter key, or click the search button, a drop down will apear.\n
Select the correct location by tabbing and using the down arrow or by using the mouse.\n
Click outside the box, and you location is now set.')
                    }
                    ListElement {
                        text: qsTr('What if my city is not showing up?')
                        answer: qsTr('
Try different spelling, if not then try a city near by.\n
If that does not help, then please contact us.')
                    }
                    ListElement {
                        text: qsTr('What are the keyboard shorcuts?')
                        answer: qsTr('
Left/Right arrow keys to go to previous/next date.\n
Down arrow key to return to current date.\n
Up arrow to toggle between Hijri and Miladi.')
                    }
                    ListElement {
                        text: qsTr('How do I uninstall Munadi?')
                        answer: qsTr('
GNU/Linux/Unix .sh installer: Run the following command: /home/$USER/.munadi/uninstall.sh\n
Flatpak: Run the following command: flatpak uninstall org.munadi.Munadi\n
Android: Just like other apps: Drag into the uninstall icon.\n
Windows: Just like other apps: Control Panel > Uninstall.\n\n
NOTE: If you have an old Linux Munadi 15.06 or earlier, run the following command: sudo rm -r /opt/munadi')
                    }
                    ListElement {
                        text: qsTr('How can I contact you?')
                        answer: qsTr('Please visit munadi.org/contact')
                    }
                }

                Text {
                    id: helpAnswer
                    color: theme.defaultFontColour
                    text: helpContent.get(help.currentIndex).answer
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    visible: help.currentIndex === 0;

                    Text {
                        text: qsTr("Author")
                        color: theme.fontColour;
                        font.family: theme.defaultFontFamily
                        font.italic: true
                    }
                    Text {
                        text: qsTr("Abou A. Abbad")
                        Layout.fillWidth: true
                        color: theme.fontColour;
                        font.family: theme.defaultFontFamily
                    }
                    Text {
                        text: qsTr("Website")
                        color: theme.fontColour;
                        font.family: theme.defaultFontFamily
                        font.italic: true
                    }
                    Text {
                        property string link: "http://munadi.org/"
                        text: "<a href=\"http://munadi.org/\">munadi.org</a>"
                        Layout.fillWidth: true
                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                    Text {
                        text: qsTr("Version")
                        color: theme.fontColour;
                        font.family: theme.defaultFontFamily
                        font.italic: true
                    }
                    Text {
                        text: engine.getVersionNo()
                        Layout.fillWidth: true
                        color: theme.fontColour;
                        font.family: theme.defaultFontFamily
                    }
                }
            }
        }
    }
}
