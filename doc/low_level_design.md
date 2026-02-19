# Low Level Design

## Image class

### Memory

```
stride = align_up(width * 4, 32)
data   = posix_memalign(stride * height, 32)
```

Move-only. Destructor calls `aligned_free`. `clone()` does a deep copy via
`memcpy`.

### PPM I/O

Load reads the P6 header (skipping comment lines), then reads RGB triples
one row at a time and expands to RGBA with alpha = 255. Save does the
reverse -- strips alpha and writes RGB.

## Why PPM for images

I needed a file format to load and save test images. PPM (Portable Pixmap) is
the simplest image format that exists -- a short ASCII header followed by raw
pixel bytes. No compression, no library dependency, trivially readable.

A P6 (binary) PPM file looks like this on disk:

```
P6
1920 1080
255
<1920 * 1080 * 3 bytes of raw RGB data>
```

That's it. The header is human-readable text, the body is just the pixel values
packed sequentially. I can write a loader in 20 lines of code without pulling in
libpng or stb_image.

The one limitation is that PPM only stores RGB -- no alpha channel. So when
loading, I expand to RGBA (alpha = 255), and when saving, I strip the alpha.


## Brightness

### Scalar version

Loop over every pixel, add the brightness value to R/G/B, clamp to [0, 255].
Alpha left alone.

### SSE version

An SSE register is 128 bits = 16 bytes = 4 RGBA pixels. The idea is to
load 4 pixels at once, apply the brightness offset to all of them in one
instruction, then store back.

The brightness delta is set up as `[val, val, val, 0]` repeated 4 times.
The zero in each alpha position means alpha doesn't change.

For positive brightness, `_mm_adds_epu8` does a saturating add -- values
that would go above 255 get clamped automatically. For negative brightness,
`_mm_subs_epu8` does the same but clamping at 0.

If the image width isn't a multiple of 4, the leftover pixels at the end of
each row are handled by a scalar tail loop.
