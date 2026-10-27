# ExcellentZip release notes

## 2026-10-27.2

- Uses ExcellentZip as the Windows app-picker name.
- Adds a File types tab with current defaults, individual checkboxes, Select all/Clear, and guided Windows confirmation for selected extensions.
- Adds an optional Setup step to choose ExcellentZip for supported file types through Windows Settings.
- Refreshes associated icons and previews using the saved icon pack after default-app changes.

## 2026-10-27.1

- Smaller app downloads bundle Monogram EZ only, with its complete set of 64 ICOs and Windows package images.
- Settings offers one consent-based download for the six other icon packs, then caches them per Windows user for offline use.
- Optional downloads use an immutable archive name and compiled SHA-256 checks for the archive and all 384 icons.
- Existing icon artwork, all seven styles, and the 62 distinct file-type colors are preserved.
- Source projects use plain directory names and icon artwork lives in root `icons/`.
- Setup upgrades remove the obsolete app-owned `IconPacks` folder; the per-user download cache persists through uninstall.
- Date/revision versioning continues with a new revision; published 2026-10-26.1 downloads remain available.

## 2026-10-26.1

- ExcellentZip branding, project names, executable names, and Windows package identity.
- Seven approved dimensional icon packs, with Monogram EZ as default.
- Distinct colors and readable labels for 62 registered file types.
- Icon-pack previews and selection in Tools → Options → Settings.
- Per-user Setup and extract-and-run portable packaging.
- Automated Windows x64 builds and downloadable build artifacts through GitHub Actions.

Validation and publication status are recorded in the root CHANGELOG.md. Original source credits and third-party terms are preserved in License.md and THIRD_PARTY_NOTICES.md.
