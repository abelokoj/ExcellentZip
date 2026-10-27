# Changelog

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
