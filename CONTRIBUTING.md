# Contributing to ExcellentZip

Use an issue to describe substantial changes before opening a pull request. Match the surrounding C++/WinRT and Win32 style, preserve third-party license notices, and avoid modifying shared `Mile.*` sources. New project code is contributed under the MIT License.

Before submitting, run `python scripts/generate_icon_download.py --check`, `python scripts/test_icon_assets.py`, `python scripts/test_packaging.py`, and `scripts\test_icon_runtime.cmd`. Use `scripts/Build-Release.ps1` for installer and portable packaging. Document user-visible changes and validation in `CHANGELOG.md`.

App versions follow `YYYY-MM-DD.revision`; the Windows numeric equivalent uses dots. The approved icon masters and generator are in `icons` and `scripts/generate_icons.py`.

App distributions contain only pack 4 in `icons/4-monogram-ez`. After changing approved artwork, regenerate the optional download and native catalog with `python scripts/generate_icon_download.py` (also run by the full icon generator). Commit its ZIP and JSON metadata with the generated header. Preserve older content-addressed ZIPs so released app downloads continue to work.
