"""Migration accepts only the recognized, intact image below the resource boundary."""
import hashlib
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch
from types import SimpleNamespace
sys.path.insert(0,str(Path(__file__).resolve().parent.parent/'tools'))
from flash_badge import old_badge_application, validate_existing, validate_write_ranges

def image(payload=b'Arasaka Personnel Badge 1.7.1 starting'):
    header=bytearray(24);header[0]=0xe9;header[1]=1;header[23]=1
    data=header+struct.pack('<II',0x3c000020,len(payload))+payload
    checksum=0xef
    for value in payload:checksum^=value
    data+=bytes((15-len(data)%16)%16)+bytes([checksum])
    return bytes(data)+hashlib.sha256(data).digest()

class MigrationTests(unittest.TestCase):
    def test_write_plan_preserves_sectors(self):
        validate_write_ranges([(0,0x6000),(0x8000,0xc00),(0x10000,0x3c0000),
                               (0x3d0000,0x2c0000),(0x750000,0x80000)])
        for offset,size in [(0,0x7d0000),(0x9000,1),(0x8000,0x1001),
                            (0x68ffff,2),(0x750000,0x80001),(0x7d0000,1)]:
            with self.assertRaisesRegex(ValueError,'user data'):
                validate_write_ranges([(offset,size)])
        for offset,size in [(-1,1),(0,0),(0x800000,1)]:
            with self.assertRaisesRegex(ValueError,'Invalid'):
                validate_write_ranges([(offset,size)])

    def test_voice_layout_migration_preserves_user_regions(self):
        fixed=[('nvs',0x9000,0x6000),('phy_init',0xf000,0x1000),
               ('badge_slots',0x690000,0xc0000),('badge_user',0x7d0000,0x30000),
               ('voice_tail',0x750000,0x80000)]
        for app_size,offset,size in [(0x200000,0x210000,0x480000),(0x2c0000,0x2d0000,0x3c0000),(0x2e0000,0x2f0000,0x3a0000),(0x3c0000,0x3d0000,0x2c0000)]:
            parts=[SimpleNamespace(label=n,offset=o,size=s) for n,o,s in
                   fixed+[('factory',0x10000,app_size),('voice_data',offset,size)]]
            with patch('flash_badge.parse_partition_table',return_value=(parts,None)):
                validate_existing(b'')
                parts[-1].offset+=0x10000
                with self.assertRaisesRegex(ValueError,'voice layout'):validate_existing(b'')
                parts[-1].offset=offset
                parts[2].offset+=0x1000
                with self.assertRaisesRegex(ValueError,'user-data'):validate_existing(b'')

    def test_recognized_image(self):
        app=image();flash=bytes(0x10000)+app+b'\xff'*100
        self.assertEqual(old_badge_application(flash),app)

    def test_unknown_or_damaged(self):
        with self.assertRaises(ValueError):old_badge_application(bytes(0x10000)+image(b'Unrelated application'))
        app=bytearray(image());app[-1]^=1
        with self.assertRaisesRegex(ValueError,'hash'):old_badge_application(bytes(0x10000)+app)

    def test_bounds(self):
        app=bytearray(image());struct.pack_into('<I',app,28,0x300000)
        with self.assertRaisesRegex(ValueError,'boundary'):old_badge_application(bytes(0x10000)+app)
        with self.assertRaises(ValueError):old_badge_application(b'\xff'*16)

if __name__=='__main__':unittest.main()
