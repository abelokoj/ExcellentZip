"""Exercise the built archive engine without installation or file-association changes."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile


def smoke(payload, version):
    root = Path(__file__).resolve().parents[1]
    catalog = json.loads((root / 'icons/manifest.json').read_text(encoding='utf-8'))
    expected = {Path('icons/4-monogram-ez') / f'{stem}.ico'
                for stem in ['app', 'archive', *catalog['extensions']]}
    actual = {path.relative_to(payload) for path in payload.rglob('*.ico')}
    if actual != expected or (payload / 'IconPacks').exists():
        raise AssertionError('Release payload must contain only the 64 bundled Monogram EZ icons.')
    executable = (payload / 'ExcellentZipC.exe').resolve()
    def run(*arguments, cwd=None):
        result = subprocess.run([str(executable), *arguments], cwd=cwd,
                                capture_output=True, text=True, errors='replace', timeout=90)
        if result.returncode:
            raise RuntimeError(f'Archive command failed ({result.returncode}): {result.stdout} {result.stderr}')
        return result.stdout

    banner = run('i')
    if f'ExcellentZip {version}' not in banner:
        raise AssertionError('The executable banner does not match the release version.')
    with tempfile.TemporaryDirectory(prefix='excellentzip-smoke-') as directory:
        root = Path(directory)
        source = root / 'source'
        source.mkdir()
        fixtures = {'example.txt': b'ExcellentZip archive round trip\n' * 100,
                    'unicode-例.txt': 'Archive text: é 例\n'.encode('utf-8'),
                    'binary.dat': bytes(range(256)) * 64}
        for name, data in fixtures.items():
            (source / name).write_bytes(data)
        for extension, options in [('zip', []), ('7z', ['-pExcellentZip-test-only', '-mhe=on'])]:
            archive = root / f'roundtrip.{extension}'
            run('a', f'-t{extension}', str(archive), *fixtures, *options, cwd=source)
            run('t', str(archive), '-pExcellentZip-test-only')
            output = root / extension
            run('x', str(archive), f'-o{output}', '-y', '-pExcellentZip-test-only')
            for name, data in fixtures.items():
                if (output / name).read_bytes() != data:
                    raise AssertionError(f'Archive round trip changed {name}')
    print(f'ExcellentZip {version}: ZIP and encrypted 7Z create, test, and extract checks passed.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--payload', required=True, type=Path)
    parser.add_argument('--version', required=True)
    args = parser.parse_args()
    smoke(args.payload, args.version)
