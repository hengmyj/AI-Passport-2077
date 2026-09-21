"""Package a preserving update separately from the destructive merged image."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import sys
import tempfile
import zipfile
from flash_badge import validate_write_ranges, validate_segmented_build

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build',type=Path,default=ROOT/'build')
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();build=args.build.resolve()
    subprocess.run([sys.executable,str(ROOT/'tools/verify_firmware.py'),str(build)],check=True)
    manifest=json.loads((build/'flasher_args.json').read_text())
    images=manifest['flash_files']
    expected={'0x0':'bootloader/bootloader.bin','0x8000':'partition_table/partition-table.bin',
              '0x10000':'FoloToy-AI-Passport.bin','0x3d0000':'voice_data.bin','0x750000':'voice_tail.bin'}
    if images!=expected:raise ValueError('Unsupported target layout')
    validate_write_ranges([(int(offset,16),(build/name).stat().st_size) for offset,name in images.items()])
    version=re.search(r'#define BADGE_VERSION "([^"]+)"',(ROOT/'main/badge_profile.h').read_text())[1]
    english=(ROOT/'docs/preserving-upgrade.md').read_text(encoding='utf-8').replace('(preserving-upgrade.zh_CN.md)','(README.zh_CN.md)')
    chinese=(ROOT/'docs/preserving-upgrade.zh_CN.md').read_text(encoding='utf-8').replace('(preserving-upgrade.md)','(README.md)')
    files={'README.md':english.encode(),'README.zh_CN.md':chinese.encode(),
           'requirements.txt':b'esptool>=4.8,<5\n',
           'upgrade-windows.cmd':(ROOT/'tools/upgrade-windows.cmd').read_bytes()}
    for name in ('flash_badge.py','verify_firmware.py','verify_voice_bundle.py','upgrade_badge.py'):
        files['tools/'+name]=(ROOT/'tools'/name).read_bytes()
    for name in (*images.values(),'flasher_args.json','flash_args','voice_catalog.h'):
        files['firmware/'+name]=(build/name).read_bytes()
    files['release.json']=(json.dumps({'version':version,'install_mode':'backup-and-segmented-write',
        'sha256':{name:hashlib.sha256(data).hexdigest() for name,data in files.items()}},indent=2)+'\n').encode()
    args.output.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(args.output,'w',zipfile.ZIP_DEFLATED) as archive:
        for name,data in files.items():archive.writestr('AI-Passport-Upgrade/'+name,data)
    with tempfile.TemporaryDirectory(prefix='badge-upgrade-check-') as folder:
        with zipfile.ZipFile(args.output) as archive:
            assert archive.testzip() is None
            archive.extractall(folder)
        extracted=Path(folder)/'AI-Passport-Upgrade'
        subprocess.run([sys.executable,str(extracted/'tools/flash_badge.py'),'--help'],check=True,stdout=subprocess.DEVNULL)
        for name,data in files.items():assert (extracted/name).read_bytes()==data
        validate_segmented_build(extracted/'firmware',images)
    print(f'Preserving upgrade package {version}: {args.output} ({args.output.stat().st_size} bytes)')

if __name__=='__main__':main()
