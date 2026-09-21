"""Rasterize the deliberately small SVG vocabulary used by our original face.

Offline only: Pillow, XML, rect/ellipse/polygon/polyline, white alpha masks.
Unsupported SVG constructs fail instead of silently rendering a wrong asset.
Checked-in A8 arrays need neither Pillow nor an SVG decoder on the device.
"""
from pathlib import Path
import struct
import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw
from draw_face_sources import NAMES as EMOTIONS
ROOT=Path(__file__).resolve().parents[1]
DECORS=['cheek','tear','heart','sweat','question','zzz','star','hand']
EYES=EMOTIONS+['blink','cool_round','cool_sport','angry_puff','wink_left']
NAMES=['body']+['eyes_'+s for s in EYES]+['mouth_'+s for s in EMOTIONS]+['talk_'+str(i) for i in range(3)]+['decor_'+s for s in DECORS]
BASE_COUNT=len(NAMES)
NAMES+=['abstract_'+s for s in NAMES[:-len(DECORS)]]
NAMES+=['abstract_detail']
HUMAN_END=len(NAMES)
NAMES+=['abstract_female_body','abstract_female_detail','abstract_female_accent']
NAMES+=['abstract_bald_gloss']
NAMES+=['abstract_tear']
ROUND_NAMES=['outline','hair','skin','nose','shirt']+['eyes_'+side+'_'+s for side in ['left','right'] for s in EYES]+['mouth_'+s for s in EMOTIONS]+['talk_'+str(i) for i in range(3)]
NAMES+=['round_'+s for s in ROUND_NAMES]
def alpha(value):
    if value=='white':return 255
    if value=='black':return 0
    if len(value)==7 and value[0]=='#' and value[1:3]==value[3:5]==value[5:7]:return int(value[1:3],16)
    raise ValueError(f'Only grayscale mask colors are supported: {value}')
def raster(path):
    eye_prefix=next((p for p in ('eyes_','abstract_eyes_','round_eyes_') if path.stem.startswith(p)),None)
    if eye_prefix and path.stem[len(eye_prefix):] in ('winking','wink_left'):
        path=path.with_name(eye_prefix+'blink.svg')
    root=ET.parse(path).getroot();w,h=int(root.attrib['width']),int(root.attrib['height']);scale=4
    assert root.attrib['viewBox']==f'0 0 {w} {h}'
    im=Image.new('L',(w*scale,h*scale));draw=ImageDraw.Draw(im)
    for node in root:
        tag=node.tag.rsplit('}',1)[-1];a=node.attrib
        color=alpha(a.get('fill','white')) if a.get('fill')!='none' else 0
        def v(name,default=0):return float(a.get(name,default))*scale
        if tag=='rect':
            x,y,ww,hh=v('x'),v('y'),v('width'),v('height')
            draw.rounded_rectangle((x,y,x+ww-1,y+hh-1),radius=v('rx'),fill=color)
        elif tag=='ellipse':
            x,y,rx,ry=v('cx'),v('cy'),v('rx'),v('ry');draw.ellipse((x-rx,y-ry,x+rx-1,y+ry-1),fill=color)
        elif tag in ('polygon','polyline'):
            points=[tuple(float(n)*scale for n in p.split(',')) for p in a['points'].split()]
            if tag=='polygon':draw.polygon(points,fill=color)
            else:
                assert a.get('fill')=='none'
                color=alpha(a['stroke'])
                width=round(v('stroke-width'));draw.line(points,fill=color,width=width,joint='curve')
                if a.get('stroke-linecap')=='round':
                    for x,y in [points[0],points[-1]]:draw.ellipse((x-width/2,y-width/2,x+width/2-1,y+width/2-1),fill=color)
        else:raise ValueError(f'Unsupported SVG tag: {tag}')
    im=im.resize((w,h),Image.Resampling.LANCZOS)
    if eye_prefix or path.stem in ('abstract_tear','decor_tear','decor_sweat'):
        # All styles use the left eye as the authoring reference. Mirror after
        # downsampling so small strokes/highlights and paired tears agree exactly.
        assert w%2==0
        half=w//2
        im.paste(im.crop((0,0,half,h)).transpose(Image.Transpose.FLIP_LEFT_RIGHT),(half,0))
    return im
def pack_rows(data,w,h):
    offset=2*(h+1);offsets=[];runs=bytearray()
    for y in range(h):
        offsets.append(offset+len(runs));row=data[y*w:(y+1)*w];i=0
        while i<w:
            end=i+1
            while end<w and row[end]==row[i] and end-i<255:end+=1
            runs.extend((end-i,row[i]));i=end
    offsets.append(offset+len(runs))
    assert offsets[-1]<65536
    return struct.pack('<'+'H'*len(offsets),*offsets)+runs

def generate():
    rows=['/* Original SVG face artwork; generated offline by tools/generate_face_assets.py. */','#include "xiaozhi_face_assets.h"'];descs=[];total=0;buffers={}
    for name in NAMES:
        source=name;origin_x=0
        # Store separated eye areas instead of the wide transparent gap. This
        # is lossless and also deduplicates symmetric eyes without a decoder.
        split=name.startswith(('round_eyes_left_','round_eyes_right_'))
        if split:
            source=name.replace('_left_','_').replace('_right_','_')
            origin_x=56 if '_right_' in name else 0
        im=raster(ROOT/'assets/images/xiaozhi-face'/f'{source}.svg')
        if split:im=im.crop((origin_x,0,origin_x+56,im.height))
        x,y,r,b=im.getbbox();w=(r-x+3)//4*4;h=b-y
        cropped=Image.new('L',(w,h));cropped.paste(im.crop((x,y,r,b)),(0,0));data=cropped.tobytes()
        packed=pack_rows(data,w,h)
        compressed=len(packed)<len(data)
        if compressed:data=packed
        flags='LV_IMAGE_FLAGS_USER1|LV_IMAGE_FLAGS_COMPRESSED' if compressed else '0'
        key=(w,h,compressed,data)
        if key not in buffers:
            buffers[key]=name;total+=len(data);rows.append(f'static const uint8_t {name}_data[]={{')
            rows.extend(','.join(str(v) for v in data[i:i+32])+',' for i in range(0,len(data),32));rows.append('};')
        descs.append(f'{{.image={{.header={{.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_A8,.flags={flags},.w={w},.h={h},.stride={w}}},.data_size={len(data)},.data={buffers[key]}_data}},.x={x+origin_x},.y={y}}}')
    eyes=1+len(EYES);mouths=eyes+len(EMOTIONS)+3
    rows.extend(['const xz_face_asset_t xz_face_body='+descs[0]+';','const xz_face_asset_t xz_face_eye_frames[XZ_EYE_COUNT]={'+',\n'.join(descs[1:eyes])+'};','const xz_face_asset_t xz_face_mouth_frames[XZ_FACE_COUNT+3]={'+',\n'.join(descs[eyes:mouths])+'};','const xz_face_asset_t xz_face_decor[XZ_DECOR_COUNT]={'+',\n'.join(descs[mouths:BASE_COUNT])+'};'])
    a=BASE_COUNT
    rows.extend(['const xz_face_asset_t xz_abstract_body='+descs[a]+';','const xz_face_asset_t xz_abstract_eye_frames[XZ_EYE_COUNT]={'+',\n'.join(descs[a+1:a+eyes])+'};','const xz_face_asset_t xz_abstract_mouth_frames[XZ_FACE_COUNT+3]={'+',\n'.join(descs[a+eyes:HUMAN_END-1])+'};','const xz_face_asset_t xz_abstract_detail='+descs[HUMAN_END-1]+';'])
    for i,name in enumerate(['body','detail','accent']):rows.append('const xz_face_asset_t xz_female_'+name+'='+descs[HUMAN_END+i]+';')
    by_name=dict(zip(NAMES,descs))
    rows.append('const xz_face_asset_t xz_bald_gloss='+by_name['abstract_bald_gloss']+';')
    rows.append('const xz_face_asset_t xz_portrait_tear='+by_name['abstract_tear']+';')
    for name in ['outline','hair','skin','nose','shirt']:rows.append('const xz_face_asset_t xz_round_'+name+'='+by_name['round_'+name]+';')
    for side in ['left','right']:rows.append('const xz_face_asset_t xz_round_eye_'+side+'_frames[XZ_EYE_COUNT]={'+',\n'.join(by_name['round_eyes_'+side+'_'+s] for s in EYES)+'};')
    rows.append('const xz_face_asset_t xz_round_mouth_frames[XZ_FACE_COUNT+3]={'+',\n'.join(by_name['round_'+s] for s in ['mouth_'+e for e in EMOTIONS]+['talk_'+str(i) for i in range(3)])+'};')
    (ROOT/'main/xiaozhi_face_assets.c').write_text('\n'.join(rows)+'\n',encoding='utf-8')
    print(f'Face lossless masks: {total} bytes; bounded scanline decoding, no full-screen frame buffer')
if __name__=='__main__':generate()
