#!/usr/bin/env python3
"""Package ONE compatible, already trained WakeNet9s model; never train text.

Use --source DIRECTORY to package a custom model supplied for ESP-SR 2.1.3/C3.
The displayed phrase comes from the model metadata, not a user-editable label.
"""
import argparse,hashlib,json,re,struct
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
def pack(source,destination):
    name=source.name
    if not re.fullmatch(r'wn9s_[a-z0-9_]{1,26}',name):raise ValueError('Require a WakeNet9s model directory name')
    info=(source/'_MODEL_INFO_').read_text(encoding='utf-8').strip()
    parts=info.split('_')
    if len(parts)!=6 or parts[0]!='wakenet9s':raise ValueError('Require a single-phrase WakeNet9s metadata format')
    phrase=parts[2]
    if not phrase or len(phrase)>16 or any(ord(c)<32 for c in phrase):raise ValueError('Invalid wake phrase')
    files=['_MODEL_INFO_','wn9_data','wn9_index'];offset=40+40*len(files)
    header=bytearray(struct.pack('<I32sI',1,name.encode(),len(files)));payload=bytearray()
    for filename in files:
        raw=(source/filename).read_bytes()
        if not raw or len(raw)>256*1024:raise ValueError('Model file size out of range')
        header.extend(struct.pack('<32sII',filename.encode(),offset,len(raw)));payload.extend(raw);offset+=len(raw)
    destination.mkdir(parents=True,exist_ok=True)
    (destination/'wake_model.bin').write_bytes(header+payload)
    # Bench-calibrated only for the bundled coefficients. Custom models retain
    # their own normal-mode threshold until separately validated on the device.
    known=hashlib.sha256((source/'wn9_data').read_bytes()).hexdigest()=='f93799743dad310e2933446c302088d32605d3275206f53d3f3c184220caecf6'
    threshold='0.67f' if known else '0.0f'
    (destination/'wake_model_config.h').write_text('#pragma once\n#define BADGE_WAKE_PHRASE '+json.dumps(phrase,ensure_ascii=False)+'\n#define BADGE_WAKE_THRESHOLD '+threshold+'\n',encoding='utf-8')
    return len(header+payload)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('destination',type=Path);p.add_argument('--source',type=Path,default=ROOT/'assets/xiaozhi/wake/wn9s_nihaoxiaozhi')
    a=p.parse_args();print('Packaged WakeNet9s model bytes:',pack(a.source,a.destination))
