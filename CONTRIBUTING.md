# AI Contribution Policy

We aim to be transparent about contributions from AI agents, LLMs, and bots. AI-assisted
contributions by humans are welcome. Any other autonomous AI contributions — including merge/pull
requests, issues, and comments — will be rejected. This includes any "Signed-off-by:
`<agent's operator>`" variants, i.e. human-operated agentic submissions. Human contributors may
include an "Assisted by: " footer in any contribution that involved AI or LLM assistance but is not
required.

# Project Structure

```sh
./
├── android/                        # Android-specific build files
├── pkg/                            # Configs and manifests for packaging and tool building
│   ├── android/                      # Android apk
│   ├── docker/                       # Builder images
│   │   ├── static-builder/             # Spec dir for the static-builder image
│   │   └── web-server/ -> ../../web    # Spec dir for the webserver
│   ├── flatpak/                      # Flatpak manifests
│   ├── makeself/                     # Makeself SFX archive
│   │   └── makeself/                   # megastep/makeself submodule
│   └── snap/                         # Snap manifest
├── screenshots/                   # App Screenshots
├── src/                           # App source files
│   ├── cpp/                         # C++ libs, engine, plugins and backends
│   ├── data/                        # Various resources
│   ├── i18n/                        # Translations
│   ├── img/                         # Image resources
│   ├── qml/                         # QML frontend
│   └── <redacted>                   # Various other config files and icons
├── web/                           # Everything related to the homepage website
├── xdg/                           # XDG manifests, desktop files and icons
├── .gitlab-ci.yml                 # CI/CD config
├── .gitmodules                    # List of project submodules
├── .<other_dot_files>             # Various auto-formatting and linting rules and configs
├── CHANGELOG.md                   # Main tracked changes file
├── CMakeLists.txt                 # Project main CMakeLists.txt
├── CONTRIBUTING.md                # Must-read for contributors
├── LICENSE                        # License
└── README.md                      # Read this first!!!!
```

# Building from Source

Clone the repo:

```bash
git clone --recursive --depth 1 https://gitlab.com/munadi/munadi.git ./munadi_munadi \
  && cd ./munadi_munadi
```

This project uses CMake as the build-system with Ninja Multi-Config as the preferred generator. Both
GCC and Clang are supported in source, but notes apply (see
[further](#1-static-build-via-builder-image-recommended) for details). Using a Qt installation under
a custom prefix is preferred over system-installed versions. Refer to
[Qt documentations](https://doc.qt.io/qt-6/get-and-install-qt.html) on how to (easily) install Qt in
your system or use [aqtinstall](https://github.com/miurahr/aqtinstall) if you'd rather prefer
avoiding opening a Qt account.

## Supported Platforms

- Linux (x86_64 | arm64 )
- macOS (x86_64 | arm64 )

> [!Note]
>
> arm64 building is not supported inside the container image (see below for details)

Windows build is broken as of now, and Android building is not verified.

- Qt >= 6.12 (required)
- FFmpeg >= 9.0.1 (recommended)

## Available Build Flags

| Option              | Default   | Available            | Description                                                                                                         |
| ------------------- | --------- | -------------------- | ------------------------------------------------------------------------------------------------------------------- |
| `MUNADI_LINUX_DIST` | `Default` | `Default`, `Flatpak` | Sets build-type and activates quirks accordingly. `Flatpak` is automatically set when flatpak building is detected. |
| `BUILD_SHARED_LIBS` | `OFF`     | `ON`, `OFF`          | Sets build-type of third-party libraries. Recommended to keep at default                                            |

## 1. Static Build via Builder Image (Recommended)

The recommended method is to build a statically-linked app in the `static-builder` container. From
project root:

**Docker:**

```bash
docker run --rm -it \
  --name munadi-builder \
  -v "$(pwd)":/src:z,shared \
  registry.gitlab.com/munadi/munadi/static-builder:latest
```

**Podman:**

```bash
podman run --rm -it \
  --name munadi-builder \
  -v "$(pwd)":/src:z \
  registry.gitlab.com/munadi/munadi/static-builder:latest
```

<details><summary>Further Developer Notes (Click to expand)</summary>

To build the image locally:

**Docker:**

```bash
# From project root
docker build -t munadi-builder pkg/docker/
docker run --rm -it \
  --name munadi-builder \
  -v "$(pwd)":/src:z,shared \
  munadi-builder
```

**Podman:**

```bash
# From project root
podman build -t munadi-builder pkg/docker/
podman run --rm -it \
  --name munadi-builder \
  -v "$(pwd)":/src:z \
  munadi-builder
```

This builds a custom image upon Rocky 10.2 UBI with necessary dependencies needed for building
statically-linked Qt and FFmpeg libraries to-be-installed under `/usr`.

> [!CAUTION]
>
> Keep at least 50GB space free in your `/var/lib/containers` (`/var/tmp` or `$TMPDIR` if using
> podman) before building the image.

</details>

Then from inside the container:

```bash
#rm -rf ./build  # recommended housekeeping
cmake -S . -B build \
  --fresh \
  -G'Ninja Multi-Config'
cmake --build build --parallel --config Release
cd pkg/makeself && ./generate_installer && cd -

# Pro tip: press up-arrow key to find these commands in history ;)
```

> [!Note]
>
> The above command uses Clang by default. Qt and FFmpeg in the image is built with Clang+LLVM, so
> GCC may not work inside the container

A self-extracting archive named `Munadi-<ver>.run` with the built binary will be available under
`pkg/makeself`.

**_For development purposes_**: the built binary remains available under `build/Release`. To test
standalone, execute from the host (outside container):

```bash
./build/Release/munadi
```

Other build types supported: `Debug`, `RelWithDebInfo`

As a healthy practice, always clean `~/.cache/Munadi`, `~/.cache/munadi/Munadi` and
`~/.config/munadi` folders across subsequent builds to avoid unexpected surprises.

## 2. Build as Flatpak

The recommended method is to use `org.flatpak.Builder`:

```bash
# on project root
curl -fsORL https://github.com/flathub/org.munadi.Munadi/raw/refs/heads/master/org.munadi.Munadi.yml

flatpak install -y flathub org.flatpak.Builder

# Add the flathub repo user-wide
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo

# Then build the manifest
flatpak run --command=flathub-build org.flatpak.Builder --install org.munadi.Munadi.yml

# Run the app (provided no previous installation of flatpak munadi is present)
flatpak run org.munadi.Munadi
```

<details><summary>Further Developer Notes (Click to expand)</summary>

There are two manifests already present under `pkg/flatpak`:

- **`org.munadi.Munadi.yml`** — Regular build using platform runtimes, closely resembling actual
  flathub manifest
- **`org.munadi.Munadi-sfx.yml`** - Using latest pre-built [SFX archive](#primary-release)

### Regular build

```bash
# Make the manifest available on repo root
cp -a pkg/flatpak/org.munadi.Munadi.yml

# Build the regular version with platform runtimes and install
# Note: installation is necessary
flatpak run --command=flathub-build org.flatpak.Builder --install org.munadi.Munadi.yml

# Run the built app
flatpak run --user --branch=master org.munadi.Munadi
```

### SFX archive build

```bash
# Make the manifest available on repo root
ln -s pkg/flatpak/org.munadi.Munadi-sfx.yml

# Build static version with linked libraries
flatpak-builder \
--verbose \
--force-clean \
--user \
--install-deps-from=flathub \
--ccache \
--repo=./repo ./builddir ./org.munadi.Munadi-sfx.yml

# Run the built app
flatpak-builder --run \
--env=WAYLAND_DISPLAY="$WAYLAND_DISPLAY" \
./builddir ./org.munadi.Munadi-sfx.yml munadi
```

### Known Issues

- Few core & UI features like adding custom adhan doesn't work as expected due to flatpak sandboxing
  when using `flatpak-builder`. Alternative is to install and run the app from the functional
  sandbox but introduces more clutter.

</details>

## 3. Standalone development on IDE

The app can be built standalone for debugging and developing purposes. We recommend using an IDE
over plugin-loaded text-editors (TUI or not). The repository already contains some essential
auto-formatting and linting configurations for improving development experience (DX). VSCode,
VSCodium and Qt Creator are highly recommended, although you're free to choose your preferred
program. For UI developments especially, Qt Design Studio is highly preferred.

### Prerequisites

> [!NOTE]
>
> Packages refer to development packages where applicable. Generic package names are provided. These
> may not match on all distributions. Search your distribution's package repository for accurate
> package names.

> [!WARNING]
>
> Qt versions older than 6.12 are not compatible. Build with project's latest tracking Qt version
> before filing bugs. Alternatively,
> [test build statically](#1-static-build-via-builder-image-recommended).

#### Install instructions recommended IDEs

- [VSCodium](https://vscodium.com/install)
- [Qt Creator](https://doc.qt.io/qtcreator/creator-how-to-install.html)
- [VSCode](https://code.visualstudio.com/download)

#### Recommended Linters and Formatters

- [clang-tidy](https://clang.llvm.org/extra/clang-tidy/)
- [clangd](https://clangd.llvm.org/installation)
- [Clazy](https://github.com/KDE/clazy)
- [Prettier](https://prettier.io/docs/install)
- [qmlls](https://doc.qt.io/qt-6/qtqml-tooling-qmlls.html)
- [Ruff](https://docs.astral.sh/ruff/)
- [ShellCheck](https://github.com/koalaman/shellcheck)

#### Build tools

- `ccache` (recommended, for faster rebuilds)
- `cmake >= 3.30`
- `dbus`
- `gcc-c++` or `clang++`
- `libzstd`
- _If using clang:_
  - `lld`
  - `llvm`
  - `clang-tools-extra`
- `nasm`
- `ninja-build`
- _For packaging:_
  - `tar`
  - `zstd`

**Libs: `<pkg>-devel` or `<pkg>-dev`**

- `qtbase`
- `qtdeclarative`
- `qtimageformats`
- `qtlocation`
- `qtmultimedia`
- `qtpositioning`
- `qtshadertools`
- `qtsvg`
- `qtvirtualkeyboard`
- `qtwayland`
- `libglvnd`
- `libxkbcommon`
- `libzstd`
- `openssl`
- `pipewire`
- `vulkan-loader`
- `wayland`

After installing the packages, open the repository folder or the main `CMakeLists.txt` on project
root in your IDE.

**_Tips:_** Build with `Debug` or `RelWithDebInfo` configuration for further debugging.

## Guidelines

### Versioning

Version format follows `YY.MM` convention (e.g., `21.04` for April 2021). Subsequent releases within
same month follows `YY.MM.m` convention (e.g., `21.04.1` for another release on April 2021).

### Code Guidelines

The codebase follows a preferred coding and formatting style derived from Microsoft style. Usage of
clang-tidy is highly recommended. Please try following these guidelines for easier merge and
fathomable diffs. Too much deviation in code-style may cause MRs/contributions to be dropped.

#### C++

- Prefer readability over clever code-gymnastics when space-time cost is unaffected
- Use inline comments to briefly explain or keep note of the following block
- Prefer using out-of-line definitions, applicable for member variables too. Use inline definitions
  for static member variables. You may use inline member function definitions when body is
  guaranteed to be trivial, but still discouraged.
- Prepend all members with `this->`, e.g. `this->m_var`, `this->myFunc()`. For static members use
  full qualification, e.g. `MyClass::m_var`, `MyClass::myFunc()`
- Fully state return types, do not keep them implied, even in lambdas. This equally applies to using
  `auto` in variables init except where applicable (e.g. redundant type specifications)
- Prefer trailing return type function declaration in declarations, use regular format in
  definitions
- Member functions are preferred over overloaded operators when available, for example, use
  `myVar.set(val)` over `myVar = val`
- Prefer using `{}` for default-constructed value over redundant assignment
- Prefer using braces `{}` over `()` for variable initiation
- Only members associated (or closely so) with public function returns get `m_`/`s_`/`b_` prefixes,
  use plain names otherwise
- When using pointers, use bare pointers for Qt-managed objects, QPointer for non-owned objects, and
  `std::unique_ptr<T>` for owned objects not managed by Qt
- `defaultConfigs()` function should initiate class members without dependency on any other Engines
- `NOLINT` (and siblngs) or other lint-disabler comment maybe used only if you know what you're
  doing. Highly recommended NOT to otherwise
- Use Qt string literal operators `u""_s` for owning types (e.g. assignments, init, comparisons,
  table keys, returns) and `""_L1` for non-owning types or views (e.g. passing to parameters,
  non-persistent uses). Equally applies to using `QString` and `Q*StringViews`
- Run a pass with clazy upto level2 checks
- Consider warnings generated at build-time to be fatal
- For MunadiEngines (the `*Engine` classes), try not to extend their scopes, each Engine class
  should remain on the scope its prescribed and characteristically applicable for, do not mix-match
  responsibilities.
- Containers and structures need not be UI-facing and should not aim for either, that is the job for
  UiEngine to handle. If a container/structure can be used in the UI as-is, then UiEngine should not
  redundantly act to be a translation layer.
- All Engines must depend on the CoreEngine, either implicitly or directly
- Prefer using properties over parameterized `Q_INVOKABLE` functions, except where applicable. The
  latter introduces more clutter keeping QML-side and backend in sync
- Remember to run `lupdate` on all source files when finalizing. Use this command from project root:

  ```bash
  # Adjust path accordingly
  /path/to/qt/bin/lupdate src/qml/ src/cpp/ -ts src/i18n/munadi_bn.ts src/i18n/munadi_ar.ts
  ```

#### QML & UI

- Aim for responsive UI
- Arrange your design on the default window size (refer to `Main.qml`), then verify if it sticks on
  resize
- Prefer using Layouts over regular components, this allows using `Layout` properties to arrange
  items over `anchors`, which are not as responsive as former.
- Prefer `pointSize` for texts over `pixelSize`, the latter doesn't scale with DPI so well
- Use relative resizing over fixed sizes to maintain proportionality, but be careful about
  parent-children recursive bindings
- Environment variables `QSG_VISUALIZE=overdraw` and `QSG_VISUALIZE=batches` may come in handy
  sometimes

### Icons

- Prefer SVGs generally over PNGs as they do scaling well. When using PNGs, use a bigger size as
  relevant, e.g. 96x96 for button/item icons.
- Perform SVG cleanup before adding to repository, `scour` is recommended:

  ```bash
  scour \
    --create-groups \
    --enable-comment-stripping \
    --enable-viewboxing \
    --nindent=4 \
    --remove-descriptive-elements \
    --set-precision=8 \
    --strip-xml-prolog \
    --strip-xml-space \
    input.svg output.svg
  ```

  Add your desired xml elements only after cleanup, else it will be stripped too.

- Prefer to use (cleaned) SVG-converted QML icons if using in QML unless otherwise necessary. Use
  `svg2qml` for the purpose:

  ```bash
  svg2qml -cap input.svg output.svg
  ```

  Scrutinize the QML icon file, and strip-off `QtQuick.VectorImage` related elements if not
  necessary for the drawable. Flatpak has issues building QML files with VectorImage.

### General

- Maintain [versioning scheme](#versioning)
- Prefer incremental commits over larger ones for easier reverts and debugging
- In-source building is supported, but highly discouraged. Always prefer out-of-source building
- Update `CHANGELOG.md` and XDG metainfo with notable changes.
- Suggested to set these environment variables on app executions for extended-yet-sane level of
  logging:
  - `QT_MESSAGE_PATTERN="[%{time process} %{type}] %{category} - %{message}"`
  - `QT_LOGGING_RULES="*.debug=true;qt.*.debug=false;qt.*.info=true;kf.*.debug=false"`
- A symbols-map file is available as `munadi.map` under the build tree when building statically on
  Linux. Use it for extended symbols-linked information and debugging purposes

### Validations

- For UI, test elements' sanity in all possible forms and sizes. Recommended to test in HiDPI
  displays if possible
- Test in both static build and regular on a clean "Release" config build. Use builder image for
  reproducible environments.
- Test all packagings, use verbose loggings to catch bugs. For makeself installer, pass
  `MUNADI_DEBUG=1` as environment variable to enable extended debug logging.
- Validate the metainfo file before finalizing

  ```bash
  appstreamcli validate /path/to/your.metainfo.xml
  ```

#### Extras For Flatpaks

- Validate your manifest first:

  ```bash
  flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest /path/to/your/org.munadi.Munadi.metainfo.xml
  ```

  > [!NOTE]
  >
  > Make sure your manifest uses
  > [latest tagged commit of the project](https://gitlab.com/munadi/munadi/-/tags) as the source. Do
  > not use other commits as those are not guaranteed to be stable.

- Then your AppStream metainfo:

  ```bash
  flatpak run --command=flatpak-builder-lint org.flatpak.Builder metainfo /path/to/your.metainfo.xml
  ```

- Then test build and install [following the recommended workflow](#2-build-as-flatpak). Replace
  with your manifest.

- Additionally, check your repo after building

  ```bash
  flatpak run --command=flatpak-builder-lint repo repo
  ```

---

After passing all validations and ensuring code-base follows guidelines,
[submit a merge request](https://gitlab.com/munadi/munadi/-/merge_requests/new) to master branch
with a brief summary of the MR. Only include necessary points that make the MR noteworthy.

Contributions are also warmly accepted on the
[flathub repository](https://github.com/flathub/org.munadi.Munadi).
