# simd-img

An image processing library in C++ where I'm learning SSE intrinsics from
scratch. Each filter is implemented twice -- a plain scalar loop and an SSE
version -- so I can verify correctness by comparing outputs and measure the
actual speedup.

## Background -- what is SIMD and why bother

Say I want to brighten an image. I loop over every pixel, add some value to
each color channel, and clamp it. A 1920x1080 image has about 2 million pixels,
each with 3 color channels. That's 6 million additions and 6 million clamp
operations. One at a time.

The CPU can do better than that.

Normal CPU registers are 32 or 64 bits wide. They hold one value. But modern
x86 CPUs also have SSE registers that are **128 bits wide**. They can hold 16
bytes at once. The type in C++ is `__m128i` (for integers) or `__m128` (for
floats).

One RGBA pixel is 4 bytes: `[R, G, B, A]`. So one 128-bit register holds
exactly 4 pixels:

```
One __m128i register (128 bits = 16 bytes):

byte:  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
     [ R0 | G0 | B0 | A0 | R1 | G1 | B1 | A1 | R2 | G2 | B2 | A2 | R3 | G3 | B3 | A3 ]
       ---- pixel 0 ----   ---- pixel 1 ----   ---- pixel 2 ----   ---- pixel 3 ----
```

Instead of processing one byte at a time, I pack 16 bytes into one register and
operate on all of them with a single instruction. That's SIMD -- Single
Instruction, Multiple Data.

## The pattern -- load, math, store

Every SIMD operation I've written so far follows the same three steps:

```cpp
// 1. Load 16 bytes (4 RGBA pixels) from memory into a register
__m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));

// 2. Do math on all 16 bytes at once
__m128i result = _mm_adds_epu8(data, delta);

// 3. Store the register back to memory
_mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), result);
```

`_mm_adds_epu8` adds two registers byte-by-byte with saturation -- if a byte
would exceed 255, it clamps to 255 automatically instead of wrapping. Exactly
what I need for pixel brightness without any manual clamping logic.

## Reading intrinsic names

The names look cryptic but follow a pattern:

```
_mm_adds_epu8
 |   |    |  |
 |   |    |  +-- 8-bit elements
 |   |    +----- unsigned (u), packed (p), extended (e)
 |   +---------- add with saturation
 +-------------- SSE prefix (128-bit)
```

Common suffixes I keep running into:
- `epi8` / `epu8` -- 8-bit signed / unsigned
- `epi16` / `epu16` -- 16-bit
- `epi32` -- 32-bit integers
- `ps` -- packed 32-bit floats

The Intel Intrinsics Guide is the reference I use constantly -- search for a
function name and it shows exactly what it does with a diagram.

## The scalar tail

If the image is 130 pixels wide, I process 32 iterations of 4 pixels (128
pixels), then 2 pixels are left over. They don't fill a full register, so I
fall back to a scalar loop at the end:

```cpp
for (; x < img.width(); ++x) { /* scalar fallback */ }
```

Every SIMD function needs this unless the data size is always a multiple of the
processing width.


## Prerequisites

- A C++ compiler with C++20 and SSE4.2 support (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.14 or later
- Python 3 (for the build script)

On Ubuntu/Debian:

```bash
sudo apt install build-essential cmake python3
```

On macOS (with Homebrew):

```bash
brew install cmake python3
```

SSE4.2 is supported on virtually all x86-64 CPUs from the last 15 years.
This project does not support ARM (no NEON port).

## Build

```bash
./build.py -b             # configure and compile (Release)
./build.py -b --debug     # compile in Debug mode
./build.py -t             # run unit tests
./build.py -c             # remove build directory
./build.py -r             # clean + build
./build.py -b -t          # build then test
./build.py -r -t          # clean, build, test
```

Or manually:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./simd_img_tests
```

## Design docs

- [High Level Design](doc/high_level_design.md)
- [Low Level Design](doc/low_level_design.md)
