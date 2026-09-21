"""Convert an explicitly supplied test WAV to a probe-only flash header."""
from pathlib import Path
import sys
import wave

with wave.open(sys.argv[1], "rb") as source:
    assert (source.getnchannels(), source.getsampwidth(), source.getframerate()) == (1, 2, 16000)
    pcm = source.readframes(source.getnframes())
assert 0 < len(pcm) <= 128 * 1024
target = Path(__file__).resolve().parent.parent / "build/xiaozhi_probe_pcm.h"
target.parent.mkdir(exist_ok=True)
rows = [",".join(str(v) for v in pcm[i:i + 32]) for i in range(0, len(pcm), 32)]
target.write_text("/* Test input: excluded unless CONVERSATION_PROBE is enabled. */\n"
                  "static const unsigned char xz_probe_pcm[]={\n" + ",\n".join(rows) + "\n};\n")
print(f"Probe PCM: {len(pcm)} bytes; production flag must remain OFF")
