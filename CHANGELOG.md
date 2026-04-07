# Changelog

All notable changes to this project will be documented in this file.

---

## [26.04](#munadi-2604) — 2026-04-01

### Bug Fixes

- Resolve typos in `install` and `uninstall` scripts, simplify code
- Fix tooltip "Time Left" not updating when started hidden or after sleep
- Show time left to next Fajr after Isha instead of waiting for 23:59
- Prevent prayer labels running into adjacent time values; elide if parent too narrow

### Improvements

- Upgrade to Qt 6.11
- Left-align Prayer Times table labels
- Show "Time Left" string only on current date
- Skip time-left calculations for non-today prayers

### Build & Infrastructure

- Switch to Rocky Linux 10.1 as builder base image
- Use `/tmp` for archive creation instead of CWD
- Simplify Prayer Times table object alignment approach
- Enable clickable hyperlinks on What's New page
- Add CI/CD pipeline configuration
- Munadi-builder docker image for app development now available at the project's registry
- Polish README.md for user and developer facing briefing
- Update screenshots

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

Munadi can now be installed using a self-extracting installer. No need to download and manage loads of dependencies—just make it executable (`chmod +x`) and run the interactive installer.

> Latest installer: [pkg/makeself](https://munadi.org/get/26.02)

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

Special thanks to [Hasan Esa](https://gitlab.com/HasanEsa) for the contributions and motivation, more will follow as a result insha-allah.

---

## [22.04](#munadi-2204) — 2022-03-31

### Ramadan Kareem Release

- Dark mode
- Major UI changes
- Improved tray UI and usability
- Flatpak integration (autostart, background, hidden autostart)

---

## [21.12](#munadi-2112) — 2021-12-01

- Option to hide window when closed (run in background)
- Reworked alarm code with sleep and time change detection
- Flag as ready for mobile Linux (Phoc, Plasma Mobile)

---

## [21.04.1](#munadi-21041) — 2021-04-28

- Fixed broken progress bar/time left display

---

## [21.04](#munadi-2104) — 2021-04-01

- UI enhancements
- Reworked alarm code
- Upgraded Adhan.js library to v3
- Restored Android support (coming soon)
- Fixed no sound issue on Ubuntu
- Restored date navigation (scroll or click date for previous/next day)

---

## [21.02](#munadi-2102) — 2021-02-01

- Prayer times now correct for countries with n.5 hours UTC offset
- Full localization of date, time, and UI
- Hijri disabled temporarily (localization not available)

---

## [20.10](#munadi-2010) — 2020-10-01

### Tray Icon Support

- Click (or double-click) tray icon to hide/show Munadi
- Right-click for context menu
- Click "time left" to stop currently playing Athan

> **Note:** Not all desktop environments support tray icons consistently.

---

## [20.09.1](#munadi-20091) — 2020-09-26

### RTL & Snap Improvements

- Behind-the-scenes RTL improvements
- Re-enabled window switching via dragging
- Switched to core20 and CMake for forward-compatible Snap support

---

## [20.09](#munadi-2009) — 2020-09-01

### RTL Language Support

- Added RTL language support
- Added Arabic translation (thanks to Damascene)
- Minor enhancements

---

## [20.07](#munadi-2007) — 2020-07-01

- Added more calculation methods
- Ability to add custom Athans

> **Note:** Flatpak is now the preferred packaging method. This may be the last Snap release.

---

## [19.12](#munadi-1912) — 2019-12-01

### Snap Packaging

- Removed about page (merged into settings)
- Added Snap support
- Minor UI changes

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

*For full commit history, see [GitLab](https://gitlab.com/munadi/munadi/-/commits/master).*
