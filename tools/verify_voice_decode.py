"""Decode every shipped packet with this firmware's vendored fixed-point libopus.

Host-only verification; needs a C compiler, not audio output or ESP-IDF.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import ctypes as C
import re
import subprocess
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from build_voice_pack import packets

root = Path(__file__).resolve().parent.parent
out = root/'build/voice-host'
out.mkdir(parents=True, exist_ok=True)
component = root/'components/opus'
sources = re.findall(r'"(libopus/[^"\n]+\.c)"', (component/'CMakeLists.txt').read_text())
flags = ['cc','-O2','-fPIC','-ffunction-sections','-fdata-sections','-w','-DOPUS_BUILD','-DFIXED_POINT','-DVAR_ARRAYS','-DOPUS_EXPORT=']
flags += ['-I'+str(component/'libopus'/d) for d in ['include','src','celt','silk','silk/fixed','silk/float']]
def compile_source(source):
    obj = out/(source.replace('/','_')+'.gc.o')
    if not obj.exists() or obj.stat().st_mtime < (component/source).stat().st_mtime:
        subprocess.run(flags+['-c',str(component/source),'-o',str(obj)],check=True)
    return str(obj)
with ThreadPoolExecutor(max_workers=8) as pool:
    objects = list(pool.map(compile_source,sources))
(out/'exports.map').write_text('{ global: opus_decoder_create; opus_decoder_destroy; opus_decode; local: *; };')
subprocess.run(['cc','-shared','-Wl,--gc-sections','-Wl,--version-script='+str(out/'exports.map'),*objects,'-lm','-o',str(out/'libvoice-opus.so')],check=True)
lib = C.CDLL(str(out/'libvoice-opus.so'))
lib.opus_decoder_create.argtypes=[C.c_int,C.c_int,C.POINTER(C.c_int)]
lib.opus_decoder_create.restype=C.c_void_p
lib.opus_decoder_destroy.argtypes=[C.c_void_p]
lib.opus_decode.argtypes=[C.c_void_p,C.c_void_p,C.c_int,C.c_void_p,C.c_int,C.c_int]
total_packets=total_samples=total_clips=0
for path in sorted((root/'assets/audio/voice-keychain').rglob('*.opus')):
    total_clips+=1
    error=C.c_int();decoder=lib.opus_decoder_create(16000,1,C.byref(error));assert decoder and error.value==0
    pcm=(C.c_int16*960)()
    try:
        for packet in packets(path.read_bytes()):
            samples=lib.opus_decode(decoder,packet,len(packet),pcm,960,0)
            assert 0<samples<=960,(path.name,samples)
            total_packets+=1;total_samples+=samples
    finally:
        lib.opus_decoder_destroy(decoder)
print(f'Fixed-point Opus decode: PASS ({total_clips} clips, {total_packets} packets, {total_samples} samples)')
