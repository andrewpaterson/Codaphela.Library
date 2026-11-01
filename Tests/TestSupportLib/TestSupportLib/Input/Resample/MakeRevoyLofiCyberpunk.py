import sys
from PIL import Image
src, out = sys.argv[1], sys.argv[2]
im = Image.open(src).convert('RGB')
print(im.size)
box = (760, 228, 1570, 876)  #Her head and headphones; 810 x 648 (5:4).
crop = im.crop(box)
crop.resize((640, 512), Image.LANCZOS).save(out + '/RevoyLofiCyberpunk-640.png')
crop.resize((160, 128), Image.LANCZOS).save(out + '/RevoyLofiCyberpunk.png')
