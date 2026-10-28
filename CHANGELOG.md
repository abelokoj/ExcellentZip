# Changelog

## [2026-10-28.1](https://github.com/abelokoj/ExcellentZip/releases/tag/2026-10-28.1)

Recorded 2026-10-28T10:41:15-04:00 (recording time). Publication confirmed 2026-10-28T11:40:14-04:00 (recording time). Installer and portable downloads are available.

- Adds installer choices for all supported file types or an entered list of extensions. Selected-type registration adds only those capabilities while preserving other existing registrations and Windows default choices.
- Adds an extension-entry field to Tools → Options → File types and shares validated, catalog-only registration with the installer's in-app confirmation guide.
- Adds interface zoom from 100% to 200%, including custom whole percentages and presets. The saved setting takes effect on the next launch and scales native dialog fonts/layout, file-list text, toolbar, address/status bars, and modern app dialogs. Windows-owned menus and prompts keep system sizing.
- Keeps enlarged dialogs reachable with scrolling when they exceed the available desktop area and allows toolbar commands to move into overflow.

Validation update 2026-10-28T11:37:53-04:00 (recording time): 13 packaging/version checks, eight asset checks, two source-layout checks, native icon/download/default-registration checks, and zoom parsing/persistence/template tests passed. Selected-only registration preserves existing default choices; custom percentages, invalid settings, and scrolling to off-screen dialog controls are covered. Windows app compilation and native installer/portable packaging passed. ZIP/encrypted-7Z archive operations, checksums, ZIP CRCs, default-only icons, and the shipped editable zoom/extension-entry dialog controls were verified. Windows requires confirmation for default-app changes; live confirmation and visual layout checks remain manual. Builds are unsigned. The four public release downloads and their uploaded artifact hashes/sizes were verified.

## [2026-10-27.2](https://github.com/abelokoj/ExcellentZip/releases/tag/2026-10-27.2)

Recorded 2026-10-27T03:24:49-04:00. Publication confirmed 2026-10-27T03:47:54-04:00 (recording time). Installer and portable downloads are available.

- Corrects the executable description and Windows application registration so the app picker shows ExcellentZip.
- Adds Tools → Options → File types with all 62 extensions, current default-app names, native checkbox selection, Select all/Clear, and guided Windows confirmation for all or selected extensions.
- Adds an optional installer step to choose default file types in Windows. Existing default choices are preserved until the user confirms them.
- Refreshes file-type previews and ExcellentZip-owned icons from the saved pack when defaults change.

Validation: 12 packaging/name/installer checks passed. Native tests passed for all 62 capability registrations, exact app naming, extension/URI validation, launch failures, preserved existing defaults, and compilation of the File types controls/resources. Icon/download tests also passed. Validation update 2026-10-27T03:47:54-04:00 (recording time): the full Windows build, compiled executable name/version, all 11 shipped dialog controls and their bounds, portable ZIP CRCs/checksums, default-only icons, extracted archive operations, and test install/uninstall with association tasks disabled passed. The installed executable reports exactly ExcellentZip and version2026.10.27.2. Windows confirmations and Explorer behavior have not been tested live; older Windows may open the general Default apps screen. Builds are unsigned.

## [2026-10-27.1](https://github.com/abelokoj/ExcellentZip/releases/tag/2026-10-27.1)

Recorded 2026-10-27T02:09:30-04:00. Publication confirmed 2026-10-27T02:44:04-04:00 (recording time). The Windows x64 installer is 13.0 MB and portable ZIP is 24.4 MB; both include only the default icon pack.

- Moves all icon artwork and downloadable packs to the repository's icons/ directory.
- Bundles only Monogram EZ (pack 4). Selecting an unavailable optional pack asks before downloading all six additional packs from GitHub, with progress and cancellation. Declining or failing leaves the selected pack unchanged.
- Checks the download's SHA-256 and all 384 optional ICO files before installing into a per-user cache. Downloaded packs remain available offline and across app upgrades.
- Removes obsolete bundled IconPacks files during Setup upgrades while preserving downloaded caches and Windows default-opener choices.
- Renames source directories to Codecs, Core, Modern, Project, Specification, UI_Modern, Universal, ZS_Codecs, and Package; updates build paths and documentation.

Validation: 10 packaging tests, eight asset tests, two source-layout tests, and native icon/download checks passed. Native checks cover consent refusal, cancellation, corrupted downloads, safe installation/repair, association ownership, and compilation of Settings controls/resources. Validation update 2026-10-27T02:44:04-04:00 (recording time): the full Windows build, live GitHub download of all six packs (38,251,555 bytes), transfer cancellation, ZIP checksums/CRCs, and an isolated installer upgrade from all 448 old icons to 64 default icons passed. Installed ZIP/encrypted-7Z operations and uninstall also passed. Live Explorer/UI testing remains pending. Builds are unsigned.

## [2026-10-26.1](https://github.com/abelokoj/ExcellentZip/releases/tag/2026-10-26.1)

Recorded 2026-10-27T00:52:39-04:00. Release publication confirmed 2026-10-27T01:16:34-04:00 (recording time). Windows x64 installer, portable ZIP, checksums, and an artifact manifest are available on GitHub Releases.

- Introduces ExcellentZip branding and seven bundled icon packs. Monogram EZ (pack 4) is the default.
- Gives each of the 62 registered file types a distinct color and readable format label, consistent across all packs.
- Adds icon-pack selection and previews in Tools → Options → Settings, with updates limited to ExcellentZip-owned file associations.
- Uses `YYYY-MM-DD.revision` app versions; `2026-10-26.1` maps to Windows version `2026.10.26.1`.
- Adds per-user installer and portable packaging, checksums, and automated Windows x64 builds.
- Presents all seven icon packs separately in the README and retains third-party source licenses and credits.
- Removes the unused classic frontend, legacy translation migrator, obsolete custom build tasks, duplicate frontend project, and retired build/installer scripts. The supported release build uses one packaging path.
- Packages the complete built app and all 448 icons, omitting compiler symbols, build metadata, and private folders from both downloads.
- Reduces package artwork to Windows image-size limits. Private development notes are excluded from source and release packages.

Validation update 2026-10-27T01:25:42-04:00 (recording time): the [GitHub Actions Windows x64 build](https://github.com/abelokoj/ExcellentZip/actions/runs/40453541255) completed successfully and uploaded the installer, portable ZIP, checksums, and manifest as build artifacts.

Validation updated 2026-10-27T01:06:58-04:00 (recording time): the full Windows x64 package build, six asset checks, five packaging checks, and ZIP/encrypted-7Z creation, integrity, and extraction checks passed. Native icon checks, portable extraction checks, and a per-user test install/uninstall with association tasks disabled also passed. Portable GUI startup was checked without exercising its controls. Live Windows Explorer testing remains pending. Builds are unsigned.
