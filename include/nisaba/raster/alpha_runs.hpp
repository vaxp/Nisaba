#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <span>
#include <cassert>
#include "nisaba/types.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/raster/blitter.hpp"

namespace nisaba {

/// Sparse array of run-length-encoded alpha (coverage) values.
///
/// Sparseness allows us to independently compose several spans into the same buffer.
class AlphaRuns {
public:
    std::vector<AlphaRun> runs;
    std::vector<uint8_t> alpha;

    AlphaRuns() = default;

    explicit AlphaRuns(LengthU32 width) {
        reinit(width);
    }

    void reinit(LengthU32 width) {
        size_t len = static_cast<size_t>(width.get()) + 1;
        if (runs.size() < len) {
            runs.resize(len, std::nullopt);
            alpha.resize(len, 0);
        }
        reset(width);
    }

    /// Returns 0-255 given 0-256.
    static constexpr AlphaU8 catch_overflow(uint16_t a) noexcept {
        assert(a <= 256);
        return static_cast<AlphaU8>(a - (a >> 8));
    }

    /// Returns true if the scanline contains only a single run of alpha value 0.
    bool is_empty() const noexcept {
        assert(runs[0].has_value());
        if (!runs[0].has_value()) return true;
        size_t run_len = static_cast<size_t>(*runs[0]);
        return alpha[0] == 0 && !runs[run_len].has_value();
    }

    /// Reinitialize for a new scanline.
    void reset(LengthU32 width) noexcept {
        runs[0] = static_cast<uint16_t>(width.get());
        runs[width.get()] = std::nullopt;
        alpha[0] = 0;
    }

    /// Insert into the buffer a run starting at (x - offset_x).
    size_t add(
        uint32_t x_in,
        AlphaU8 start_alpha,
        size_t middle_count,
        AlphaU8 stop_alpha,
        uint8_t max_value,
        size_t offset_x
    ) {
        size_t x = static_cast<size_t>(x_in);
        size_t runs_offset = offset_x;
        size_t alpha_offset = offset_x;
        size_t last_alpha_offset = offset_x;
        x -= offset_x;

        if (start_alpha != 0) {
            break_run(
                std::span<AlphaRun>(runs.data() + runs_offset, runs.size() - runs_offset),
                std::span<uint8_t>(alpha.data() + alpha_offset, alpha.size() - alpha_offset),
                x,
                1
            );

            uint16_t tmp = static_cast<uint16_t>(alpha[alpha_offset + x]) + static_cast<uint16_t>(start_alpha);
            assert(tmp <= 256);
            alpha[alpha_offset + x] = static_cast<uint8_t>(tmp - (tmp >> 8));

            runs_offset += x + 1;
            alpha_offset += x + 1;
            x = 0;
        }

        if (middle_count != 0) {
            break_run(
                std::span<AlphaRun>(runs.data() + runs_offset, runs.size() - runs_offset),
                std::span<uint8_t>(alpha.data() + alpha_offset, alpha.size() - alpha_offset),
                x,
                middle_count
            );
            alpha_offset += x;
            runs_offset += x;
            x = 0;

            while (true) {
                uint8_t a = catch_overflow(
                    static_cast<uint16_t>(alpha[alpha_offset]) + static_cast<uint16_t>(max_value)
                );
                alpha[alpha_offset] = a;

                assert(runs[runs_offset].has_value());
                size_t n = static_cast<size_t>(*runs[runs_offset]);
                assert(n <= middle_count);
                alpha_offset += n;
                runs_offset += n;
                middle_count -= n;

                if (middle_count == 0) {
                    break;
                }
            }

            last_alpha_offset = alpha_offset;
        }

        if (stop_alpha != 0) {
            break_run(
                std::span<AlphaRun>(runs.data() + runs_offset, runs.size() - runs_offset),
                std::span<uint8_t>(alpha.data() + alpha_offset, alpha.size() - alpha_offset),
                x,
                1
            );
            alpha_offset += x;
            alpha[alpha_offset] = static_cast<uint8_t>(alpha[alpha_offset] + stop_alpha);
            last_alpha_offset = alpha_offset;
        }

        return last_alpha_offset;
    }

    /// Break the runs in the buffer at offsets x and x + count.
    static void break_run(std::span<AlphaRun> r, std::span<uint8_t> a, size_t x, size_t count) {
        assert(count > 0);

        size_t orig_x = x;
        size_t runs_offset = 0;
        size_t alpha_offset = 0;

        while (x > 0) {
            assert(r[runs_offset].has_value());
            size_t n = static_cast<size_t>(*r[runs_offset]);
            assert(n > 0);

            if (x < n) {
                a[alpha_offset + x] = a[alpha_offset];
                r[runs_offset + 0] = static_cast<uint16_t>(x);
                r[runs_offset + x] = static_cast<uint16_t>(n - x);
                break;
            }
            runs_offset += n;
            alpha_offset += n;
            x -= n;
        }

        runs_offset = orig_x;
        alpha_offset = orig_x;
        x = count;

        while (true) {
            assert(r[runs_offset].has_value());
            size_t n = static_cast<size_t>(*r[runs_offset]);
            assert(n > 0);

            if (x < n) {
                a[alpha_offset + x] = a[alpha_offset];
                r[runs_offset + 0] = static_cast<uint16_t>(x);
                r[runs_offset + x] = static_cast<uint16_t>(n - x);
                break;
            }

            x -= n;
            if (x == 0) {
                break;
            }

            runs_offset += n;
            alpha_offset += n;
        }
    }

    /// Cut run at offset x into two shorter runs.
    static void break_at(std::span<AlphaU8> a, std::span<AlphaRun> r, int32_t x) {
        size_t alpha_i = 0;
        size_t run_i = 0;
        while (x > 0) {
            assert(r[run_i].has_value());
            uint16_t n = *r[run_i];
            size_t n_usize = static_cast<size_t>(n);
            int32_t n_i32 = static_cast<int32_t>(n);
            if (x < n_i32) {
                a[alpha_i + static_cast<size_t>(x)] = a[alpha_i];
                r[0] = static_cast<uint16_t>(x);
                r[static_cast<size_t>(x)] = static_cast<uint16_t>(n_i32 - x);
                break;
            }

            run_i += n_usize;
            alpha_i += n_usize;
            x -= n_i32;
        }
    }
};

} // namespace nisaba
