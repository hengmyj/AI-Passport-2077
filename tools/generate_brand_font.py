"""Create the offline company-name Latin font from the existing OFL Noto Sans SC source."""
from pathlib import Path
import sys
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont
from fontTools import subset

font = instantiateVariableFont(TTFont(sys.argv[1]), {'wght': 700}, inplace=True)
options = subset.Options()
options.flavor = 'woff2'
subsetter = subset.Subsetter(options=options)
subsetter.populate(unicodes=list(range(32, 127)) + [0x2026])
subsetter.subset(font)
font.flavor = 'woff2'
font.save(Path(__file__).resolve().parents[1] / 'assets/fonts/NotoSansSC-brand-latin.woff2')
