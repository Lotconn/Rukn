# Changelog

All notable changes to this project will be documented in this file.

---

<div><a id="rukn-26.11"></a></div>
<div><a name="rukn-26.11"></a></div>

## 26.11 - 2026-11-16

The first introduction of Rukn. Changes here reflect modifications past the fork from munadi/Munadi

### New

- Added prayer-time icons! System-tray icons are also updated.
- Sunnah times like midnight and Last third of the night times were added
- The qibla page is back! Currently it only shows static qibla direction for the currently set
  location without device compass support
- Language can now be selected from UI, or let system-default be followed
- Relevant controls can now be adjusted using locale-aware spinboxes. `SpinBoxComponent` is a new
  template component for aiding in adding such spinboxes across the UI.
- AppEngine, the new C++ backend that migrates the former QML/JS-based app-logic operations in
  MunadiModel to actual strongly-typed C++ code. The abstract AppEngine is realized in handful of
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
- Rukn now uses a new C++ adhan calcuations lib from
  [Lotconn/Adhan-C++](https://github.com/lotconn/adhan-cpp) replacing the former JS lib. The whole
  is integrated in the C++ backend (PrayerEngine).
- Background images were updated with new and fresher visuals
- Updated screenshots for 2611 version
- Auto update checks are disabled until an API is formed

### Fixes

- RTL switch now correctly happens on Arabic language
- Flatpak Rukn now correctly shows and autostarts alongside SFX Rukn in app drawer/menu if both are
  installed
- Logging in SFX installer is now in-sync with actual job operations
- Uninstalling the SFX Rukn correctly prunes config and store leftovers by default. Can be opted out
  by passing `--nocleanup` or `-N`

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

- **DIFF**: (ci) Web-server building is disabled
- **DIFF**: (static-builder) The image container is now at `ghcr.io/lotconn/rukn`
- **DIFF**: (static-builder) Do not link Openssl in Qt build anymore, use the system-installed
  library
- **ENH**: (static-builder) Download-stage in static-builder image building are parallelized
- **ENH**: (static-builder) Trimmed and streamlined some unused tools in Qt build
- **ENH**: (static-builder) Update base image to Rocky Linux 10.2 UBI
- **ENH**: (makeself) Generalized hardcoded branding literals with variables where applicable
- **FIX**: (makeself) A bug in SFX archive's shell scripts causes archive generation to fail or
  become incomplete
- **FIX**: (static-builder) Fixed Qt module dependency hierarchy, causing some features and tools to
  be missed. Couple of new modules and packages were added for dependency reasons

#### Docs

- **NEW**: Separate CONTRIBUTING.md for various contrib guidelines, also keeps technical clutter
  away from landing-page README
- Update phrases and documentations

#### Flatpak

- **NEW**: `APP_LINUX_DIST=(Default|Flatpak)` cmake option to specify flatpak building explicitly
- **NEW**: Arabic translations in metainfo file
- Update manifest for code changes and new release

#### Project Misc

- **NEW**: Add auto-formatting, auto-completion, static linting and IDE settings configs
- Stale, migrated and replaced files were removed, some files were renamed
- Not-planned source files were removed from tree for now
- Project renamed from munadi/Munadi to Lotconn/Rukn

Full Changelog: [26.08...26.11](https://gitlab.com/lotconn/rukn/-/compare/26.08...26.11)

---

## Get in Touch

Visit [our homepage](https://lotconn.org).

Follow us on social media for updates, news, and tutorials:

- [Twitter](https://twitter.com/lotconn_org)
- [YouTube](https://youtube.com/@lotconn_org)

---

_For full commit history, see [GitLab](https://gitlab.com/lotconn/rukn/-/commits/master)._
