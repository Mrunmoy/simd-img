#include "simd_img/filters.h"

#include <algorithm>

namespace simd_img {
namespace scalar {

void brightness(Image& img, int value)
{
    for (uint32_t y = 0; y < img.height(); ++y)
    {
        uint8_t* px = img.row(y);
        for (uint32_t x = 0; x < img.width(); ++x)
        {
            // Adjust RGB channels, leave alpha alone
            for (int c = 0; c < 3; ++c)
            {
                int v = px[x * 4 + c] + value;
                px[x * 4 + c] = static_cast<uint8_t>(std::clamp(v, 0, 255));
            }
        }
    }
}

}  // namespace scalar
}  // namespace simd_img
