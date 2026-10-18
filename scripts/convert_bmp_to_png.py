#!/usr/bin/env python3
import os
import glob
from PIL import Image

screenshots_dir = os.path.join(os.path.dirname(__file__), "..", "verification", "screenshots")
for bmp_file in glob.glob(os.path.join(screenshots_dir, "*.bmp")):
    png_file = os.path.splitext(bmp_file)[0] + ".png"
    if not os.path.exists(png_file) or os.path.getmtime(bmp_file) > os.path.getmtime(png_file):
        try:
            im = Image.open(bmp_file)
            im.save(png_file, "PNG")
            print(f"Converted {os.path.basename(bmp_file)} -> {os.path.basename(png_file)}")
        except Exception as e:
            print(f"Error converting {bmp_file}: {e}")
