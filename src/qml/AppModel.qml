import QtQuick
import QtQuick.Controls.Material

QtObject {
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
