#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <optional>
#include <string>
#include <span>
#include <cstring>
#include <algorithm>
#include <fstream>

namespace orionimg {

enum class PixelFormat : uint8_t {
    RGBA8888 = 0,
    BGRA8888 = 1,
    RGB888   = 2,
    Gray8    = 3
};

inline constexpr size_t bytes_per_pixel(PixelFormat format) noexcept {
    switch (format) {
        case PixelFormat::RGBA8888:
        case PixelFormat::BGRA8888: return 4;
        case PixelFormat::RGB888:   return 3;
        case PixelFormat::Gray8:    return 1;
    }
    return 4;
}

struct PremultipliedColorU8;

inline constexpr uint8_t premultiply_u8(uint8_t c, uint8_t a) noexcept {
    uint32_t prod = static_cast<uint32_t>(c) * static_cast<uint32_t>(a) + 128;
    return static_cast<uint8_t>((prod + (prod >> 8)) >> 8);
}

inline constexpr uint8_t rgb_to_gray8(uint8_t r, uint8_t g, uint8_t b) noexcept {
    return static_cast<uint8_t>((static_cast<uint32_t>(r) * 54 +
                                 static_cast<uint32_t>(g) * 183 +
                                 static_cast<uint32_t>(b) * 19) >> 8);
}

struct ColorU8 {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    uint8_t a{255};

    constexpr ColorU8() noexcept = default;
    constexpr ColorU8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept
        : r(r), g(g), b(b), a(a) {}

    static constexpr ColorU8 from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        return ColorU8(r, g, b, a);
    }

    constexpr uint8_t red() const noexcept { return r; }
    constexpr uint8_t green() const noexcept { return g; }
    constexpr uint8_t blue() const noexcept { return b; }
    constexpr uint8_t alpha() const noexcept { return a; }

    constexpr bool is_opaque() const noexcept { return a == 255; }
    constexpr bool operator==(const ColorU8& other) const noexcept = default;

    constexpr PremultipliedColorU8 premultiply() const noexcept;

    constexpr uint8_t to_gray8() const noexcept {
        return rgb_to_gray8(r, g, b);
    }
};

struct PremultipliedColorU8 {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    uint8_t a{255};

    constexpr PremultipliedColorU8() noexcept = default;
    constexpr PremultipliedColorU8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept
        : r(r), g(g), b(b), a(a) {}

    static constexpr PremultipliedColorU8 from_rgba_unchecked(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        return PremultipliedColorU8(r, g, b, a);
    }

    constexpr uint8_t red() const noexcept { return r; }
    constexpr uint8_t green() const noexcept { return g; }
    constexpr uint8_t blue() const noexcept { return b; }
    constexpr uint8_t alpha() const noexcept { return a; }

    constexpr bool is_opaque() const noexcept { return a == 255; }
    constexpr bool operator==(const PremultipliedColorU8& other) const noexcept = default;

    constexpr uint8_t to_gray8() const noexcept {
        return rgb_to_gray8(r, g, b);
    }

    ColorU8 demultiply() const noexcept {
        if (a == 255) return ColorU8(r, g, b, a);
        if (a == 0) return ColorU8(0, 0, 0, 0);
        double af = static_cast<double>(a) / 255.0;
        return ColorU8(
            static_cast<uint8_t>(static_cast<double>(r) / af + 0.5),
            static_cast<uint8_t>(static_cast<double>(g) / af + 0.5),
            static_cast<uint8_t>(static_cast<double>(b) / af + 0.5),
            a
        );
    }
};

inline constexpr PremultipliedColorU8 ColorU8::premultiply() const noexcept {
    if (a == 255) return PremultipliedColorU8(r, g, b, 255);
    if (a == 0) return PremultipliedColorU8(0, 0, 0, 0);
    return PremultipliedColorU8(premultiply_u8(r, a), premultiply_u8(g, a), premultiply_u8(b, a), a);
}

class Image;

class ImageRef {
public:
    constexpr ImageRef() noexcept = default;
    constexpr ImageRef(const uint8_t* data, size_t data_len, uint32_t width, uint32_t height, PixelFormat format = PixelFormat::RGBA8888) noexcept
        : data_(data), data_len_(data_len), width_(width), height_(height), format_(format) {}

    [[nodiscard]] constexpr uint32_t width() const noexcept { return width_; }
    [[nodiscard]] constexpr uint32_t height() const noexcept { return height_; }
    [[nodiscard]] constexpr PixelFormat format() const noexcept { return format_; }
    [[nodiscard]] constexpr size_t bytes_per_pixel() const noexcept { return orionimg::bytes_per_pixel(format_); }
    [[nodiscard]] constexpr size_t stride_bytes() const noexcept { return static_cast<size_t>(width_) * bytes_per_pixel(); }
    [[nodiscard]] constexpr bool is_empty() const noexcept { return width_ == 0 || height_ == 0 || data_ == nullptr; }
    [[nodiscard]] constexpr const uint8_t* data() const noexcept { return data_; }
    [[nodiscard]] constexpr size_t data_len() const noexcept { return data_len_; }

    [[nodiscard]] const PremultipliedColorU8* pixels() const noexcept {
        return reinterpret_cast<const PremultipliedColorU8*>(data_);
    }

    [[nodiscard]] const PremultipliedColorU8* row(size_t y) const noexcept {
        return pixels() + y * static_cast<size_t>(width_);
    }

    [[nodiscard]] std::optional<PremultipliedColorU8> pixel(uint32_t x, uint32_t y) const noexcept {
        if (x >= width_ || y >= height_ || !data_) return std::nullopt;
        if (format_ == PixelFormat::RGBA8888) {
            return pixels()[static_cast<size_t>(y) * width_ + x];
        } else if (format_ == PixelFormat::BGRA8888) {
            const auto* p = data_ + (static_cast<size_t>(y) * width_ + x) * 4;
            return PremultipliedColorU8::from_rgba_unchecked(p[2], p[1], p[0], p[3]);
        }
        return std::nullopt;
    }

    Image to_owned() const;
    Image to_rgba8888() const;

    bool save_ppm(const std::string& path) const;
    bool save_bmp(const std::string& path) const;

private:
    const uint8_t* data_{nullptr};
    size_t data_len_{0};
    uint32_t width_{0};
    uint32_t height_{0};
    PixelFormat format_{PixelFormat::RGBA8888};
};

class Image {
public:
    Image() = default;

    static std::optional<Image> allocate(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::RGBA8888) {
        if (width == 0 || height == 0) return std::nullopt;
        size_t bpp = orionimg::bytes_per_pixel(format);
        size_t total = static_cast<size_t>(width) * height * bpp;
        Image img;
        img.width_ = width;
        img.height_ = height;
        img.format_ = format;
        img.data_.resize(total, 0);
        return img;
    }

    static std::optional<Image> from_vec(std::vector<uint8_t> data, uint32_t width, uint32_t height, PixelFormat format = PixelFormat::RGBA8888) {
        size_t expected = static_cast<size_t>(width) * height * orionimg::bytes_per_pixel(format);
        if (data.size() < expected) return std::nullopt;
        Image img;
        img.width_ = width;
        img.height_ = height;
        img.format_ = format;
        img.data_ = std::move(data);
        return img;
    }

    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] PixelFormat format() const noexcept { return format_; }
    [[nodiscard]] size_t bytes_per_pixel() const noexcept { return orionimg::bytes_per_pixel(format_); }
    [[nodiscard]] size_t stride_bytes() const noexcept { return static_cast<size_t>(width_) * bytes_per_pixel(); }
    [[nodiscard]] bool is_empty() const noexcept { return width_ == 0 || height_ == 0 || data_.empty(); }

    [[nodiscard]] const uint8_t* data() const noexcept { return data_.data(); }
    [[nodiscard]] uint8_t* data_mut() noexcept { return data_.data(); }
    [[nodiscard]] size_t data_len() const noexcept { return data_.size(); }
    [[nodiscard]] size_t size_bytes() const noexcept { return data_.size(); }

    [[nodiscard]] const PremultipliedColorU8* pixels() const noexcept {
        return reinterpret_cast<const PremultipliedColorU8*>(data_.data());
    }

    [[nodiscard]] PremultipliedColorU8* pixels_mut() noexcept {
        return reinterpret_cast<PremultipliedColorU8*>(data_.data());
    }

    [[nodiscard]] const PremultipliedColorU8* row(size_t y) const noexcept {
        return pixels() + y * static_cast<size_t>(width_);
    }

    [[nodiscard]] PremultipliedColorU8* row_mut(size_t y) noexcept {
        return pixels_mut() + y * static_cast<size_t>(width_);
    }

    [[nodiscard]] std::optional<PremultipliedColorU8> pixel(uint32_t x, uint32_t y) const noexcept {
        return as_ref().pixel(x, y);
    }

    bool set_pixel(uint32_t x, uint32_t y, PremultipliedColorU8 color) noexcept {
        if (x >= width_ || y >= height_) return false;
        if (format_ == PixelFormat::RGBA8888) {
            pixels_mut()[static_cast<size_t>(y) * width_ + x] = color;
            return true;
        }
        return false;
    }

    [[nodiscard]] ImageRef as_ref() const noexcept {
        return ImageRef(data_.data(), data_.size(), width_, height_, format_);
    }

    Image to_rgba8888() const {
        if (format_ == PixelFormat::RGBA8888) return *this;
        auto opt = allocate(width_, height_, PixelFormat::RGBA8888);
        if (!opt) return {};
        Image res = std::move(*opt);
        PremultipliedColorU8* dst = res.pixels_mut();
        for (uint32_t y = 0; y < height_; ++y) {
            for (uint32_t x = 0; x < width_; ++x) {
                auto p = pixel(x, y);
                dst[y * width_ + x] = p.value_or(PremultipliedColorU8(0, 0, 0, 255));
            }
        }
        return res;
    }

    std::vector<uint8_t> take() && noexcept {
        return std::move(data_);
    }

    bool save_ppm(const std::string& path) const {
        return as_ref().save_ppm(path);
    }

    bool save_bmp(const std::string& path) const {
        return as_ref().save_bmp(path);
    }

private:
    uint32_t width_{0};
    uint32_t height_{0};
    PixelFormat format_{PixelFormat::RGBA8888};
    std::vector<uint8_t> data_;
};

inline Image ImageRef::to_owned() const {
    if (is_empty()) return {};
    auto opt = Image::allocate(width_, height_, format_);
    if (!opt) return {};
    Image res = std::move(*opt);
    std::memcpy(res.data_mut(), data_, std::min(data_len_, res.data_len()));
    return res;
}

inline Image ImageRef::to_rgba8888() const {
    if (format_ == PixelFormat::RGBA8888) return to_owned();
    auto opt = Image::allocate(width_, height_, PixelFormat::RGBA8888);
    if (!opt) return {};
    Image res = std::move(*opt);
    PremultipliedColorU8* dst = res.pixels_mut();
    for (uint32_t y = 0; y < height_; ++y) {
        for (uint32_t x = 0; x < width_; ++x) {
            auto p = pixel(x, y);
            dst[y * width_ + x] = p.value_or(PremultipliedColorU8(0, 0, 0, 255));
        }
    }
    return res;
}

inline bool ImageRef::save_ppm(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;
    out << "P6\n" << width() << " " << height() << "\n255\n";
    for (uint32_t y = 0; y < height(); ++y) {
        for (uint32_t x = 0; x < width(); ++x) {
            auto p = pixel(x, y);
            if (p) {
                ColorU8 c = p->demultiply();
                uint8_t rgb[3] = {c.red(), c.green(), c.blue()};
                out.write(reinterpret_cast<const char*>(rgb), 3);
            }
        }
    }
    return out.good();
}

inline bool ImageRef::save_bmp(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;
    uint32_t w = width();
    uint32_t h = height();
    uint32_t row_padded = (w * 3 + 3) & (~3);
    uint32_t image_size = row_padded * h;
    uint32_t file_size = 54 + image_size;

    uint8_t header[54] = {
        'B', 'M',
        static_cast<uint8_t>(file_size), static_cast<uint8_t>(file_size >> 8), static_cast<uint8_t>(file_size >> 16), static_cast<uint8_t>(file_size >> 24),
        0, 0, 0, 0,
        54, 0, 0, 0,
        40, 0, 0, 0,
        static_cast<uint8_t>(w), static_cast<uint8_t>(w >> 8), static_cast<uint8_t>(w >> 16), static_cast<uint8_t>(w >> 24),
        static_cast<uint8_t>(h), static_cast<uint8_t>(h >> 8), static_cast<uint8_t>(h >> 16), static_cast<uint8_t>(h >> 24),
        1, 0,
        24, 0,
        0, 0, 0, 0,
        static_cast<uint8_t>(image_size), static_cast<uint8_t>(image_size >> 8), static_cast<uint8_t>(image_size >> 16), static_cast<uint8_t>(image_size >> 24),
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };
    out.write(reinterpret_cast<const char*>(header), 54);

    std::vector<uint8_t> row(row_padded, 0);
    for (int32_t y = static_cast<int32_t>(h) - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto p = pixel(x, static_cast<uint32_t>(y));
            if (p) {
                ColorU8 c = p->demultiply();
                row[x * 3 + 0] = c.blue();
                row[x * 3 + 1] = c.green();
                row[x * 3 + 2] = c.red();
            }
        }
        out.write(reinterpret_cast<const char*>(row.data()), row_padded);
    }
    return out.good();
}

} // namespace orionimg
