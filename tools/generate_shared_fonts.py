"""Generate shared, visually identical Noto Sans SC 14px/4bpp UI fonts."""
from pathlib import Path
import re
import sys
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]

def generate(path):
    chars = set((ROOT/'assets/fonts/ui14-characters.txt').read_text(encoding='utf-8').strip())
    for source in (ROOT/'main').iterdir():
        if source.suffix in ('.c', '.cc', '.h') and not source.name.startswith('font_') and source.name != 'yao_text_data.c':
            chars.update(re.findall(r'[\u3000-\u303f\u3400-\u9fff\uff00-\uffef]', source.read_text(encoding='utf-8-sig')))
    chars = sorted(chars | {chr(i) for i in range(32, 127)})
    font = ImageFont.truetype(str(path), 14)
    font.set_variation_by_axes([500])
    bitmap, glyphs = [], ['{0}']
    for char in chars:
        left, top, right, bottom = font.getbbox(char, anchor='ls')
        w, h = right-left, bottom-top
        image = Image.new('L', (max(1,w), max(1,h)))
        ImageDraw.Draw(image).text((-left,-top),char,font=font,fill=255,anchor='ls')
        pixels = [round(p/17) for p in image.getdata()] if w and h else []
        offset = len(bitmap)
        pixels += [0] * (len(pixels)%2)
        bitmap.extend((pixels[i]<<4)|pixels[i+1] for i in range(0,len(pixels),2))
        glyphs.append(f'{{.bitmap_index={offset}, .adv_w={round(font.getlength(char)*16)}, .box_w={w}, .box_h={h}, .ofs_x={left}, .ofs_y={-bottom}}}')
    lines = ['/* Noto Sans SC, SIL OFL 1.1; see assets/fonts/OFL.txt. Generated shared UI font. */', '#include "lvgl.h"', 'static const uint8_t bitmap[]={']
    lines += [','.join(str(v) for v in bitmap[i:i+32])+',' for i in range(0,len(bitmap),32)]
    lines += ['};','static const lv_font_fmt_txt_glyph_dsc_t glyphs[]={', ',\n'.join(glyphs), '};',
              'static const uint16_t unicode[]={'+','.join(str(ord(c)-32) for c in chars)+'};',
              f'static const lv_font_fmt_txt_cmap_t maps[]={{{{.range_start=32,.range_length={ord(chars[-1])-31},.glyph_id_start=1,.unicode_list=unicode,.list_length={len(chars)},.type=LV_FONT_FMT_TXT_CMAP_SPARSE_TINY}}}};',
              'const lv_font_fmt_txt_dsc_t font_ui_14_descriptor={.glyph_bitmap=bitmap,.glyph_dsc=glyphs,.cmaps=maps,.cmap_num=1,.bpp=4,.bitmap_format=0};']
    (ROOT/'main/font_ui_14.c').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    for name in ('voice','radio','muyu'):
        (ROOT/'main'/f'font_{name}_14.c').write_text(
            '/* Shared Noto Sans SC 14px glyphs; see assets/fonts/OFL.txt. */\n#include "lvgl.h"\n'
            'extern const lv_font_fmt_txt_dsc_t font_ui_14_descriptor;\n'
            f'const lv_font_t font_{name}_14={{.get_glyph_dsc=lv_font_get_glyph_dsc_fmt_txt,.get_glyph_bitmap=lv_font_get_bitmap_fmt_txt,.line_height=18,.base_line=4,.dsc=&font_ui_14_descriptor}};\n',encoding='utf-8')
    print(f'Shared UI: {len(chars)} glyphs / {len(bitmap)} bitmap bytes')

if __name__ == '__main__':
    generate(sys.argv[1])
