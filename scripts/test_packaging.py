import importlib.util
import json
import zipfile
from unittest.mock import patch
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('package_release', Path(__file__).with_name('package_release.py'))
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)

class PackagingTests(unittest.TestCase):
    def test_default_version_comes_from_project_properties(self):
        self.assertRegex(release.current_version(), r'^\d{4}-\d{2}-\d{2}\.[1-9]\d*$')

    def test_app_and_windows_version_mapping(self):
        self.assertEqual('2026.10.26.1', release.version_numbers('2026-10-26.1'))
        self.assertEqual('2028.2.29.65535', release.version_numbers('2028-02-29.65535'))

    def test_invalid_versions_rejected(self):
        for value in ['1.0.0', '2026-02-29.1', '2026-10-26.0', '2026-10-26.65536', '../2026-10-26.1']:
            with self.subTest(value=value), self.assertRaises(ValueError):
                release.version_numbers(value)

    def test_registration_only_adds_own_openwith_capabilities(self):
        text = release.associations({'extensions': ['zip', '7z']})
        self.assertEqual(2, text.count('\\shell\\open\\command'))
        self.assertIn('ExcellentZip.Archive.zip', text)
        self.assertIn('\\.zip\\OpenWithProgids', text)
        self.assertIn('\\icons\\4-monogram-ez\\zip.ico', text)
        self.assertNotIn('UserChoice', text)
        self.assertNotIn('Subkey: "Software\\Classes\\.zip";', text)

    def test_distribution_excludes_private_notes_and_build_metadata(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, destination = root / 'source', root / 'distribution'
            source.mkdir()
            for name in ['ExcellentZip.exe', 'resources.pri', 'ExcellentZip.pdb', 'package.appxrecipe',
                         'AppxManifest.xml', 'AppxBlockMap.xml', '[Content_Types].xml']:
                (source / name).write_text('fixture')
            for name in ['dev_notes', '.tools', '.git', 'IconPacks']:
                (source / name).mkdir()
                (source / name / 'sample.txt').write_text('fixture')
            catalog = json.loads((release.ROOT / 'icons/manifest.json').read_text(encoding='utf-8'))
            default = source / 'icons/4-monogram-ez'
            default.mkdir(parents=True)
            for stem in ['app', 'archive', *catalog['extensions']]:
                (default / f'{stem}.ico').write_bytes(b'icon fixture')
            (default / 'unused.ico').write_bytes(b'extra icon')
            optional = source / 'icons/1-fluent-zip'
            optional.mkdir()
            (optional / 'app.ico').write_bytes(b'optional icon')
            (source / 'other.ico').write_bytes(b'old icon')
            (source / 'icons/optional-packs-fixture.zip').write_bytes(b'optional archive')
            (source / 'icons/optional-packs.json').write_text('{}', encoding='utf-8')
            release.copy_runtime_payload(source, destination)
            self.assertEqual({'ExcellentZip.exe', 'resources.pri', 'icons'},
                             {path.name for path in destination.iterdir()})
            self.assertEqual({'4-monogram-ez'}, {path.name for path in (destination / 'icons').iterdir()})
            self.assertEqual(64, len(list(destination.rglob('*.ico'))))

    def test_portable_contains_only_the_complete_default_pack(self):
        catalog = json.loads((release.ROOT / 'icons/manifest.json').read_text(encoding='utf-8'))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            payload = root / 'payload'
            payload.mkdir()
            for name in ['ExcellentZip.exe', 'ExcellentZipC.exe', 'ExcellentZipG.exe',
                         'ExcellentZip.Modern.dll', 'ExcellentZip.Core.dll', 'ExcellentZip.Codecs.dll',
                         'ExcellentZip.ZS.Codecs.dll', 'K7Base.dll', 'K7User.dll', 'resources.pri']:
                (payload / name).write_bytes(b'fixture')
            for pack in catalog['packs']:
                folder = payload / 'icons' / pack['folder']
                folder.mkdir(parents=True)
                for stem in ['app', 'archive', *catalog['extensions']]:
                    (folder / f'{stem}.ico').write_bytes(b'icon fixture')
            legacy = payload / 'IconPacks/1-fluent-zip'
            legacy.mkdir(parents=True)
            (legacy / 'app.ico').write_bytes(b'legacy icon')
            (root / 'icons').mkdir()
            (root / 'icons/manifest.json').write_text(json.dumps(catalog), encoding='utf-8')
            (root / 'packaging').mkdir()
            for name in ['License.md', 'THIRD_PARTY_NOTICES.md', 'ReadMe.md']:
                (root / name).write_text('fixture', encoding='utf-8')
            def compile_fixture(*args, **kwargs):
                (root / 'dist/ExcellentZip-2026-10-27.1-Setup-x64.exe').write_bytes(b'installer fixture')
            with patch.object(release, 'ROOT', root), patch.object(release.subprocess, 'run', side_effect=compile_fixture), patch('builtins.print'):
                release.package(payload, Path('fixture.exe'), '2026-10-27.1')
            with zipfile.ZipFile(root / 'dist/ExcellentZip-2026-10-27.1-Portable-x64.zip') as archive:
                icons = [name for name in archive.namelist() if name.endswith('.ico')]
                self.assertEqual(64, len(icons))
                self.assertTrue(all(name.startswith('ExcellentZip/icons/4-monogram-ez/') for name in icons))
                self.assertFalse(any('IconPacks' in name for name in archive.namelist()))

    def test_default_pack_is_required_but_optional_packs_are_not(self):
        catalog = json.loads((release.ROOT / 'icons/manifest.json').read_text(encoding='utf-8'))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            default = root / 'payload/icons/4-monogram-ez'
            default.mkdir(parents=True)
            for stem in ['app', 'archive', *catalog['extensions']]:
                (default / f'{stem}.ico').write_bytes(b'icon fixture')
            release.copy_runtime_payload(root / 'payload', root / 'complete')
            (default / 'zip.ico').unlink()
            with self.assertRaises(FileNotFoundError):
                release.copy_runtime_payload(root / 'payload', root / 'incomplete')

    def test_windows_package_includes_only_default_runtime_icons(self):
        import xml.etree.ElementTree as ET
        project = ET.parse(release.ROOT / 'Package/ExcellentZipPackage.wapproj')
        items = [item for item in project.iter() if item.tag.endswith('}Content')
                 and item.get('Include', '').endswith('.ico')]
        self.assertEqual(1, len(items))
        self.assertEqual(r'..\icons\Packs\4-monogram-ez\*.ico', items[0].get('Include'))
        self.assertEqual(r'icons\4-monogram-ez\%(FileName)%(Extension)', items[0].findtext('{*}Link'))

    def test_installer_upgrade_removes_only_legacy_app_icons(self):
        setup = (release.ROOT / 'packaging/ExcellentZip.Setup.iss').read_text(encoding='utf-8')
        deletes = setup.split('[InstallDelete]\n', 1)[1].split('\n[', 1)[0].strip()
        self.assertEqual('Type: filesandordirs; Name: "{app}\\IconPacks"', deletes)
        self.assertNotIn('[UninstallDelete]', setup)
        self.assertNotIn('UserChoice', setup)

    def test_incomplete_payload_rejected(self):
        with tempfile.TemporaryDirectory() as directory, self.assertRaises(FileNotFoundError):
            release.package(Path(directory), Path('not-run.exe'), '2026-10-26.1')

if __name__ == '__main__':
    unittest.main()
