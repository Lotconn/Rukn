import QtQuick
import QtQuick.Controls.Material

Item {
    // primary and secondary accents from icon
    readonly property string accentDark: "#B35900" // muted burnt orange
    readonly property string accentLight: "#FFB066"  // warm light orange
    readonly property string alterDark: "#5DADE2"  // muted steel blue
    readonly property string alterLight: "#AED6F1" // soft sky blue
    readonly property string borderColour: "darkgrey"  //"#EDEDED"
    readonly property string fontColour: "#545454"
    readonly property int margin: 10
    readonly property var materialAccentDark: Material.color(Material.Orange, Material.Shade900)
    readonly property var materialAccentLight: Material.color(Material.Orange, Material.ShadeA200)
    readonly property var materialAlterDark: Material.color(Material.Blue, Material.Shade300)
    readonly property var materialAlterLight: Material.color(Material.Blue, Material.Shade100)
    readonly property double page_margins_p: 0.02
    readonly property int pane_elevation: 6
    readonly property double uiLandscapeRatio: 1 / uiPortraitRatio
    readonly property double uiPortraitRatio: 9 / 16
    readonly property real unit: Math.max(1, Window.height / 432)
}
