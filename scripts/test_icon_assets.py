"""Validate bundled icon coverage, colors, frame sizes and package consistency."""
import hashlib
import json
from pathlib import Path
import unittest
import xml.etree.ElementTree as ET

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / 'Assets/ExcellentZip'


class IconAssetsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads((ASSETS / 'manifest.json').read_text(encoding='utf-8'))

    def test_every_association_has_a_unique_color(self):
        registered = {node.text.removeprefix('.') for node in ET.parse(next(ROOT.glob('*Package/Package.appxmanifest'))).iter()
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
