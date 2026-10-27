# ExcellentZip development and release notes

ExcellentZip uses the approved seven-family icon artwork. The default is Monogram EZ (4). App versions use `YYYY-MM-DD.revision`; release 2026-10-26.1 maps to Windows numeric version 2026.10.26.1.

## Icons

The 62 registered file extensions each have a distinct accent color, consistent across all seven packs, plus a readable format label. Unknown/wildcard file types use a generic archive icon. Color supplements the label because 62 colors cannot be reliably distinguished by color alone.

Use Tools → Options → Settings → Default icon pack. Selecting a pack previews it; Apply saves it and Cancel discards unsaved choices. The active application window follows the selected app icon. Static Windows package/Start branding uses pack 4. Windows thumbnails can take precedence over file icons.

The Windows 11 22H2+ package declares a shell icon handler. Native Setup registers separate `ExcellentZip.Archive.<extension>` classes and Open With/Default apps capabilities without choosing default openers. Icon changes touch only ExcellentZip-owned classes whose open command matches the current executable, or the current package's associations. UserChoice is never rewritten.

`Assets/ExcellentZip/Masters` contains approved dimensional source artwork and provenance. `python scripts/generate_icons.py` composes labeled, colored file icons from these masters and generates ten-resolution ICOs, package logos, catalog hashes, the native header, and the preview. Development requires Pillow, CairoSVG, and the Windows fonts used for labels. These are not app runtime dependencies. `package-layout.json` records image qualifier dimensions independently of retired artwork.

## Checks

- `python scripts/test_icon_assets.py`: coverage, unique colors, native ICO frames, hashes, package consistency, and master references.
- `scripts\test_icon_runtime.cmd`: native icon/COM boundary and ownership tests plus compilation of the actual Settings page and resources. It does not change default apps or saved icon selection.
- `python scripts/test_packaging.py`: date/revision mapping, invalid versions, safe registration entries, incomplete-payload rejection, and private/debug-file exclusion.
- The release build also exercises ZIP and encrypted 7Z creation, integrity testing, and byte-for-byte extraction before packaging.

## Build and package

Use Visual Studio 2022+ with desktop and UWP C++ tools, DesktopBridge/MSIX packaging tools, Windows 11 SDK, and .NET SDK 9. `NuGet.Config` supplies the repository-local public package source. Root `ExcellentZip.slnx` and all project paths use the ExcellentZip name.

Run `scripts/Build-Release.ps1 -Version 2026-10-26.1`. It builds Release/x64, and packages both a per-user Setup executable and portable ZIP plus checksums. Inno Setup 6.4.2 is the pinned installer compiler. The local compiler can live in ignored `.tools`; the build script can restore the pinned tool package on a clean checkout.

The portable ZIP runs after extraction without installation; all DLLs, resources, and IconPacks must stay together. Preferences are stored per Windows user. Setup adds Start-menu access and optional file-type registration. File opener choices remain with Windows Default apps.

The package identity is ExcellentZip.Desktop, publisher CN=ExcellentZip.Dev. Local native installer/portable builds do not require trusting an MSIX certificate. A signed MSIX requires an appropriate trusted publisher certificate; none is installed automatically. Public binaries are unsigned unless the release explicitly says otherwise.

Use `python scripts/publish_release.py --version 2026-10-26.1` only after validating the built downloads and pushing the source commit. It uploads all required artifacts into a draft release and publishes after every upload succeeds. It reads existing GitHub credentials in memory without printing them.

All original source license terms and author credits are preserved in License.md and THIRD_PARTY_NOTICES.md. Original branding artwork, Store links, and sponsorship features are excluded from the product.
