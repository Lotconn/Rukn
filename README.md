Munadi
======

Munadi - ***Simple Athan App***.

## Development
Usually, Qt Creator is used for editing and debugging, but once a feature is fully developed, it is then built and tested under Flatpak, to sense how it behaves under the sandbox environment.

There are many other ways one can build Munadi from source, here are some, including the recommended route, Flatpak:

### Flatpak
1 - Configure flathub for your distro, see: <https://flatpak.org/setup/>

2 - Install `git`, `flatpak` and `flatpak-builder`, example:

  - Debian: `sudo apt install git flatpak flatpak-builder`
  - Fedora: `sudo dnf install git flatpak flatpak-builder`

3 - Install KDE's Flatpak Platform and Sdk:

  - `flatpak install org.kde.{Sdk,Platform}//6.10`
  
3.1 - For the statically linked version, install Freedesktop platform and Sdk:

  - `flatpak install org.freedesktop.{Sdk,Platform}//25.08`

> Note: The commands below are executed **_outside_** the checkout "munadi" folder.\
> It is not required but recommended.

4 - _To build Munadi:_

- `flatpak-builder --force-clean --ccache ./build/ ./munadi/pkg/flathub/org.munadi.Munadi.json`

4.1 - _To build Munadi static:_

- `flatpak-builder --force-clean --ccache ./build-static/ ./munadi/pkg/flathub/org.munadi.Munadi-Static.json`

5 - _To run what you've just built:_
  
- `flatpak-builder --run --env=WAYLAND_DISPLAY="$WAYLAND_DISPLAY" ./build/ ./munadi/pkg/flathub/org.munadi.Munadi.json munadi`

  OR
  
- `flatpak-builder --run --env=WAYLAND_DISPLAY="$WAYLAND_DISPLAY" ./build-static/ ./munadi/pkg/flathub/org.munadi.Munadi-Static.json munadi`


### Qt.io
1 - Download and install Qt

> See <https://www.qt.io/download-qt-installer>

2 - Using Qt Creator, open the project file `CMakeLists.txt`

### Snapcraft
1 - Install Snap and Snapcraft:

  - `sudo apt install snap snapcraft`

> Note: The commands below are executed **_within_** the checkout "munadi" folder.\
> It is required last time I checked !

2 - Copy the `.yaml` file to the root folder (next to `CmakeLists.txt`):

  - `cp ./pkg/snap/snapcraft.yaml .`

3 - Invoke Snapcraft:

  - `snapcraft`

4 - I forgot how to run the results post snapping !

