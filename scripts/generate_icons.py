"""Build ExcellentZip icons from approved dimensional masters (Pillow, CairoSVG)."""
from __future__ import annotations

import base64
import colorsys
import hashlib
import io
import json
import re
import math
from functools import lru_cache
from pathlib import Path
import struct
import xml.etree.ElementTree as ET

import cairosvg
from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'icons'
MASTERS = OUT / 'Masters'
SIZES = (16, 20, 24, 32, 40, 48, 64, 96, 128, 256)
PACKS = (
    ('1-fluent-zip', 'Fluent Zip'), ('2-outlined-pages', 'Outlined Pages'),
    ('3-golden-parcel', 'Golden Parcel'), ('4-monogram-ez', 'Monogram EZ'),
    ('5-prism-stack', 'Prism Stack'), ('6-archive-seal', 'Archive Seal'),
    ('7-color-tabs', 'Color Tabs'),
)
PAGE = (49, 9, 193, 236)


def extract_masters() -> None:
    """Separate the approved transparent component pairs, keeping their actual materials."""
    for pack in range(1, 8):
        with Image.open(MASTERS / f'pair-{pack}.png') as source:
            source = source.convert('RGBA')
            for part, left, right in [('app', 0, source.width//2), ('page', source.width//2, source.width)]:
                image = source.crop((left, 0, right, source.height))
                alpha = image.getchannel('A')
                opaque = alpha.point(lambda value: 255 if value > 100 else 0)
                # Retain the large central object, removing detached generation speckles.
                ImageDraw.floodfill(opaque, (image.width//2, image.height//2), 128)
                component = opaque.point(lambda value: 255 if value == 128 else 0)
                if component.getbbox() is None:
                    raise ValueError(f'No central opaque component in pack {pack} {part}')
                alpha = ImageChops.multiply(alpha, component.filter(ImageFilter.MaxFilter(21)))
                image.putalpha(alpha)
                image.crop(alpha.getbbox()).save(MASTERS / f'{pack}-{part}.png')
    white_badge()


def white_badge() -> None:
    """The approved row-2 badge is white around the original teal zipper strip."""
    original = Image.open(MASTERS / '2-app.png').convert('RGBA')
    image = original.copy()
    px, src = image.load(), original.load()
    for y in range(image.height):
        for x in range(image.width):
            r, g, b, alpha = src[x, y]
            if not alpha:
                continue
            center = .39 < x/image.width < .60
            if not center:
                value = round(.19*r + .58*g + .23*b)
                value = min(255, 231 + round(value*.10))
                px[x, y] = (value, value, min(255, value+2), alpha)
    image.save(MASTERS / '2-badge.png')


def image_data(name: str) -> str:
    return '../../Masters/' + name


def embedded(name: str, x: float, y: float, width: float, height: float) -> str:
    return f'<image xlink:href="{image_data(name)}" x="{x}" y="{y}" width="{width}" height="{height}" preserveAspectRatio="none"/>'


def svg(body: str, color: str = '#006aff') -> str:
    r, g, b = (int(color[i:i+2], 16) for i in (1, 3, 5))
    light = '#' + ''.join(f'{min(255, int(v*.75+255*.25)):02x}' for v in (r,g,b))
    dark = '#' + ''.join(f'{int(v*.69):02x}' for v in (r,g,b))
    return ('<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="256" height="256" viewBox="0 0 256 256">'
            '<defs><linearGradient id="accent" x1="0" y1="0" x2=".6" y2="1">'
            f'<stop stop-color="{light}"/><stop offset=".16" stop-color="{color}"/>'
            f'<stop offset=".74" stop-color="{color}"/><stop offset="1" stop-color="{dark}"/>'
            '</linearGradient><linearGradient id="shine" x2="0" y2="1">'
            '<stop stop-color="#fff" stop-opacity=".55"/><stop offset=".5" stop-color="#fff" stop-opacity=".04"/>'
            '<stop offset="1" stop-color="#fff" stop-opacity="0"/></linearGradient></defs>' + body + '</svg>')


def app_svg(pack: int) -> str:
    return svg(embedded(f'{pack}-app.png', 12, 12, 232, 232))


def tinted_parcel(color: str) -> str:
    name = f'3-parcel-{color[1:]}.png'
    path = MASTERS / name
    if not path.exists():
        image = Image.open(MASTERS / '3-app.png').convert('RGBA')
        hue, _, saturation = colorsys.rgb_to_hls(*(int(color[i:i+2],16)/255 for i in (1,3,5)))
        pixels = image.load()
        for y in range(image.height):
            for x in range(image.width):
                r,g,b,a = pixels[x,y]
                if not a or .39 < x/image.width < .61:
                    continue
                h,l,s = colorsys.rgb_to_hls(r/255,g/255,b/255)
                if s > .25:
                    rr,gg,bb = colorsys.hls_to_rgb(hue,l,max(.65,saturation))
                    pixels[x,y] = (round(rr*255),round(gg*255),round(bb*255),a)
        image.save(path)
    return name


def corner(color: str) -> str:
    # Rounded inset fold follows the masters' modeled white folded corner.
    return ('<path d="M182 11l57 62h-45q-13 0-13-14z" fill="#15344f" opacity=".24"/>'
            '<path d="M179 10l62 64h-43q-17 0-17-17z" fill="url(#accent)"/>'
            '<path d="M183 15v41q0 13 13 14h38" fill="none" stroke="#fff" stroke-opacity=".32" stroke-width="2"/>')


def bar(y: int, color: str, x: int=49, width: int=193, height: int=58) -> str:
    return (f'<rect x="{x}" y="{y+3}" width="{width}" height="{height}" rx="12" fill="#0b1b33" opacity=".24"/>'
            f'<rect x="{x}" y="{y}" width="{width}" height="{height}" rx="12" fill="url(#accent)" stroke="{color}" stroke-width="1.4"/>'
            f'<rect x="{x+2}" y="{y+2}" width="{width-4}" height="{height*.48}" rx="10" fill="url(#shine)"/>'
            f'<path d="M{x+12} {y+2}h{width-24}" stroke="#fff" stroke-opacity=".45" stroke-width="1.3"/>')


def file_svg(pack: int, label: str, color: str, small: bool=False) -> str:
    body = embedded(f'{pack}-page.png', *PAGE) + corner(color)
    if pack in (1,4,5):
        body += bar(185, color)
        tx, ty, font, ink = 155, 226, 38, '#ffffff'
        if pack == 1:
            body += embedded('1-app.png', 87, 78, 93, 93)
        elif pack == 4:
            body += embedded('4-app.png', 15, 165, 84, 84)
        else:
            body += embedded('5-app.png', 11, 159, 89, 89)
    elif pack in (2,3):
        ink = '#ffffff' if pack == 2 else '#442300'
        body += f'<text x="64" y="59" font-family="Arial,sans-serif" font-size="29" font-weight="bold" fill="{ink}">ExZ</text>'
        body += embedded('2-badge.png' if pack == 2 else tinted_parcel(color), 14, 153, 87, 87)
        tx, ty, font = 164, 214, 37
    elif pack == 6:
        body += embedded('6-app.png', 10, 153, 88, 88)
        tx, ty, font, ink = 164, 215, 38, '#11243c'
    else:
        body += bar(91, color, 35, 208, 65)
        body += embedded('7-app.png', 12, 177, 82, 71)
        tx, ty, font, ink = 139, 136, 42, '#ffffff'
    available = 121 if pack in (2,3,6) else 137
    if pack == 7:
        available = 182
    font = min(font, available/(len(label)*.66))
    if small:
        # Native small-size variants prioritize a readable format label; materials remain unchanged.
        y = 178 if pack != 7 else 96
        body += bar(y, color, 45, 198, 61)
        tx,ty,font,ink = 144, y+44, min(49, 180/(len(label)*.66)), '#ffffff'
    body += f'<text x="{tx}" y="{ty}" text-anchor="middle" font-family="Arial,sans-serif" font-size="{font}" font-weight="bold" fill="{ink}">{label}</text>'
    return svg(body,color)


@lru_cache(maxsize=24)
def master_image(name: str) -> Image.Image:
    with Image.open(MASTERS / name) as image:
        return image.convert('RGBA')


@lru_cache(maxsize=2048)
def raster_data(name: str, width: int, height: int) -> str:
    image = master_image(name).resize((width, height), Image.Resampling.LANCZOS)
    output = io.BytesIO()
    image.save(output, format='PNG')
    return 'data:image/png;base64,' + base64.b64encode(output.getvalue()).decode()


def render(source: str, size: int) -> bytes:
    # Safe SVG renderer receives only local artwork at the needed sampling resolution.
    def local_image(match):
        name, attributes = match[1], match[2]
        width = float(re.search(r'width="([0-9.]+)"', attributes)[1])
        height = float(re.search(r'height="([0-9.]+)"', attributes)[1])
        data = raster_data(name, max(1, math.ceil(width*size/128)), max(1, math.ceil(height*size/128)))
        return '<image xlink:href="' + data + '"' + attributes + '/>'
    source = re.sub(r'<image xlink:href="../../Masters/([a-zA-Z0-9-]+[.]png)"([^>]+)/>', local_image, source)
    return cairosvg.svg2png(bytestring=source.encode(), output_width=size, output_height=size)


def ico(images: list[bytes]) -> bytes:
    offset = 6+16*len(images)
    result = struct.pack('<HHH',0,1,len(images))
    for size,data in zip(SIZES,images):
        result += struct.pack('<BBBBHHII',size%256,size%256,0,0,1,32,len(data),offset)
        offset += len(data)
    return result+b''.join(images)


def extension_colors(extensions: list[str]) -> dict[str,str]:
    previous = OUT / 'manifest.json'
    colors = json.loads(previous.read_text(encoding='utf-8'))['colors'] if previous.exists() else {}
    for index,ext in enumerate(extensions):
        if ext not in colors:
            rgb = colorsys.hls_to_rgb(((index*137.507764)%360)/360,.35+(index%4)*.015,.66+(index%3)*.06)
            colors[ext] = '#'+''.join(f'{round(v*255):02x}' for v in rgb)
    colors.update(zip='#006aff', **{'7z':'#008a7c','rar':'#800fe0'})
    colors = {ext:colors[ext] for ext in extensions}
    assert len(set(colors.values())) == len(extensions)
    return colors


def package_images() -> None:
    layout = json.loads((OUT/'package-layout.json').read_text(encoding='utf-8'))
    for name,(width,height) in layout.items():
        appsize = min(width,height)
        if 'Logo' in name and 'targetsize' not in name:
            appsize = round(appsize*.68)
        source = file_svg(4,'ARC','#2369b3',appsize<=20) if name.startswith('ArchiveFile') else app_svg(4)
        if 'contrast-' in name:
            ink = '#ffffff' if 'contrast-white' in name else '#000000'
            source = svg(f'<path d="M116 35h83l-43 48h-45L79 119h66l-40 44H78v38h44l-27 35H36V119zM148 101h85l-43 48-45 44h67l-36 43H91l50-54 47-47h-70z" fill="{ink}"/>')
        canvas = Image.new('RGBA',(width,height))
        image = Image.open(io.BytesIO(render(source,appsize))).convert('RGBA')
        canvas.alpha_composite(image,((width-appsize)//2,(height-appsize)//2))
        canvas.save(OUT/'Package'/name)
        destination = OUT / 'Package' / name
        if destination.stat().st_size > 204800:
            for colors in (256, 128, 64, 32):
                canvas.quantize(colors=colors, method=Image.Quantize.FASTOCTREE, dither=Image.Dither.NONE).save(destination, optimize=True)
                if destination.stat().st_size <= 204800:
                    break
            else:
                raise ValueError('Package image exceeds the Windows 200 KiB limit: ' + name)
    (OUT/'Package/ArchiveFile.png').write_bytes((OUT/'Package/FileTypes/archive.png').read_bytes())


def main() -> None:
    extract_masters()
    manifest_file = ROOT / 'Package/Package.appxmanifest'
    extensions = sorted({node.text.removeprefix('.') for node in ET.parse(manifest_file).iter()
                         if node.tag.endswith('}FileType') and node.text and node.text.startswith('.')})
    colors = extension_colors(extensions)
    records = []
    for pack,(folder,name) in enumerate(PACKS,1):
        destination = OUT/'Packs'/folder
        destination.mkdir(parents=True,exist_ok=True)
        source = app_svg(pack)
        (destination/'app.svg').write_text(source,encoding='utf-8')
        frames = [render(source,size) for size in SIZES]
        (destination/'app.ico').write_bytes(ico(frames))
        (destination/'app.png').write_bytes(frames[-1])
        for ext in ['archive',*extensions]:
            color = colors.get(ext,'#2369b3')
            label = ext.upper() if ext!='archive' else 'ARC'
            source = file_svg(pack,label,color)
            (destination/f'{ext}.svg').write_text(source,encoding='utf-8')
            frames = [render(file_svg(pack,label,color,size<=20),size) for size in SIZES]
            data = ico(frames)
            (destination/f'{ext}.ico').write_bytes(data)
            if ext in ('zip', '7z', 'rar'):
                (destination / f'{ext}.png').write_bytes(frames[-1])
            records.append({'pack':pack,'extension':ext,'sha256':hashlib.sha256(data).hexdigest()})
            if pack==4:
                files = OUT/'Package/FileTypes'
                files.mkdir(parents=True,exist_ok=True)
                (files/f'{ext}.png').write_bytes(frames[3])
                for size,png in zip(SIZES,frames):
                    (files/f'{ext}.targetsize-{size}.png').write_bytes(png)
        print(f'{pack}: {name}: {len(extensions)} file types',flush=True)
    package_images()
    manifest = {'version':2,'defaultPack':4,'sizes':list(SIZES),'artwork':'Approved dimensional masters with native-size label composition',
                'packs':[{'id':i,'folder':folder,'name':name} for i,(folder,name) in enumerate(PACKS,1)],
                'extensions':extensions,'colors':colors,'icons':records,
                'license':'ExcellentZip original icon artwork and generator: MIT.'}
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
    header = '// Generated by scripts/generate_icons.py.\n#pragma once\n\nnamespace ExcellentZipIcons\n{\n'
    header += 'inline constexpr int kDefaultPack = 4;\ninline constexpr int kPackCount = 7;\n'
    for name,values in [('kPackNames',[n for _,n in PACKS]),('kPackFolders',[f for f,_ in PACKS]),('kExtensions',extensions)]:
        header += f'inline constexpr const wchar_t* {name}[] = {{\n'+''.join(f'    L"{value}",\n' for value in values)+'};\n'
    ((ROOT/'UI_Modern')/'ExcellentZip.IconCatalog.h').write_text(header+'}\n',encoding='utf-8')
    preview()
    from generate_icon_download import generate
    generate()


def preview() -> None:
    sheet = Image.new('RGB',(1400,1960),'#f8f8f5')
    draw = ImageDraw.Draw(sheet)
    fonts = Path('C:/Windows/Fonts')
    title = ImageFont.truetype(str(fonts/'segoeuib.ttf'),54)
    label = ImageFont.truetype(str(fonts/'segoeuib.ttf'),28)
    small = ImageFont.truetype(str(fonts/'segoeui.ttf'),20)
    draw.text((50,28),'ExcellentZip',fill='#071d34',font=title)
    draw.text((52,96),'Approved dimensional icon collection / Monogram EZ is the default',fill='#69768b',font=small)
    for j,text in enumerate(['APP','ZIP','7Z','RAR']):
        draw.text((520+j*205,137),text,fill='#647286',font=small)
    for pack,(folder,name) in enumerate(PACKS,1):
        y = 185+(pack-1)*240
        draw.text((50,y+43),f'{pack}. {name}',fill='#102d46',font=label)
        if pack==4:
            draw.text((50,y+82),'DEFAULT',fill='#007fa6',font=small)
        directory = OUT/'Packs'/folder
        for j,stem in enumerate(['app','zip','7z','rar']):
            source = (directory/f'{stem}.svg').read_text(encoding='utf-8')
            image = Image.open(io.BytesIO(render(source,180))).convert('RGBA')
            sheet.paste(image,(470+j*205,y),image)
        for offset,size in enumerate([16,20,24,32]):
            with Image.open(directory/'zip.ico') as icon:
                image = icon.ico.getimage((size,size)).convert('RGBA')
            sheet.paste(image,(490+offset*48,y+186),image)
        draw.text((710,y+190),'Native sizes: 16 / 20 / 24 / 32 px',fill='#647286',font=small)
        draw.line((50,y+228,1350,y+228),fill='#d7dde2',width=1)
    sheet.save(OUT/'preview.png')


if __name__=='__main__':
    main()
