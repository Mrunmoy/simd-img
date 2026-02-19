#include "simd_img/image.h"
#include "simd_img/filters.h"

#include <gtest/gtest.h>
#include <cstring>

namespace {

simd_img::Image makeGradient(uint32_t w, uint32_t h)
{
    simd_img::Image img(w, h);
    for (uint32_t y = 0; y < h; ++y)
    {
        uint8_t* px = img.row(y);
        for (uint32_t x = 0; x < w; ++x)
        {
            px[x * 4 + 0] = static_cast<uint8_t>((x * 255) / (w - 1));
            px[x * 4 + 1] = static_cast<uint8_t>((y * 255) / (h - 1));
            px[x * 4 + 2] = static_cast<uint8_t>(((x + y) * 127) / (w + h - 2));
            px[x * 4 + 3] = 255;
        }
    }
    return img;
}

}  // namespace

// --- Image basics ---

TEST(Image, ConstructionAndDimensions)
{
    simd_img::Image img(64, 32);
    EXPECT_EQ(img.width(), 64u);
    EXPECT_EQ(img.height(), 32u);
    EXPECT_NE(img.data(), nullptr);
    EXPECT_GE(img.stride(), 64u * 4);
    EXPECT_EQ(img.stride() % 32, 0u);
}

TEST(Image, Fill)
{
    simd_img::Image img(64, 32);
    img.fill(10, 20, 30, 255);
    const uint8_t* px = img.row(0);
    EXPECT_EQ(px[0], 10);
    EXPECT_EQ(px[1], 20);
    EXPECT_EQ(px[2], 30);
    EXPECT_EQ(px[3], 255);
}

TEST(Image, Clone)
{
    simd_img::Image img(64, 32);
    img.fill(10, 20, 30, 255);
    simd_img::Image copy = img.clone();
    EXPECT_EQ(copy.width(), 64u);
    EXPECT_EQ(copy.height(), 32u);
    EXPECT_EQ(std::memcmp(copy.data(), img.data(), img.sizeBytes()), 0);
}

TEST(Image, PpmRoundtrip)
{
    simd_img::Image src = makeGradient(100, 80);
    src.savePpm("/tmp/simd_img_test.ppm");

    simd_img::Image loaded = simd_img::Image::loadPpm("/tmp/simd_img_test.ppm");
    EXPECT_EQ(loaded.width(), 100u);
    EXPECT_EQ(loaded.height(), 80u);

    for (uint32_t y = 0; y < src.height(); ++y)
    {
        const uint8_t* a = src.row(y);
        const uint8_t* b = loaded.row(y);
        for (uint32_t x = 0; x < src.width(); ++x)
        {
            for (int c = 0; c < 3; ++c)
                EXPECT_EQ(a[x * 4 + c], b[x * 4 + c]);
        }
    }
}

// --- Brightness ---

TEST(Brightness, SaturationClampsTo255)
{
    simd_img::Image img(16, 16);
    img.fill(250, 250, 250, 255);
    simd_img::scalar::brightness(img, 30);
    EXPECT_EQ(img.row(0)[0], 255);
}
