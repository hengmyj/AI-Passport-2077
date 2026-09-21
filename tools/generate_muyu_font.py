"""Regenerate the shared 14px font without duplicating glyphs."""
import sys
from generate_shared_fonts import generate

if __name__ == "__main__":
    generate(sys.argv[1])
