#include "simd_img/filters.h"

#include <algorithm>
#include <cmath>
#include <immintrin.h>

namespace simd_img {
namespace sse {

//
// This is the SSE version of brightness adjustment.
//
// The key insight: an SSE register (__m128i) is 128 bits = 16 bytes.
// One RGBA pixel is 4 bytes, so we can fit exactly 4 pixels in one register
// and process all of them with a single instruction.
//
// _mm_adds_epu8 does saturating unsigned addition — if the result would
// exceed 255, it clamps to 255 automatically. No manual clamping needed.
// _mm_subs_epu8 is the same idea for subtraction (clamps to 0).
//
void brightness(Image& img, int value)
{
    if (value == 0)
        return;

    // Build the per-pixel delta: [val, val, val, 0, val, val, val, 0, ...]
    // The 0 in every 4th byte means alpha stays untouched.
    uint8_t absVal = static_cast<uint8_t>(std::abs(value));
    __m128i delta = _mm_set_epi8(
        0, absVal, absVal, absVal,
        0, absVal, absVal, absVal,
        0, absVal, absVal, absVal,
        0, absVal, absVal, absVal);

    for (uint32_t y = 0; y < img.height(); ++y)
    {
        uint8_t* px = img.row(y);
        uint32_t x = 0;

        // Main loop: 4 pixels (16 bytes) per iteration
        for (; x + 4 <= img.width(); x += 4)
        {
            // Load 16 bytes from memory into an SSE register
            __m128i data = _mm_loadu_si128(
                reinterpret_cast<const __m128i*>(px + x * 4));

            // One instruction to add (or subtract) brightness to all 4 pixels
            __m128i result = (value > 0)
                ? _mm_adds_epu8(data, delta)
                : _mm_subs_epu8(data, delta);

            // Store the 16 bytes back to memory
            _mm_storeu_si128(reinterpret_cast<__m128i*>(px + x * 4), result);
        }

        // Handle remaining pixels that didn't fill a full 16-byte register.
        // If image width isn't a multiple of 4, we fall back to scalar here.
        for (; x < img.width(); ++x)
        {
            for (int c = 0; c < 3; ++c)
            {
                int v = px[x * 4 + c] + value;
                px[x * 4 + c] = static_cast<uint8_t>(std::clamp(v, 0, 255));
            }
        }
    }
}

}  // namespace sse
}  // namespace simd_img
