# Changelog

All notable changes to this project will be documented in this file.

---

## [26.11](#munadi-2611) - 2026-11-07

### Fixes

- RTL switch now correctly happens on Arabic language
- Flatpak Munadi now correctly shows and autostarts alongside SFX Munadi in app drawer/menu if both
  are installed
- Logging in SFX installer is now in-sync with actual job operations
- Uninstalling the SFX Munadi correctly prunes config and store leftovers by default. Can be opted
  out by passing `--nocleanup` or `-N`

### Improvements

- The UI has a fresh look with bolder texts that stands out with more refined and proportional UI
  components.
- The location searching operation has been improved to be more robust in cases of network failures
  and repeated requests
- The QApplication instance `app` is now a fully-fledged SingleApplication derived instance, merging
  & simplifying the job-load of `sapp` and `app` in one, while also dropping embedding `sapp` in
  root context of Qml
- X11 dependency is finally removed in static build
- Translations were updated along with automatic RTL layout switching according to language
- Location is now shown in full address rather than truncated in short
- Makeself pre-checks are now more accurate than before

### New

- Added prayer-time icons! System-tray icons are also updated.
- Sunnah times like midnight and Last third of the night times were added
- The qibla page is back! Currently it only shows static qibla direction for the currently set
  location without device compass support
- Language can now be selected from UI, or let system-default be followed
- Relevant controls can now be adjusted using locale-aware spinboxes. `SpinBoxComponent` is a new
  template component for aiding in adding such spinboxes across the UI.
- MunadiEngine, the new C++ backend that migrates the former QML/JS-based app-logic operations in
  MunadiModel to actual strongly-typed C++ code. The abstract MunadiEngine is realized in handful of
  engines - CoreEngine, UpdateEngine, LocationEngine, PrayerEngine, MultimediaEngine and the
  UiEngine - which carry out responsibilities obvious for their purposes. CoreEngine handles all the
  low-level tasks of the app and deemed as the heart of it, UpdateEngine handles updates, checks and
  fetch (WIP), LocationEngine carrys out all Geocoding operations, PrayerEngine takes care of all
  prayer times calculations, MultimediaEngine sees all media-related tasks and UiEngine sits closest
  to the UI for all business logics required there, sometimes acting as a thin-translation layer
  between other engines and the UI interface. This backend-frontend split greatly reduced QML-side
  business logics towards more declarative usages while the engines handle everything in background,
  eliminating the hassle of working around type-safety and property bindings, enhancing operations
  end-to-end and letting jobs worked out more strongly and safely. This also opened an opportunity
  to use other UI forms than graphical display, e.g. TUI.
- Munadi now uses a new C++ adhan calcuations lib from
  [Lotconn/Adhan-C++](https://github.com/lotconn/adhan-cpp) replacing the former JS lib. The whole
  is integrated in the C++ backend (PrayerEngine).

### Build & Infrastructure

#### App build (general)

- **DIFF**: `cf-protection` is dropped for FFmpeg build
- **DIFF**: Builder image uses Clang+LLVM now
- **DIFF**: Qt 6.12 LTS + FFmpeg 9.0.1 is the new baseline now
- **DIFF**: Update Lotconn/FileMan version
- **NEW**: Add Lotconn/Adhan-C++ library
- **NEW**: Added `asynchronous-unwind-tables` and `no-omit-leaf-frame-pointer` flags for FFmpeg
- **NEW**: Auto-generate translation files and QML icon files from SVGs build-time

#### CI/CD & Packaging

- **DIFF**: (static-builder) Do not link Openssl in Qt build anymore, use the system-installed
  library
- **ENH**: (static-builder) Download-stage in static-builder image building are parallelized
- **ENH**: (static-builder) Trimmed and streamlined some unused tools in Qt build
- **ENH**: (static-builder) Update base image to Rocky Linux 10.2 UBI
- **FIX**: (CI) CI pipeline fails due to invalid command line in `build-web-server` stage
- **FIX**: (CI) CI pipeline fails to use the static-builder image for building the app
- **FIX**: (makeself) A bug in SFX archive's shell scripts causes archive generation to fail or
  become incomplete
- **FIX**: (static-builder) Fixed Qt module dependency hierarchy, causing some features and tools to
  be missed. Couple of new modules and packages were added for dependency reasons

#### Docs

- **NEW**: Separate CONTRIBUTING.md for various contrib guidelines, also keeps technical clutter
  away from landing-page README
- Update phrases and documentations

#### Flatpak

- **DIFF**: Remove the static manifest `org.munadi.Munadi-static` in favor of `-sfx`
- **NEW**: `MUNADI_LINUX_DIST=(Default|Flatpak)` cmake option to specify flatpak building explicitly
- **NEW**: Arabic translations in metainfo file
- Update manifest for code changes and new release

#### Project Misc

- **NEW**: Add auto-formatting, auto-completion, static linting and IDE settings configs
- Stale, migrated and replaced files were removed, some files were renamed

Full Changelog: [26.08...26.11](https://gitlab.com/munadi/munadi/-/compare/26.08...26.11)

---

## [26.08](#munadi-2608) - 2026-08-15

### Bug Fixes

- Condition Intel CET for x86_64/amd64 only to prevent aarch64/arm64 build failures
- Fix screenshots having activity pin button, deviating from "default setup"
- Fix some numerals not being translated in the foreign locale
- Fix missing whitespaces and incomplete literals in Arabic translations

### Improvements

- Replace KDSingleApplication with SingleApplication by @itay-grudev that works across platforms
- Update `org.kde.Platform` to `6.11`
- Full Arabic and Bangla localization, including dynamic texts like calendar dates
- Enable RTL (Right-to-Left) layout support for Arabic with minor UI adjustments
- Use variable fonts instead of static ones for Arabic and Bangla to improve rendering and reduce
  size
- Update FileMan to the latest release and adapt QML components to its new API
- Simplify homepage name to just "Munadi", moving the summary to the heading
- Refactor QML components to use `let`/`const` instead of `var` for stricter scoping
- Optimize `Qt.application.layoutDirection` handling by moving it to respective UI layers

### Build & Infrastructure

#### App build (general)

- Replace submodules (`KDSingleApplication`, `FileMan`) with CMake `FetchContent`
- Increase CMake maximum policy version to 4.4
- Set C++ standard to C++23 for MSVC compatibility
- Drop Qt policy versions to LTS 6.8. Policy bumps will only occur on LTS versions from now on
- Enable conditional IPO/LTO, global PIC, and MSVC control flow guard flags
- Fix GCC debug optimization warnings and enable additional useful warnings for GCC/Clang

#### CI/CD & Packaging

- Build and push web-server container image to the container registry in the CI pipeline
- Gate only git-tracked files into the makeself SFX archive
- Generate SHA256 digest of the built package and make the file available with the package in the
  release page
- Use a temporary markdown-formatted file for the release description to allow further formatting

#### Docs

- Drop recommendation for "Assisted by: " footers
- Add new contributor @khaledkhamis26 in the list
- Update README.md to highlight full Arabic & Bangla support and fix reference links
- Reflect FileMan namespace changes in documentation

#### Flatpak

- Lowercase summary line in metainfo to comply with Flathub's standards
- Make `FetchContent` usable inside network-offline flatpak build environment

#### Project misc

- Remove stale `src/cpp/libitl/` and `src/cpp/dummysapp.h` from tree
- Add and update IDE/linter configurations (`.clangd`, `.prettierrc`, `.qmlls.ini`,
  `.vscode/settings.json`)
- Clean up `.gitignore` and apply project-wide auto-formatting

Full Changelog: [26.06...26.08](https://gitlab.com/munadi/munadi/-/compare/26.06...26.08)

---

## [26.06](#munadi-2606) - 2026-06-05

### Bug Fixes

- Fix tooltip time-left string doesn't handle time jumps.
- Fix a warning about calling uninitiated handler being deprecated
- Fix custom audio list becoming stale if was configured for a prayer when a random custom audio is
  removed
- "Contacts" link in the settings page is broken
- Fix `uninstall` command on SFX archive doesn't uninstall anything

### Improvements

- **New Icon**: Munadi now uses a new material-gradient inspired icon derived from Masjid Al-Aqsa.
- Use the brand shades for accent color throughout
- Update the symbolic icon to be a sillohuette of the new icon
- Add `xdg/icons/hicolor/scalable` for SVGs
- Add 80px and 480px icons under `xdg/icons`
- Do not calculate `timeLeftStr` for next fajr past 1100 minutes; handles even the polarmost
  settlements
- Improve keyboard navigations on location search popup
- Let UI stretch freely within 1:1 proportions, snap to left as usual otherwise
- Use a thinner font stroke on prayer table
- Right align prayer times on the table
- Minimum width bounds also follows UI proportions now, despite disrupting settings page view
- Replace "+" and "-" beside the speaker icon with actual texts to make the buttons' functions
  obvious. These are more often than not misunderstood for volume control buttons
- Add a "Video" category option in file dialog to let users add video files as audio files too which
  is supported natively
- Minor code restructures
- Lazy-load/save config and make settings changes atomic
- New file manager backend for improved, robust & stateless store management all while a better
  experience inside sandboxes (e.g. flatpaks, snap)
- Upgrade to Qt 6.11.1
- Add back Arabic support

### Build & Infrastructure

#### App build (general)

- Enable more aggressive optimizations and useful hardenings
- Qt 6.9.0 baseline standardization
- Build system improvements and streamlining

#### Flatpak

- Migrate manifests to YAML
- Add another manifest for a variant using SFX archive
- Update the static manifest to correspond with recent workflow changes
- Sync `org.munadi.Munadi.yml` with flathub upstream
- Drop $XDG_MUSIC_HOME permission since it's not needed anymore

#### makeself archive

- Register munadi symbolic icon on the execution scripts

#### Builder Images

- Enable aggresive optimizations and level-up hardening generally
- Drop network support until Munadi itself starts supporting
- Label following OpenContainer Annotations Spec
- Add clang and dependencies/tools

#### Docs

- Update README.md with changes and rephrases, better readability, fix, improve & amend workflows,
  formatting and references
- Update CHANGELOG.md
- Refresh feature list and put up a better summary/marketing line for the app

#### Web

- Update verification ID for flathub
- Add "Acknowledgements" & "Contributors" sections enlisting used libraries and contributors for the
  whole project
- Refresh feature list and put up a better summary/marketing line for the app

#### Project misc

- Symlink redundant icon files across repo to those under `xdg/icons`
- Move all image building relevant files under `pkg/docker` categorized subfolders

Full Changelog: [26.04...26.06](https://gitlab.com/munadi/munadi/-/compare/26.04...26.06)

---

## [26.04](#munadi-2604) — 2026-04-18

### Bug Fixes

- Resolve typos in `install` and `uninstall` scripts, simplify few code
- Fix tooltip "Time Left" not updating when started hidden or after sleep
- Show time left to next date's Fajr after Isha instead of waiting for 23:59
- Prevent prayer labels running into adjacent time values; elide if parent too narrow

### App Improvements

- Upgrade to Qt 6.11 (static build only)
- Left-align Prayer Times table labels
- Show "Time Left" string only on current date
- Skip time-left calculations for non-today prayers
- Clamp the Settings page width to 770.0 px if parent window is too wide
- Limit smallest resizable window size to 350x350 px
- Simplify Prayer Times table object alignment approach
- Enable clickable hyperlinks on What's New page
- Update KDSingleApplication to v1.2.0

### Build & Infrastructure

#### Static libs building

- Downgrade FFmpeg to Qt 6.11 recommended version (n7.1.3)
- Compact builds by trimming unused libraries, program & components other than the app's purpose,
  use whitelisting to allow only what's necessary
- Enable link-time optimization at the cost of building time
- Disable documentations and debugging on FFmpeg and Qt builds
- Further ensure no X dependencies (still hard to avoid anyway)
- Build other Qt modules with similar build options as qtbase
- Replace pulseaudio libs with pipewire
- Add few optimization and performance cost-free hardening to Qt and FFmpeg build

#### Builder image

- Rename Dockerfile more generically as Containerfile; keep it symlinked to Containerfile
- Switch to Rocky Linux 10.1 as builder base image
- Build a trimmed, from-scratch image bundled with only necessary files & programs. This however,
  doesn't shrink the built image's size but helps to provide a custom image in a truer sense.
- Add various linting & validation utils in the image for comprehensive development tasks. Only SFX
  installer related tools now, more will be added soon. Other tools can be installed after container
  creation with `dnf` if necessary.
- Add various labels to describe this custom image
- Simplify web server image; use the official gunicorn image instead of building from python-core
  image. Significantly reduces image size.
- Add few optimization and performance cost-free hardening during app building

#### Gitlab CI/CD

- Add CI/CD pipeline configuration
- Publish "Munadi static app builder" image to the project's registry
- Don't include built archives anymore, those should be available on "Releases" from now on

#### makeself archive

- Use `/tmp` for archive creation instead of CWD
- Level-up compression
- Log more consistently according to log-level
- Various logging improvements
- Show `uninstall` script output if invoked in `install` script with same verbosity level

#### Project misc

- Polish README.md for user and developer facing briefing & organize CHANGELOG.md
- Add AI contribution policy for the project
- Append -static to the flatpak static manifest again
- Update screenshots
- Rename `pkg/flathub` to more generic `pkg/flatpak`
- Convert `libs/KDSingleApplication` to a git submodule
- Remove unneeded `libs/SFML-2.4.0`

Full Changelog: [26.02...26.04](https://gitlab.com/munadi/munadi/-/compare/26.02...26.04)

---

## [26.02](#munadi-2602) — 2026-02-03

### Bug Fixes

- Fix 'System' theme not following system theme changes
- Fix muted athan playing on next start
- Fix time not updating in tooltip
- Allow exiting fullscreen during first onboarding
- Prevent text overflow and running into each other
- Android build updates

### New Features

- Improved main page appearance
- Monochrome tray icon
- Two additional built-in athans (including Fajr)

### Self-Extracting Installer

Munadi can now be installed using a self-extracting installer. No need to download and manage loads
of dependencies—just make it executable (`chmod +x`) and run the interactive installer.

> Latest installer: [pkg/makeself](https://munadi.org/get/26.02)

Full Changelog: [25.11...26.02](https://gitlab.com/munadi/munadi/-/compare/25.11...26.02)

---

## [25.11](#munadi-2511) — 2025-11-24

Long-awaited features and enhancements:

- New theme with time-of-day wallpapers
- System theme integration
- Prayer-time window opacity adjustment
- Background blur adjustment
- 24-hour time format
- Time offset adjustments (–30 to +60 minutes)
- Separate custom athan for each prayer
- Choose between Hijri, Gregorian, or both dates
- Hijri date offset adjustments
- Improved keyboard navigation
- [Flatpak] Statically linked libraries
- Fixed bugs
- Slimmed codebase

Special thanks to [Hasan Esa](https://gitlab.com/HasanEsa) for the contributions and motivation,
more will follow as a result insha-allah.

Full Changelog: [22.04...25.11](https://gitlab.com/munadi/munadi/-/compare/22.04...25.11)

---

## [22.04](#munadi-2204) — 2022-03-31

### Ramadan Kareem Release

- Dark mode
- Major UI changes
- Improved tray UI and usability
- Flatpak integration (autostart, background, hidden autostart)

Full Changelog: [21.12...22.04](https://gitlab.com/munadi/munadi/-/compare/21.12...22.04)

---

## [21.12](#munadi-2112) — 2021-12-01

- Option to hide window when closed (run in background)
- Reworked alarm code with sleep and time change detection
- Flag as ready for mobile Linux (Phoc, Plasma Mobile)

Full Changelog: [21.04.1...21.12](https://gitlab.com/munadi/munadi/-/compare/21.04.1...21.12)

---

## [21.04.1](#munadi-21041) — 2021-04-28

- Fixed broken progress bar/time left display

Full Changelog: [21.04...21.04.1](https://gitlab.com/munadi/munadi/-/compare/21.04...21.04.1)

---

## [21.04](#munadi-2104) — 2021-04-01

- UI enhancements
- Reworked alarm code
- Upgraded Adhan.js library to v3
- Restored Android support (coming soon)
- Fixed no sound issue on Ubuntu
- Restored date navigation (scroll or click date for previous/next day)

Full Changelog: [21.02...21.04](https://gitlab.com/munadi/munadi/-/compare/21.02...21.04)

---

## [21.02](#munadi-2102) — 2021-02-01

- Prayer times now correct for countries with n.5 hours UTC offset
- Full localization of date, time, and UI
- Hijri disabled temporarily (localization not available)

Full Changelog: [20.10...21.02](https://gitlab.com/munadi/munadi/-/compare/20.10...21.02)

---

## [20.10](#munadi-2010) — 2020-10-01

### Tray Icon Support

- Click (or double-click) tray icon to hide/show Munadi
- Right-click for context menu
- Click "time left" to stop currently playing Athan

> **Note:** Not all desktop environments support tray icons consistently.

Full Changelog: [20.09.1...20.10](https://gitlab.com/munadi/munadi/-/compare/20.09.1...20.10)

---

## [20.09.1](#munadi-20091) — 2020-09-26

### RTL & Snap Improvements

- Behind-the-scenes RTL improvements
- Re-enabled window switching via dragging
- Switched to core20 and CMake for forward-compatible Snap support

Full Changelog: [20.09...20.09.1](https://gitlab.com/munadi/munadi/-/compare/20.09...20.09.1)

---

## [20.09](#munadi-2009) — 2020-09-01

### RTL Language Support

- Added RTL language support
- Added Arabic translation (thanks to Damascene)
- Minor enhancements

Full Changelog: [20.07...20.09](https://gitlab.com/munadi/munadi/-/compare/20.07...20.09)

---

## [20.07](#munadi-2007) — 2020-07-01

- Added more calculation methods
- Ability to add custom Athans

> **Note:** Flatpak is now the preferred packaging method. This may be the last Snap release.

Full Changelog: [19.12...20.07](https://gitlab.com/munadi/munadi/-/compare/19.12...20.07)

---

## [19.12](#munadi-1912) — 2019-12-01

### Snap Packaging

- Removed about page (merged into settings)
- Added Snap support
- Minor UI changes

Full Changelog: [18.12...19.12](https://gitlab.com/munadi/munadi/-/compare/17.02...19.12)

---

## [18.12](#munadi-1812) — 2018-12-01

- Removed: Date can now go forward and backwards
- Individual Athan volume configuration
- Added Android and Flatpak support
- UI enhancements

---

## [17.02](#munadi-1702) — 2017-02-01

- Brand new UI with latest Qt features
- Toggle between Hijri and Miladi dates
- System proxy support by default
- 3x lighter codebase
- Forward and backward date navigation
- Removed Iqama functionality
- Added keyboard shortcuts (see help)
- Many bug fixes

---

## [15.05](#munadi-1505) — 2015-05-01

### Iqama Time (Experimental)

- Get Iqama time from any mosque
- Add mosques to favorites
- Add mosques to Munadi's database
- Add monthly timetable to any mosque
- Edit mosque details and timetable
- **Note:** Please be responsible; we can't validate every mosque

### User Interface

- Added animations
- New font and color scheme
- Improved performance

### Other Changes

- Integrated help
- Automatic DST detection
- New calculation methods
- City search instead of menu selection
- Many behind-the-scenes improvements

---

## Get in Touch

Visit [our homepage](https://munadi.org).

Follow us on social media for updates, news, and tutorials:

- [Twitter](https://twitter.com/munadi_org)
- [YouTube](https://youtube.com/@munadi_org)

---

_For full commit history, see [GitLab](https://gitlab.com/munadi/munadi/-/commits/master)._
