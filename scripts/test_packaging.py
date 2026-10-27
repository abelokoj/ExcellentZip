import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('package_release', Path(__file__).with_name('package_release.py'))
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)

class PackagingTests(unittest.TestCase):
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
            release.copy_runtime_payload(source, destination)
            self.assertEqual({'ExcellentZip.exe', 'resources.pri', 'IconPacks'},
                             {path.name for path in destination.iterdir()})
            self.assertTrue((destination / 'IconPacks/sample.txt').is_file())

    def test_incomplete_payload_rejected(self):
        with tempfile.TemporaryDirectory() as directory, self.assertRaises(FileNotFoundError):
            release.package(Path(directory), Path('not-run.exe'), '2026-10-26.1')

if __name__ == '__main__':
    unittest.main()
