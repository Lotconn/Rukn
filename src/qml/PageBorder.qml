import QtQuick 2.0

Item {
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: theme.margin
    anchors.rightMargin: theme.margin
    anchors.topMargin: theme.margin
    anchors.bottomMargin: mm.stickerMode ? theme.margin : 0
}
