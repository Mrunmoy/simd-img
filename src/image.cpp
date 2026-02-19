#include "simd_img/image.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace simd_img {

namespace {

// Round up to the nearest multiple of alignment. Uses bitmask trick which
// only works when alignment is a power of two (which it always is for SIMD).
uint32_t alignUp(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

// Standard malloc doesn't guarantee any particular alignment beyond what the
// platform requires (usually 8 or 16 bytes). For SSE/AVX we need 32-byte
// alignment, so we use platform-specific aligned allocation.
uint8_t* alignedMalloc(size_t size, size_t alignment)
{
    void* ptr = nullptr;
#if defined(_MSC_VER)
    ptr = _aligned_malloc(size, alignment);
#else
    if (posix_memalign(&ptr, alignment, size) != 0)
        ptr = nullptr;
#endif
    return static_cast<uint8_t*>(ptr);
}

}  // namespace

Image::Image(uint32_t width, uint32_t height)
    : m_width(width), m_height(height)
{
    allocate();
}

void Image::allocate()
{
    // SSE registers are 16 bytes, AVX registers are 32 bytes. When we load pixel
    // data into these registers, the CPU can do it faster if the memory address
    // is aligned to the register width. We pad each row's stride to 32 bytes so
    // that every row starts on an aligned boundary.
    m_stride = alignUp(m_width * kChannels, static_cast<uint32_t>(kAlignment));
    size_t total = static_cast<size_t>(m_stride) * m_height;
    if (total == 0)
        return;

    m_data.reset(alignedMalloc(total, kAlignment));
    if (!m_data)
        throw std::bad_alloc();

    std::memset(m_data.get(), 0, total);
}

void Image::fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    for (uint32_t y = 0; y < m_height; ++y)
    {
        uint8_t* px = row(y);
        for (uint32_t x = 0; x < m_width; ++x)
        {
            px[x * 4 + 0] = r;
            px[x * 4 + 1] = g;
            px[x * 4 + 2] = b;
            px[x * 4 + 3] = a;
        }
    }
}

Image Image::clone() const
{
    Image copy(m_width, m_height);
    std::memcpy(copy.m_data.get(), m_data.get(), sizeBytes());
    return copy;
}

Image Image::loadPpm(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file: " + path);

    std::string magic;
    file >> magic;
    if (magic != "P6")
        throw std::runtime_error("Not a P6 PPM file: " + path);

    // Skip comment lines
    char ch;
    file.get(ch);
    while (file.peek() == '#')
    {
        std::string comment;
        std::getline(file, comment);
    }

    uint32_t w = 0, h = 0;
    int maxVal = 0;
    file >> w >> h >> maxVal;
    if (maxVal != 255)
        throw std::runtime_error("Unsupported max value: " + std::to_string(maxVal));

    // Skip the single whitespace byte after the header
    file.get(ch);

    Image img(w, h);

    // PPM stores RGB — expand to RGBA on load
    std::vector<uint8_t> rgbRow(w * 3);
    for (uint32_t y = 0; y < h; ++y)
    {
        file.read(reinterpret_cast<char*>(rgbRow.data()), w * 3);
        uint8_t* dst = img.row(y);
        for (uint32_t x = 0; x < w; ++x)
        {
            dst[x * 4 + 0] = rgbRow[x * 3 + 0];
            dst[x * 4 + 1] = rgbRow[x * 3 + 1];
            dst[x * 4 + 2] = rgbRow[x * 3 + 2];
            dst[x * 4 + 3] = 255;
        }
    }

    return img;
}

void Image::savePpm(const std::string& path) const
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to create file: " + path);

    file << "P6\n" << m_width << " " << m_height << "\n255\n";

    // Strip alpha channel for PPM output
    std::vector<uint8_t> rgbRow(m_width * 3);
    for (uint32_t y = 0; y < m_height; ++y)
    {
        const uint8_t* src = row(y);
        for (uint32_t x = 0; x < m_width; ++x)
        {
            rgbRow[x * 3 + 0] = src[x * 4 + 0];
            rgbRow[x * 3 + 1] = src[x * 4 + 1];
            rgbRow[x * 3 + 2] = src[x * 4 + 2];
        }
        file.write(reinterpret_cast<const char*>(rgbRow.data()), m_width * 3);
    }
}

}  // namespace simd_img
