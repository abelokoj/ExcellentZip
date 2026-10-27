# Changelog

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
