"""Build the immutable optional-icon ZIP and its authenticated native catalog."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def download_artifacts(root=ROOT):
    catalog = json.loads((root / 'icons/manifest.json').read_text(encoding='utf-8'))
    files = []
    output = io.BytesIO()
    with zipfile.ZipFile(output, 'w', compression=zipfile.ZIP_STORED) as archive:
        for pack in sorted(catalog['packs'], key=lambda item: item['folder']):
            if pack['id'] == catalog['defaultPack']:
                continue
            if not re.fullmatch(r'[1-7]-[a-z0-9-]+', pack['folder']):
                raise ValueError('Invalid icon pack folder')
            for stem in sorted(['app', 'archive', *catalog['extensions']]):
                if not re.fullmatch(r'[a-z0-9]+', stem):
                    raise ValueError('Invalid icon stem')
                name = f"{pack['folder']}/{stem}.ico"
                data = (root / 'icons/Packs' / name).read_bytes()
                entry = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
                entry.create_system = 3
                entry.external_attr = 0o100644 << 16
                archive.writestr(entry, data)
                files.append({'pack': pack['id'], 'stem': stem, 'path': name,
                              'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()})
    data = output.getvalue()
    digest = hashlib.sha256(data).hexdigest()
    set_id = digest[:16]
    name = f'optional-packs-{set_id}.zip'
    metadata = {'version': 1, 'setId': set_id,
                'archive': {'name': name, 'sha256': digest, 'bytes': len(data)}, 'files': files}
    header = ('#pragma once\n\nnamespace ExcellentZipIcons {\n'
              f'inline constexpr wchar_t kOptionalSetId[] = L"{set_id}";\n'
              f'inline constexpr wchar_t kOptionalArchiveName[] = L"{name}";\n'
              f'inline constexpr wchar_t kOptionalArchiveSha256[] = L"{digest}";\n'
              f'inline constexpr unsigned long long kOptionalArchiveBytes = {len(data)};\n'
              'struct OptionalIconFile { int Pack; const wchar_t* Stem; unsigned long long Bytes; const wchar_t* Sha256; };\n'
              'inline constexpr OptionalIconFile kOptionalFiles[] = {\n')
    header += ''.join(f'    {{{file["pack"]}, L"{file["stem"]}", {file["bytes"]}, L"{file["sha256"]}"}},\n'
                      for file in files)
    header += '};\n}\n'
    return data, metadata, header


def generate(root=ROOT, check=False):
    data, metadata, header = download_artifacts(root)
    outputs = {
        root / 'icons' / metadata['archive']['name']: data,
        root / 'icons/optional-packs.json': (json.dumps(metadata, indent=2) + '\n').encode(),
        root / 'UI_Modern/ExcellentZip.IconDownloadCatalog.h': header.encode(),
    }
    for path, content in outputs.items():
        if check:
            if not path.is_file() or path.read_bytes() != content:
                raise ValueError(f'Generated optional icon download is stale or missing: {path}')
        elif not path.is_file() or path.read_bytes() != content:
            path.write_bytes(content)
    print(json.dumps(metadata['archive']))
    return metadata


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    generate(check=parser.parse_args().check)
