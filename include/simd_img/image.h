#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>

namespace simd_img {

// Custom deleter for memory allocated with posix_memalign / _aligned_malloc.
// Regular delete[] can't be used on aligned memory -- it must go through
// free() (or _aligned_free on MSVC). Wrapping this in a stateless struct
// gives us zero-overhead RAII via unique_ptr.
struct AlignedDeleter
{
    void operator()(uint8_t* ptr) const
    {
#if defined(_MSC_VER)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
};

using AlignedBuffer = std::unique_ptr<uint8_t[], AlignedDeleter>;

// RGBA pixel buffer with 32-byte aligned rows for SSE/AVX operations.
class Image
{
public:
    static constexpr uint32_t kChannels  = 4;   // RGBA
    static constexpr size_t   kAlignment = 32;  // AVX-friendly row alignment

    Image(uint32_t width, uint32_t height);
    ~Image() = default;

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&&) noexcept = default;
    Image& operator=(Image&&) noexcept = default;

    uint32_t    width()     const { return m_width; }
    uint32_t    height()    const { return m_height; }
    uint32_t    stride()    const { return m_stride; }  // bytes per row (may include padding)
    size_t      sizeBytes() const { return static_cast<size_t>(m_stride) * m_height; }

    uint8_t*       data()       { return m_data.get(); }
    const uint8_t* data() const { return m_data.get(); }

    uint8_t*       row(uint32_t y)       { return m_data.get() + y * m_stride; }
    const uint8_t* row(uint32_t y) const { return m_data.get() + y * m_stride; }

    void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // Deep copy
    Image clone() const;

    // PPM (Portable Pixmap) I/O.
    //
    // PPM is the simplest uncompressed image format: a short ASCII header
    // followed by raw pixel bytes. No library needed to read or write it.
    //
    // P6 binary format on disk:
    //   "P6\n"
    //   "<width> <height>\n"
    //   "255\n"
    //   <width * height * 3 bytes of RGB data>
    //
    // PPM only stores RGB (no alpha). On load, alpha is set to 255.
    // On save, the alpha channel is stripped.
    static Image loadPpm(const std::string& path);
    void savePpm(const std::string& path) const;

private:
    uint32_t      m_width  = 0;
    uint32_t      m_height = 0;
    uint32_t      m_stride = 0;
    AlignedBuffer m_data;

    void allocate();
};

}  // namespace simd_img
