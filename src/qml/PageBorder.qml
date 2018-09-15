import QtQuick 2.0

Rectangle {
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    //anchors.horizontalCenter: parent.horizontalCenter

    //width: parent.width - 20

    anchors.leftMargin: theme.margin
    anchors.rightMargin: theme.margin
    anchors.topMargin: theme.margin
    anchors.bottomMargin: mm.stickerMode ? theme.margin : 0

    //border.color: theme.borderColour
}
