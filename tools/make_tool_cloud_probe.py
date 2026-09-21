"""Pack four explicit 8 kHz mono PCM WAVs into a temporary probe header.

Usage: python tools/make_tool_cloud_probe.py BUILD_DIR WAV1 WAV2 WAV3 WAV4
Use fixed test phrases for battery, reminder query, volume=100, set/cancel.
The cloud probe restores the original volume before exiting. Never enable
the probe in a release, or pass personal microphone recordings here.
"""
from pathlib import Path
import sys
import wave

if len(sys.argv) != 6:
    raise SystemExit(__doc__)
samples, offsets, sizes = [], [], []
for path in sys.argv[2:]:
    with wave.open(path, "rb") as source:
        assert (source.getframerate(), source.getnchannels(), source.getsampwidth()) == (8000, 1, 2)
        data = source.readframes(source.getnframes())
    assert data
    offsets.append(len(samples))
    sizes.append(len(data) // 2)
    samples.extend(int.from_bytes(data[i:i + 2], "little", signed=True) for i in range(0, len(data), 2))
assert len(samples) * 2 < 180000, "Probe audio exceeds the flash budget"
header = "static const unsigned cloud_pcm_offsets[]={" + ",".join(map(str, offsets)) + "};\n"
header += "static const unsigned cloud_pcm_sizes[]={" + ",".join(map(str, sizes)) + "};\n"
header += "static const int16_t cloud_pcm[]={\n"
header += ",\n".join(",".join(map(str, samples[i:i + 32])) for i in range(0, len(samples), 32)) + "\n};\n"
output = Path(sys.argv[1]) / "cloud_probe_pcm.h"
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(header, encoding="utf-8")
print(f"Temporary probe audio: {len(samples) * 2} bytes; production flag must stay OFF")
