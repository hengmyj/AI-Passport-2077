#!/usr/bin/env python3
"""Noto Sans SC 14px / 1bpp captions with checked coverage (SIL OFL 1.1)."""
from pathlib import Path
import sys,re
from fontTools.ttLib import TTFont
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parent.parent
chars=set(chr(c) for c in range(32,127))
for a in range(0xa1,0xf8):
    for b in range(0xa1,0xff):
        try: chars.add(bytes([a,b]).decode('gb2312'))
        except UnicodeDecodeError: pass
original = set(chars)
# Preserve GB2312 and add UI/name characters plus everyday punctuation and symbols.
chars.update((root/'assets/fonts/ui14-characters.txt').read_text(encoding='utf-8').strip())
chars.update('妳祢喆祎玥珩琛璟瑄昱昀翊颢钰铖铄槿栩梓垚淼焱燚龘囍囧冇嘅啲佢咁唔喺噉嗨嘞嘻嘤哒哟诶欸呗哇喔咯咩麼麽裏裡臺灣網絡謝歡愛聽說話開關這個體驗與為會時來學習幫聲請問對嗎還讓點腦電樂貓龍鬱')
for a,b in [(0xa0,0x100),(0x2000,0x2070),(0x2100,0x2150),(0x2190,0x2200),(0x2600,0x2700)]:
    chars.update(chr(c) for c in range(a,b))
for source in (root/'main').iterdir():
    if source.suffix in ('.c','.cc','.h') and not source.name.startswith('font_'):
        chars.update(re.findall(r'[\u3000-\u303f\u3400-\u9fff\uff00-\uffef]',source.read_text(encoding='utf-8-sig')))
with TTFont(sys.argv[1]) as ttf:
    available=set(ttf.getBestCmap())
missing=original-{chr(c) for c in available}
if missing: raise ValueError(f'Source font is missing original glyphs: {missing}')
chars=sorted(c for c in chars if ord(c) in available and ord(c)<=0xffff)
font=ImageFont.truetype(sys.argv[1],14);font.set_variation_by_axes([500])
bitmap=[];glyphs=['{0}']
for char in chars:
    left,top,right,bottom=font.getbbox(char,anchor='ls');w=right-left;h=bottom-top
    image=Image.new('L',(max(w,1),max(h,1)));ImageDraw.Draw(image).text((-left,-top),char,font=font,fill=255,anchor='ls')
    pixels=[int(p>=112) for p in image.getdata()] if w and h else []
    offset=len(bitmap);pixels += [0]*((-len(pixels))%8)
    bitmap += [sum(pixels[i+j]<<(7-j) for j in range(8)) for i in range(0,len(pixels),8)]
    glyphs.append(f'{{.bitmap_index={offset},.adv_w={round(font.getlength(char)*16)},.box_w={w},.box_h={h},.ofs_x={left},.ofs_y={-bottom}}}')
lines=['/* Noto Sans SC, SIL OFL 1.1; see assets/fonts/OFL.txt. Generated captions. */','#include "lvgl.h"','static const uint8_t bitmap[]={']
lines += [','.join(str(v) for v in bitmap[i:i+32])+',' for i in range(0,len(bitmap),32)]
lines += ['};','static const lv_font_fmt_txt_glyph_dsc_t glyphs[]={',',\n'.join(glyphs),'};','static const uint16_t unicode[]={'+','.join(str(ord(c)-32) for c in chars)+'};',
    'static const lv_font_fmt_txt_cmap_t maps[]={{.range_start=32,.range_length='+str(ord(chars[-1])-31)+',.glyph_id_start=1,.unicode_list=unicode,.list_length='+str(len(chars))+',.type=LV_FONT_FMT_TXT_CMAP_SPARSE_TINY}};',
    'static const lv_font_fmt_txt_dsc_t desc={.glyph_bitmap=bitmap,.glyph_dsc=glyphs,.cmaps=maps,.cmap_num=1,.bpp=1,.bitmap_format=0};',
    'const lv_font_t font_xiaozhi_14={.get_glyph_dsc=lv_font_get_glyph_dsc_fmt_txt,.get_glyph_bitmap=lv_font_get_bitmap_fmt_txt,.line_height=18,.base_line=4,.dsc=&desc};',
    '#include "xiaozhi_text.h"',
    'bool xiaozhi_font_has_glyph(uint32_t cp){',
    '    if(cp<32 || cp>65535)return false;',
    '    size_t low=0,high=sizeof(unicode)/sizeof(unicode[0]);',
    '    while(low<high){size_t mid=low+(high-low)/2;uint32_t value=unicode[mid]+32u;if(value==cp)return true;if(value<cp)low=mid+1;else high=mid;}',
    '    return false;',
    '}']
(root/'main/font_xiaozhi_14.c').write_bytes(('\n'.join(lines)+'\n').encode())
print(f'Caption glyphs={len(chars)} bitmap={len(bitmap)} bytes')
