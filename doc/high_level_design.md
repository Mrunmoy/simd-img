# High Level Design

## Overview

simd-img is an image processing library where every pixel operation is
implemented twice: once as a plain scalar loop, once using SSE intrinsics.
The scalar version acts as a correctness reference. Unit tests compare the
two outputs to make sure the SSE path produces the same result.

The library uses RGBA pixel buffers internally. File I/O is PPM (P6 binary)
to keep dependencies at zero.

## Component layout

```
Application (tests, benchmarks)
        |
   filters.h    -- pixel operations (scalar + SSE)
        |
    image.h     -- RGBA buffer, PPM I/O
```

`image.h` / `image.cpp` -- Owns the pixel buffer. Rows are 32-byte aligned
for SIMD loads. Handles PPM read/write (RGB on disk, RGBA in memory).

`filters.h` -- Declares the filter functions. Scalar implementations live in
`filters_scalar.cpp`, SSE implementations in `filters_sse.cpp`.

## Memory layout

Image rows are padded to 32-byte alignment so that SSE/AVX loads don't
straddle alignment boundaries unnecessarily. The stride (bytes per row) may
be larger than `width * 4` because of this padding. Allocation goes through
`posix_memalign` with RAII cleanup.

## What's implemented

- Image buffer with PPM I/O
- Brightness (scalar + SSE)
