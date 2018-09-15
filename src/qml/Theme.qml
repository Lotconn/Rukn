import QtQuick 2.5

Item {
    FontLoader { source: "qrc:/data/Ubuntu.ttf" }

    property string borderColour: "darkgrey"//"#EDEDED"
    property string fontColour: "#545454"
    property string defaultFontColour: "#545454"
    //property int defaultFontSize: 5
    property string defaultFontFamily: "Ubuntu"

    property bool mirror: engine.lang() === "ar"
    property int margin: mm.stickerMode ? 5 : 10
}
