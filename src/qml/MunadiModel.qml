import QtQuick
import QtQuick.Controls.Material

QtObject {
    id: munadiModel

    property bool checkForUpdates: true
    property bool hideOnClose: false
    property double overrideOffsetBy
    property double page_margins_p: 0.02
    property int pane_elevation: 6
    property int rectangle_radius: 3
    property bool stickerMode: false

    function applyTheme(theme) {
        if (theme === "System")
            mainWindow.Material.theme = Material.System;
        else if (theme === "Light")
            mainWindow.Material.theme = Material.Light;
        else if (theme === "Dark")
            mainWindow.Material.theme = Material.Dark;
        else
            console.error("Invalid theme:", theme);
    }
}
