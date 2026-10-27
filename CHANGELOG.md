# Changelog

## Unreleased — 2026-10-26.1

Recorded 2026-10-27T00:52:39-04:00. Source changes are available; installer and portable downloads are being prepared.

- Introduces ExcellentZip branding and seven bundled icon packs. Monogram EZ (pack 4) is the default.
- Gives each of the 62 registered file types a distinct color and readable format label, consistent across all packs.
- Adds icon-pack selection and previews in Tools → Options → Settings, with updates limited to ExcellentZip-owned file associations.
- Uses `YYYY-MM-DD.revision` app versions; `2026-10-26.1` maps to Windows version `2026.10.26.1`.
- Adds per-user installer and portable packaging, checksums, and automated Windows x64 builds.
- Presents all seven icon packs separately in the README and retains third-party source licenses and credits.
- Removes the unused classic frontend, legacy translation migrator, obsolete custom build tasks, duplicate frontend project, and retired build/installer scripts. The supported release build uses one packaging path.
- Reduces package artwork to Windows image-size limits. Private development notes are excluded from source and release packages.

Asset, packaging, and native icon checks have passed. Full distribution build and live Windows Explorer testing remain pending. Builds are unsigned.
