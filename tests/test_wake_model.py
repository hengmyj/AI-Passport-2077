"""Model packaging must preserve coefficients and derive the actual wake phrase."""
import importlib.util,struct,tempfile,shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('wake_pack',ROOT/'tools/build_wake_model.py')
mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
with tempfile.TemporaryDirectory() as tmp:
    d=Path(tmp);source=ROOT/'assets/xiaozhi/wake/wn9s_nihaoxiaozhi'
    size=mod.pack(source,d/'out');raw=(d/'out/wake_model.bin').read_bytes()
    count,name,files=struct.unpack_from('<I32sI',raw)
    assert count==1 and files==3 and name.rstrip(b'\0')==source.name.encode()
    assert size==len(raw) and size<0x20000
    for i in range(files):
        filename,offset,length=struct.unpack_from('<32sII',raw,40+40*i)
        expected=(source/filename.rstrip(b'\0').decode()).read_bytes()
        assert offset>=160 and raw[offset:offset+length]==expected
    assert '你好小智' in (d/'out/wake_model_config.h').read_text(encoding='utf-8')
    custom=d/'wn9s_custom';shutil.copytree(source,custom)
    info=custom/'_MODEL_INFO_';original=info.read_text(encoding='utf-8')
    for invalid in ['',original.replace('wakenet9s','wakenet9'),original.replace('你好小智',''),original+'_another_word']:
        info.write_text(invalid,encoding='utf-8')
        try:mod.pack(custom,d/'bad')
        except ValueError:pass
        else:raise AssertionError('Invalid model metadata accepted')
print('Wake model packaging PASS: coefficients, offsets, actual phrase, unsupported models rejected')
