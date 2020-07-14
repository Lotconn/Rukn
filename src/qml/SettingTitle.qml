import QtQuick 2.5
import QtQuick.Layouts 1.2

Text {
    font.bold: true
    //font.pointSize: 10 //theme.defaultFontSize
    color: theme.fontColour
    //font.family: theme.defaultFontFamily
    Layout.fillWidth: true
    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
}
