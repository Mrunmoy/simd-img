#pragma once

#include "simd_img/image.h"

namespace simd_img {

// Brightness: adds offset to each RGB channel, clamped to [0, 255].
// Range: [-255, 255]. Alpha is preserved.

namespace scalar {
void brightness(Image& img, int value);
}  // namespace scalar

namespace sse {
void brightness(Image& img, int value);
}  // namespace sse

}  // namespace simd_img
