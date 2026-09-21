"""Verify the compiled request packets match the documented synthetic source."""
from pathlib import Path
import re, struct
root = Path(__file__).resolve().parents[1]
source = (root / "assets/cyberyao/interpret-request.ogg").read_bytes()
packets, pending, offset = [], bytearray(), 0
while offset < len(source):
    assert source[offset:offset+4] == b"OggS"
    segments = source[offset+26]
    sizes = source[offset+27:offset+27+segments]
    pos = offset + 27 + segments
    for size in sizes:
        pending.extend(source[pos:pos+size]); pos += size
        if size < 255:
            packets.append(bytes(pending)); pending.clear()
    offset = pos
assert not pending and packets[0][:8] == b"OpusHead" and packets[1][:8] == b"OpusTags"
assert packets[0][9] == 1
expected = b"".join(struct.pack("<H",len(p)) + p for p in packets[2:])
header = (root / "main/assets/yao_request.h").read_text()
actual = bytes(map(int,re.findall(r"\d+",header.split("[]={",1)[1].split("}",1)[0])))
assert actual == expected
assert len(packets[2:]) == 78 and len(actual) == 5303
assert all(0 < len(p) <= 1500 for p in packets[2:])
print("Synthetic request: compiled/source match; 78 Opus packets; 5303 bytes: PASS")
