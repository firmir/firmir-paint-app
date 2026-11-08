# firmir-paint

Mouse painting on a canvas: color palette (white = eraser), brush size,
clear, and PNG export (libpng).

```
cmake -S . -B build && cmake --build build -j

Firmir is located via pkg-config (installed `firmir-*.pc` files); if not
found there, the sibling `../firmir` checkout is configured and built.
./build/firmir-paint [output.png]        # default ./painting.png
./build/firmir-paint --selftest out.png  # headless canvas + PNG check
```
