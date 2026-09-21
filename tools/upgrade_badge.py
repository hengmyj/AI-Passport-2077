"""Interactive entry point for the standalone preserving upgrade package."""
from pathlib import Path
from datetime import datetime
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys

def verify_package(root):
    release=json.loads((root/'release.json').read_text(encoding='utf-8'))
    for name,digest in release['sha256'].items():
        path=(root/name).resolve()
        if not path.is_relative_to(root.resolve()) or hashlib.sha256(path.read_bytes()).hexdigest()!=digest:
            raise ValueError('Package file missing or modified: '+name)
    return release['version']

def main():
    root=Path(__file__).resolve().parents[1]
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port')
    parser.add_argument('--backup-dir',type=Path)
    parser.add_argument('--check-only',action='store_true',help='Verify package without connecting to a device')
    args=parser.parse_args()
    version=verify_package(root)
    from flash_badge import validate_segmented_build
    flash=json.loads((root/'firmware/flasher_args.json').read_text())
    validate_segmented_build(root/'firmware',flash['flash_files'])
    print('Package verified: '+version,flush=True)
    if args.check_only:return
    import esptool
    if esptool.__version__.split('.')[0]!='4':raise RuntimeError('Install esptool 4: python -m pip install -r requirements.txt')
    from serial.tools import list_ports
    if not args.port:
        print('Connect USB. Disconnect the badge in the web configuration page.')
        for port in list_ports.comports():print(port.device,port.description)
        args.port=input('Badge USB port (for example COM6): ').strip()
    if os.name=='nt' and not re.fullmatch(r'COM[1-9][0-9]*',args.port,re.IGNORECASE):
        raise ValueError('Expected a COM port followed by digits')
    private=Path(os.environ.get('LOCALAPPDATA',Path.home()))/'AI-Passport'/'Backups'
    dest=args.backup_dir or private/datetime.now().strftime('%Y%m%d-%H%M%S-%f')
    print('Private full-device backup: '+str(dest),flush=True)
    subprocess.run([sys.executable,str(root/'tools/flash_badge.py'),'--port',args.port,
                    '--build',str(root/'firmware'),'--backup-dir',str(dest)],check=True)
    print('Update completed and user data verified. Keep the private backup.',flush=True)

if __name__=='__main__':main()
