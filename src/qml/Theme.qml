import QtQuick
import QtQuick.Controls.Material

Item {
    property string accentDark: "#B35900" // muted burnt orange
    // primary and secondary accents from icon
    property string accentLight: "#FFB066"  // warm light orange
    property string alterDark: "#5DADE2"  // muted steel blue
    property string alterLight: "#AED6F1" // soft sky blue
    property string borderColour: "darkgrey"  //"#EDEDED"
    property string fontColour: "#545454"
    property int margin: mm.stickerMode ? 5 : 10
    property var materialAccentDark: Material.color(Material.Orange, Material.Shade900)
    property var materialAccentLight: Material.color(Material.Orange, Material.ShadeA200)
    property var materialAlterDark: Material.color(Material.Blue, Material.Shade300)
    property var materialAlterLight: Material.color(Material.Blue, Material.Shade100)
    property double uiLandscapeRatio: 1 / uiPortraitRatio
    property double uiPortraitRatio: 9 / 16
}
