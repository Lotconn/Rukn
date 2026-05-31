import QtQuick

Item {

    property string borderColour: "darkgrey"  //"#EDEDED"
    property string fontColour: "#545454"
    // primary and secondary accents from icon
    property string accentLightColour: "#FFB366"  // warm light orange, dome highlight
    property string accentDarkColour: "#CC7A33" // muted burnt orange, dome shading
    property string alterLightColour: "#B3D9F5" // soft sky blue, from center/upper walls
    property string alterDarkColour: "#4A7C9B"  // muted steel blue, for shadows/arches

    property double uiPortraitRatio: 9 / 16
    property double uiLandscapeRatio: 1 / uiPortraitRatio

    property int margin: mm.stickerMode ? 5 : 10
}
