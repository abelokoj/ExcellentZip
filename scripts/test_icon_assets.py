"""Validate source icons, optional downloads, colors, frames and package consistency."""
import hashlib
import io
import tempfile
import zipfile
from generate_icon_download import download_artifacts, generate
import json
from pathlib import Path
import unittest
import xml.etree.ElementTree as ET

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / 'icons'


class IconAssetsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads((ASSETS / 'manifest.json').read_text(encoding='utf-8'))

    def test_every_association_has_a_unique_color(self):
        registered = {node.text.removeprefix('.') for node in ET.parse(ROOT / 'Package/Package.appxmanifest').iter()
                      if node.tag.endswith('}FileType') and node.text and node.text.startswith('.')}
        self.assertEqual(registered, set(self.catalog['extensions']))
        self.assertEqual(registered, set(self.catalog['colors']))
        self.assertEqual(len(registered), len(set(self.catalog['colors'].values())))

    def test_seven_complete_packs_with_native_sizes_and_distinct_art(self):
        self.assertEqual(4, self.catalog['defaultPack'])
        self.assertEqual(list(range(1, 8)), [pack['id'] for pack in self.catalog['packs']])
        expected_sizes = {(size, size) for size in self.catalog['sizes']}
        app_hashes = set()
        for pack in self.catalog['packs']:
            directory = ASSETS / 'Packs' / pack['folder']
            app_hashes.add(hashlib.sha256((directory / 'app.ico').read_bytes()).hexdigest())
            for extension in ['app', 'archive', *self.catalog['extensions']]:
                with self.subTest(pack=pack['id'], extension=extension):
                    with Image.open(directory / f'{extension}.ico') as image:
                        self.assertEqual(expected_sizes, image.ico.sizes())
                        for size in expected_sizes:
                            frame = image.ico.getimage(size)
                            self.assertEqual(size, frame.size)
                            self.assertIsNotNone(frame.getbbox())
                    ET.parse(directory / f'{extension}.svg')
        self.assertEqual(7, len(app_hashes))

    def test_dimensional_master_dependencies_are_present(self):
        for pack in range(1, 8):
            for part in ('app', 'page'):
                with self.subTest(pack=pack, part=part):
                    with Image.open(ASSETS / 'Masters' / f'{pack}-{part}.png') as master:
                        self.assertEqual('RGBA', master.mode)
                        self.assertGreater(min(master.size), 400)
                        self.assertLess(master.getchannel('A').getextrema()[0], 255)
                        self.assertIsNone(master.convert('RGB').getcolors(4096))

    def test_icon_hashes_and_default_package_logos(self):
        for record in self.catalog['icons']:
            folder = self.catalog['packs'][record['pack']-1]['folder']
            data = (ASSETS / 'Packs' / folder / (record['extension'] + '.ico')).read_bytes()
            self.assertEqual(record['sha256'], hashlib.sha256(data).hexdigest())
            if record['pack'] == 4:
                with Image.open(ASSETS / 'Packs' / folder / (record['extension'] + '.ico')) as icon:
                    for size in self.catalog['sizes']:
                        with Image.open(ASSETS / 'Package/FileTypes' / f"{record['extension']}.targetsize-{size}.png") as png:
                            self.assertEqual(icon.ico.getimage((size, size)).tobytes(), png.convert('RGBA').tobytes())

    def test_optional_download_is_reproducible_complete_and_authenticated(self):
        data, metadata, header = download_artifacts(ROOT)
        self.assertEqual(metadata, json.loads((ASSETS / 'optional-packs.json').read_text(encoding='utf-8')))
        archive = metadata['archive']
        self.assertEqual(data, (ASSETS / archive['name']).read_bytes())
        self.assertEqual(archive['sha256'], hashlib.sha256(data).hexdigest())
        self.assertEqual(metadata['setId'], archive['sha256'][:16])
        self.assertEqual(archive['bytes'], len(data))
        self.assertEqual(header, (ROOT / 'UI_Modern/ExcellentZip.IconDownloadCatalog.h').read_text(encoding='utf-8'))
        self.assertEqual(384, len(metadata['files']))
        self.assertEqual({1, 2, 3, 5, 6, 7}, {file['pack'] for file in metadata['files']})
        with zipfile.ZipFile(io.BytesIO(data)) as download:
            self.assertEqual(sorted(download.namelist()), download.namelist())
            self.assertEqual([file['path'] for file in metadata['files']], download.namelist())
            for file in metadata['files']:
                content = download.read(file['path'])
                self.assertEqual(file['bytes'], len(content))
                self.assertEqual(file['sha256'], hashlib.sha256(content).hexdigest())
                self.assertEqual(content, (ASSETS / 'Packs' / file['path']).read_bytes())
                entry = download.getinfo(file['path'])
                self.assertEqual((1980, 1, 1, 0, 0, 0), entry.date_time)
                self.assertEqual(0o100644 << 16, entry.external_attr)
                self.assertEqual(3, entry.create_system)
                self.assertEqual(zipfile.ZIP_STORED, entry.compress_type)

    def test_optional_generator_preserves_older_archives(self):
        from unittest.mock import patch
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'icons').mkdir()
            (root / 'UI_Modern').mkdir()
            old = root / 'icons/optional-packs-previous.zip'
            old.write_bytes(b'previous immutable release')
            metadata = {'archive': {'name': 'optional-packs-new.zip'}}
            with patch('generate_icon_download.download_artifacts', return_value=(b'new', metadata, 'header')):
                with patch('builtins.print'):
                    generate(root)
            self.assertEqual(b'previous immutable release', old.read_bytes())
            self.assertEqual(b'new', (root / 'icons/optional-packs-new.zip').read_bytes())

    def test_windows_package_image_size_limit(self):
        for path in (ASSETS / 'Package').rglob('*.png'):
            self.assertLessEqual(path.stat().st_size, 204800, str(path))

    def test_package_qualifiers_preserve_required_dimensions(self):
        layout = json.loads((ASSETS / 'package-layout.json').read_text(encoding='utf-8'))
        self.assertGreater(len(layout), 300)
        for name, dimensions in layout.items():
            with Image.open(ASSETS / 'Package' / name) as actual:
                self.assertEqual(tuple(dimensions), actual.size)
                self.assertIsNotNone(actual.getbbox())


if __name__ == '__main__':
    unittest.main()
