"""Upload an already verified ExcellentZip release, publishing only after all assets upload."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import urllib.error
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
REPOSITORY = 'abelokoj/ExcellentZip'

def credential():
    token = os.environ.get('GITHUB_TOKEN')
    if token:
        return token
    result = subprocess.run(['git', 'credential', 'fill'],
        input='protocol=https\nhost=github.com\n\n', text=True, capture_output=True, check=True)
    values = dict(line.split('=', 1) for line in result.stdout.splitlines() if '=' in line)
    if not values.get('password'):
        raise RuntimeError('Sign in to GitHub with Git Credential Manager first.')
    return values['password']

def request(token, url, method='GET', data=None, binary=False):
    headers = {'Authorization': 'Bearer ' + token, 'User-Agent': 'ExcellentZip-release',
               'Accept': 'application/vnd.github+json'}
    body = data if binary else json.dumps(data).encode() if data is not None else None
    if body is not None:
        headers['Content-Type'] = 'application/octet-stream' if binary else 'application/json'
    with urllib.request.urlopen(urllib.request.Request(url, data=body, headers=headers, method=method), timeout=300) as response:
        return json.load(response)

def publish(version):
    from package_release import version_numbers
    version_numbers(version)
    names = [f'ExcellentZip-{version}-Setup-x64.exe', f'ExcellentZip-{version}-Portable-x64.zip',
             f'ExcellentZip-{version}-SHA256.txt', f'ExcellentZip-{version}-manifest.json']
    files = [ROOT / 'dist' / name for name in names]
    if any(not file.is_file() for file in files):
        raise FileNotFoundError('Build the installer, portable ZIP and checksum files before publishing.')
    token = credential()
    api = f'https://api.github.com/repos/{REPOSITORY}/releases'
    try:
        existing = request(token, api + '/tags/' + urllib.parse.quote(version, safe=''))
    except urllib.error.HTTPError as error:
        if error.code != 404:
            raise
    else:
        raise RuntimeError('This version already has a release; choose a new app revision to preserve existing downloads: ' + existing['html_url'])
    head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    body = f'''ExcellentZip {version} for Windows x64.

- Monogram EZ is included; Settings downloads all six optional packs together after consent.
- 62 registered file types with distinct colors and readable labels.
- Windows shows the app as ExcellentZip. Setup can open default-app confirmation; Tools > Options > File types supports all or selected extensions with Windows confirmation.
- The portable ZIP runs after extraction. Keep all DLLs, resources and the icons folder together; preferences are stored per Windows user.

Download **Setup-x64.exe** to install, or **Portable-x64.zip** to extract and run. SHA-256 checksums are included.

These binaries are unsigned; Windows may request confirmation before running them.
'''
    release = request(token, api, 'POST', {'tag_name': version, 'target_commitish': head,
        'name': 'ExcellentZip ' + version, 'body': body, 'draft': True, 'prerelease': False})
    upload = release['upload_url'].split('{', 1)[0]
    for file in files:
        request(token, upload + '?name=' + urllib.parse.quote(file.name), 'POST', file.read_bytes(), True)
        print('Uploaded ' + file.name, flush=True)
    released = request(token, api + '/' + str(release['id']), 'PATCH', {'draft': False})
    print(released['html_url'])

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    from package_release import current_version
    parser.add_argument('--version', default=current_version())
    publish(parser.parse_args().version)
