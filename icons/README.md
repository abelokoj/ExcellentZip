# ExcellentZip icon packs

Seven approved dimensional icon families are retained here as source artwork. Only Monogram EZ (pack 4), the default, is bundled with the app. Settings offers the six optional packs together after download consent and caches them per Windows user for offline use. Color Tabs (pack 7) uses a glossy coral archive case with a handle, gold latch, reinforced corners and an archive label.

Each registered file extension has its own stable accent color in all seven packs. Text identifies the format independently of color. Sixty-two distinct color values cannot all be reliably distinguished by color alone.

The actual app and page materials come from the approved high-resolution transparent masters in `Masters/`. These preserve the dimensional gloss, bevels, metal zippers and shadows. The generator composes per-format colors, corner folds, labels and badges over those masters. Row 2 uses a teal page, white ExZ and a white zipper badge; row 3 uses gold, dark ExZ and a format-colored parcel badge. Rows 4 and 5 use silver vertical zippers; row 6 uses a gold zipper. Row 4 has the approved colored horizontal label bar.

Every ICO has separately composed 16, 20, 24, 32, 40, 48, 64, 96, 128 and 256 px frames. Native 16/20 px layouts give the format label extra room while retaining the dimensional materials. `Packs/` SVG sources reference reusable PNG masters instead of embedding repeated copies. Compiled ICOs and package PNGs are self-contained and need no SVG renderer at runtime.

Regenerate from the repository root with `python scripts/generate_icons.py`. Development needs Python 3.9+, Pillow, CairoSVG, and Windows Arial/Segoe UI fonts. Run `python scripts/test_icon_assets.py` to check extension coverage, unique colors, all frame resolutions, hashes and package images. The package manifest supplies the extension list. `package-layout.json` preserves required package filename/dimension rules without using legacy artwork.

Artwork and generator: MIT, copyright 2026 ExcellentZip contributors. Master images were created expressly for ExcellentZip with the built-in image generation tool from the approved design reference. Fonts are used to render text and are not redistributed. Application and third-party components retain their original licenses.

## Optional download

`python scripts/generate_icon_download.py` packages the existing ICOs without rendering new artwork or requiring image libraries. It generates a reproducible `optional-packs-<first16-sha256>.zip`, `optional-packs.json`, and the native download catalog. The ZIP contains 384 ICOs under `<pack-folder>/<stem>.ico`, with no enclosing directory. Entries are sorted and use fixed timestamps, permissions, and stored compression because the frames already contain compressed PNGs. The compiled archive and file hashes authenticate downloads.

Run `python scripts/generate_icon_download.py --check` to check all generated outputs. Keep previous content-addressed archives here when artwork changes, because released apps refer to their original archive name. The full icon generator invokes this packaging step too. Runtime bundle paths use `icons/4-monogram-ez`; repository sources retain `icons/Packs/<folder>` for all seven styles.
