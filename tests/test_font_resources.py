"""Ensure deduplication preserves legacy pixels, and captions cover fallback labels."""
from pathlib import Path
import hashlib
import json
import re

ROOT=Path(__file__).resolve().parents[1]
def read_font(name):
    s=(ROOT/'main'/name).read_text(encoding='utf-8')
    start=int(re.search(r'\.range_start=(\d+)',s)[1])
    chars=[start+int(n) for n in re.search(r'uint16_t unicode\[\]\s*=\s*\{([^}]+)',s)[1].split(',')]
    raw=re.search(r'uint8_t bitmap\[\]\s*=\s*\{(.*?)\};',s,re.S)[1]
    data=bytes(int(x,0) for x in re.findall(r'0x[0-9a-fA-F]+|\d+',raw))
    bpp=int(re.search(r'\.bpp=(\d+)',s)[1])
    glyphs=re.findall(r'\{\.bitmap_index=(\d+),\s*\.adv_w=(\d+),\s*\.box_w=(\d+),\s*\.box_h=(\d+),\s*\.ofs_x=(-?\d+),\s*\.ofs_y=(-?\d+)\}',s)
    assert len(chars)==len(glyphs)
    result={}
    for c,g in zip(chars,glyphs):
        pos,adv,w,h,x,y=map(int,g)
        end=pos+(w*h*bpp+7)//8
        assert end<=len(data)
        result[c]=[adv,w,h,x,y,data[pos:end].hex()]
    return result

shared=read_font('font_ui_14.c')
baseline=json.loads((ROOT/'tests/ui14_baseline.json').read_text())
for name,item in baseline.items():
    pixels=[shared[c] for c in item['chars']]
    assert hashlib.sha256(json.dumps(pixels,separators=(',',':')).encode()).hexdigest()==item['sha256'], name
    wrapper=(ROOT/'main'/f'font_{name}_14.c').read_text()
    assert 'static const uint8_t bitmap' not in wrapper
    assert 'font_ui_14_descriptor' in wrapper

caption=read_font('font_xiaozhi_14.c')
for a in range(0xa1,0xf8):
    for b in range(0xa1,0xff):
        try: c=bytes([a,b]).decode('gb2312')
        except UnicodeDecodeError: continue
        assert ord(c) in caption, c
labels=(ROOT/'main/xiaozhi_text.c').read_text(encoding='utf-8')
for label in re.findall(r'"(\[[^"\n]+\])"',labels):
    assert all(ord(c) in caption for c in label),label
for c in '口妳玥喆祎囧囍謝臺灣網絡哒欸':
    assert ord(c) in caption,c
assert set(range(32,127))<=set(caption)
print('Shared fonts preserve all legacy glyph pixels/metrics; caption coverage and fallback labels: PASS')

# Classical passages must render without missing-glyph squares.
classic=(ROOT/"main/yao_text_data.c").read_text(encoding="utf-8")
for ch in set(re.findall(r"[\u3400-\u9fff]",classic)):
    assert ord(ch) in caption, ch
print("Yao classical glyph coverage: PASS")
