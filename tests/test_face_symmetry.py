"""Check the actual compiled A8/RLE resources, without image-library dependencies."""
from pathlib import Path
import re
import struct

source = (Path(__file__).resolve().parents[1] / 'main/xiaozhi_face_assets.c').read_text()
buffers = {name: bytes(map(int, re.findall(r'\d+', body))) for name, body in
           re.findall(r'static const uint8_t (\w+)\[\]=\{(.*?)\};', source, re.S)}
pattern = re.compile(r'\.flags=([^,]+),\.w=(\d+),\.h=(\d+),\.stride=\d+\},'
                     r'\.data_size=(\d+),\.data=(\w+)\},\.x=(-?\d+),\.y=(-?\d+)')

def assets(name):
    body = re.search(r'const xz_face_asset_t '+name+r'(?:\[[^]]+\])?=(.*?);', source, re.S)[1]
    return pattern.findall(body)

def plane(asset, width, height):
    flags, w, h, size, name, x, y = asset
    w, h, size, x, y = map(int, (w, h, size, x, y))
    data = buffers[name]
    assert len(data) == size
    result = [bytearray(width) for _ in range(height)]
    for row in range(h):
        if 'COMPRESSED' in flags:
            start, end = struct.unpack_from('<HH', data, row * 2)
            assert 2*(h+1) <= start <= end <= size
            pixels = bytearray()
            for at in range(start, end, 2):
                assert data[at] > 0
                pixels.extend([data[at+1]] * data[at])
        else:
            pixels = data[row*w:(row+1)*w]
        assert len(pixels) == w
        for col, alpha in enumerate(pixels):
            if 0 <= y+row < height and 0 <= x+col < width:
                result[y+row][x+col] = alpha
            else:
                assert alpha == 0  # Only transparent alignment padding may extend outside.
    return result

for name in ('xz_face_eye_frames', 'xz_abstract_eye_frames'):
    entries = assets(name)
    assert len(entries) == 26
    for entry in entries:
        assert all(row == row[::-1] for row in plane(entry, 80, 32)), name
    # Both historical wink IDs now perform the same synchronized blink.
    for index in (12, 25):
        assert plane(entries[index], 80, 32) == plane(entries[21], 80, 32)
left, right = assets('xz_round_eye_left_frames'), assets('xz_round_eye_right_frames')
assert len(left) == len(right) == 26
for a, b in zip(left, right):
    joined = [bytearray(max(x, y) for x, y in zip(l, r))
              for l, r in zip(plane(a, 112, 28), plane(b, 112, 28))]
    assert all(row == row[::-1] for row in joined)
for entry, w, h in ((assets('xz_portrait_tear')[0], 10, 16), (assets('xz_face_decor')[1], 12, 27),
                    (assets('xz_face_decor')[3], 12, 20)):
    assert all(row == row[::-1] for row in plane(entry, w, h))
print('Compiled face masks: all 78 eye pairs, synchronized winks and both tear masks symmetric PASS')
