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
  
> Note: The commands below are executed **_outside_** the checkout "munadi" folder.\
> It is not required but recommended.

There are currently two Flatpak versions in this repo

  - The default, statically-linked version
  - The older, platform-dependent version, denoted by 'Munadi-runtimes' henceforth

3 - _To build Munadi:_

- `flatpak-builder --install-deps-from=flathub --force-clean --ccache ./build/ ./munadi/pkg/flathub/org.munadi.Munadi.json`

3.1 - _To build Munadi-runtimes:_

- `flatpak-builder --install-deps-from=flathub --force-clean --ccache ./build-runtimes/ ./munadi/pkg/flathub/org.munadi.Munadi-runtimes.json`

4 - _To run what you've just built:_
  
- `flatpak-builder --run --env=WAYLAND_DISPLAY="$WAYLAND_DISPLAY" ./build/ ./munadi/pkg/flathub/org.munadi.Munadi.json munadi`

  OR
  
- `flatpak-builder --run --env=WAYLAND_DISPLAY="$WAYLAND_DISPLAY" ./build-runtimes/ ./munadi/pkg/flathub/org.munadi.Munadi-runtimes.json munadi`

5 - _**Optional:** (Works only on default version) If you care to test the app built using `flatpak-builder` outside the bwrap sandbox as a regular program:_

- `./build/files/bin/munadi`

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


### Bug reporting

- Always start fresh, update runtimes and platforms to the latest versions and then try reproducing the issue. Try to quote steps to reproduce as exactly as possible.
- Run a clean build following a complete removal of `~/.config/munadi.org`, `~/.var/app/org.munadi.Munadi` and `~/.var/app/org.munadi.Munadi-runtimes`
- Run `flatpak --env=QT_DEBUG_PLUGINS=1 --command=munadi org.munadi.Munadi` on a terminal, and try reproducing the problem. Attach the full log when opening a bug report
- If testing the app in this rolling repository, ensure relevant environment variables exists _inside_ the bwrap sandbox. Can usually be achieved by invoking `bash` instead of `munadi` in those `flatpak-builder --run` commands above. Also try testing the app _outside_ the sandbox using step 5 mentioned above.