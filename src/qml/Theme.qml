import QtQuick
import QtQuick.Controls.Material

Item {

    property string borderColour: "darkgrey"  //"#EDEDED"
    property string fontColour: "#545454"
    // primary and secondary accents from icon
    property string accentLight: "#FFB066"  // warm light orange
    property var materialAccentLight: Material.color(Material.Orange, Material.ShadeA200)

    property string accentDark: "#B35900" // muted burnt orange
    property var materialAccentDark: Material.color(Material.Orange, Material.Shade900)

    property string alterLight: "#AED6F1" // soft sky blue
    property var materialAlterLight: Material.color(Material.Blue, Material.Shade100)

    property string alterDark: "#5DADE2"  // muted steel blue
    property var materialAlterDark: Material.color(Material.Blue, Material.Shade300)

    property double uiPortraitRatio: 9 / 16
    property double uiLandscapeRatio: 1 / uiPortraitRatio

    property int margin: mm.stickerMode ? 5 : 10
}
