#pragma once

#include <vector>
#include <optional>
#include <numeric>
#include "nisaba/types.hpp"
#include "nisaba/path/path.hpp"

namespace nisaba {

class StrokeDash {
public:
    static std::optional<StrokeDash> create(std::vector<float> dash_array, float dash_offset) noexcept;

    const std::vector<float>& array() const noexcept { return array_; }
    float offset() const noexcept { return offset_; }
    NonZeroPositiveF32 interval_len() const noexcept { return interval_len_; }
    float first_len() const noexcept { return first_len_; }
    size_t first_index() const noexcept { return first_index_; }

    bool operator==(const StrokeDash& o) const noexcept {
        return array_ == o.array_ && offset_ == o.offset_;
    }

    bool operator!=(const StrokeDash& o) const noexcept {
        return !(*this == o);
    }

private:
    StrokeDash(std::vector<float> arr, float off, NonZeroPositiveF32 int_len, float f_len, size_t f_idx) noexcept
        : array_(std::move(arr)), offset_(off), interval_len_(int_len), first_len_(f_len), first_index_(f_idx) {}

    std::vector<float> array_;
    float offset_{0.0f};
    NonZeroPositiveF32 interval_len_{NonZeroPositiveF32::create_unchecked(1.0f)};
    float first_len_{0.0f};
    size_t first_index_{0};

    friend std::optional<Path> dash_path(const Path& path, const StrokeDash& dash, float resolution_scale);
};

std::optional<Path> dash_path(const Path& path, const StrokeDash& dash, float resolution_scale);
const Path* dash_path_fast(const Path& path, const StrokeDash& dash, float resolution_scale);

} // namespace nisaba
