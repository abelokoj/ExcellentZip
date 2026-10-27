"""Package a built ExcellentZip payload; refuses missing executable/dependency/assets."""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import re
import shutil
import subprocess
import tempfile
import zipfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]


def current_version():
    return ET.parse(ROOT / 'Project/ExcellentZip.Project.Version.props').findtext(
        './/{*}ExcellentZipDisplayVersion')

def version_numbers(version):
    match = re.fullmatch(r'(\d{4}-\d{2}-\d{2})\.([1-9]\d*)', version)
    if not match:
        raise ValueError('Use an app version like 2026-10-27.1')
    day = datetime.date.fromisoformat(match[1])
    revision = int(match[2])
    if revision > 65535:
        raise ValueError('Windows revision must not exceed 65535')
    return f'{day.year}.{day.month}.{day.day}.{revision}'

def associations(catalog):
    lines = []
    for extension in catalog['extensions']:
        if not re.fullmatch(r'[a-z0-9]+', extension):
            raise ValueError('Invalid file extension in icon catalog')
        progid = 'ExcellentZip.Archive.' + extension
        icon = '""{app}\\icons\\4-monogram-ez\\' + extension + '.ico"",0'
        command = '""{app}\\ExcellentZip.exe"" ""%1"" -open'
        lines.extend([
            f'Root: HKCU; Subkey: "Software\\Classes\\{progid}"; ValueType: string; ValueData: "ExcellentZip {extension.upper()} archive"; Flags: uninsdeletekey; Tasks: fileassociations',
            f'Root: HKCU; Subkey: "Software\\Classes\\{progid}\\DefaultIcon"; ValueType: string; ValueData: "{icon}"; Tasks: fileassociations',
            f'Root: HKCU; Subkey: "Software\\Classes\\{progid}\\shell\\open\\command"; ValueType: string; ValueData: "{command}"; Tasks: fileassociations',
            f'Root: HKCU; Subkey: "Software\\Classes\\.{extension}\\OpenWithProgids"; ValueType: none; ValueName: "{progid}"; Flags: uninsdeletevalue; Tasks: fileassociations',
            f'Root: HKCU; Subkey: "Software\\ExcellentZip\\Capabilities\\FileAssociations"; ValueType: string; ValueName: ".{extension}"; ValueData: "{progid}"; Tasks: fileassociations',
        ])
    return '\n'.join(lines) + '\n'

def copy_runtime_payload(source, destination):
    catalog = json.loads((ROOT / 'icons/manifest.json').read_text(encoding='utf-8'))
    default = next(pack for pack in catalog['packs'] if pack['id'] == catalog['defaultPack'])
    folder = Path('icons') / default['folder']
    icons = [folder / f'{stem}.ico' for stem in ['app', 'archive', *catalog['extensions']]]
    for path in icons:
        if not (source / path).is_file():
            raise FileNotFoundError(f'Missing default icon: {source / path}')
    # Copy the catalog's default icons explicitly so old payloads cannot bundle optional packs.
    shutil.copytree(source, destination, dirs_exist_ok=True,
        ignore=shutil.ignore_patterns('*.pdb', '*.lib', '*.exp', '*.ilk', '*.obj', '*.ico',
                                     '*.appxrecipe', 'AppxManifest.xml', 'AppxBlockMap.xml', 'AppxSignature.p7x',
                                     '[[]Content_Types].xml', 'AppxMetadata', 'dev_notes', '.tools', '.git',
                                     'IconPacks', 'icons'))
    (destination / folder).mkdir(parents=True, exist_ok=True)
    for path in icons:
        shutil.copy2(source / path, destination / path)


def package(payload, compiler, version):
    numeric = version_numbers(version)
    catalog = json.loads((ROOT / 'icons/manifest.json').read_text(encoding='utf-8'))
    payload = payload.resolve()
    for name in ['ExcellentZip.exe', 'ExcellentZipC.exe', 'ExcellentZipG.exe', 'ExcellentZip.Modern.dll',
                 'ExcellentZip.Core.dll', 'ExcellentZip.Codecs.dll', 'ExcellentZip.ZS.Codecs.dll',
                 'K7Base.dll', 'K7User.dll', 'resources.pri']:
        if not (payload / name).is_file():
            raise FileNotFoundError(f'Incomplete release payload: {name}')
    with tempfile.TemporaryDirectory(prefix='excellentzip-release-') as directory:
        staged = Path(directory)
        copy_runtime_payload(payload, staged)
        payload = staged
        dist = ROOT / 'dist'
        dist.mkdir(exist_ok=True)
        for name in ['License.md', 'THIRD_PARTY_NOTICES.md', 'ReadMe.md']:
            shutil.copy2(ROOT / name, payload / name)
        (payload / 'PORTABLE.txt').write_text(
            f'ExcellentZip {version}\n\nExtract this entire folder and run ExcellentZip.exe.\n'
            'No installer or administrator rights are required. Keep the icons folder and all DLLs beside the app.\n'
            'Settings are stored per Windows user. Moving the folder does not erase those preferences.\n'
            'Monogram EZ is included. Settings can download the six optional packs together with your consent.\n'
            'Downloaded packs are cached per Windows user. Selecting a pack does not change default apps.\n'
            'Use the Setup download to register ExcellentZip file types in Windows Default apps.\n', encoding='utf-8')
        archive = dist / f'ExcellentZip-{version}-Portable-x64.zip'
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as output:
            for path in sorted(payload.rglob('*')):
                if path.is_file():
                    output.write(path, Path('ExcellentZip') / path.relative_to(payload))
        (ROOT / 'packaging/ExcellentZip.Associations.iss').write_text(associations(catalog), encoding='utf-8')
        subprocess.run([str(compiler), '/Q', f'/DAppVersion={version}', f'/DWindowsVersion={numeric}',
                        f'/DPayloadDir={payload}', f'/DReleaseDir={dist}',
                        str(ROOT / 'packaging/ExcellentZip.Setup.iss')], check=True)
        installer = dist / f'ExcellentZip-{version}-Setup-x64.exe'
        if not installer.is_file():
            raise FileNotFoundError('Installer compiler did not produce the expected output')
        artifacts = []
        for path in [archive, installer]:
            with path.open('rb') as stream:
                digest = hashlib.file_digest(stream, 'sha256').hexdigest()
            artifacts.append({'name': path.name, 'bytes': path.stat().st_size, 'sha256': digest})
        (dist / f'ExcellentZip-{version}-SHA256.txt').write_text(
            ''.join(f"{entry['sha256']}  {entry['name']}\n" for entry in artifacts), encoding='utf-8')
        (dist / f'ExcellentZip-{version}-manifest.json').write_text(json.dumps(
            {'appVersion': version, 'windowsVersion': numeric, 'architecture': 'x64', 'artifacts': artifacts}, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(artifacts, indent=2))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--payload', required=True, type=Path)
    parser.add_argument('--compiler', required=True, type=Path)
    parser.add_argument('--version', default=current_version())
    arguments = parser.parse_args()
    package(arguments.payload, arguments.compiler, arguments.version)
