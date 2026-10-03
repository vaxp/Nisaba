#include "nisaba/raster/scan.hpp"
#include "nisaba/raster/alpha_runs.hpp"
#include "nisaba/raster/edge_builder.hpp"
#include "nisaba/raster/fixed_point.hpp"
#include "nisaba/math/scalar.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/pipeline/simd.hpp"
#include <algorithm>
#include <cassert>

namespace nisaba {

namespace {

constexpr uint32_t SUPERSAMPLE_SHIFT = 2;
constexpr uint32_t SHIFT = SUPERSAMPLE_SHIFT;
constexpr uint32_t SCALE = 1 << SHIFT;
constexpr uint32_t MASK = SCALE - 1;

constexpr double CONSERVATIVE_ROUND_BIAS = 0.5 + 1.5 / 64.0;

inline int32_t round_down_to_int(float x) noexcept {
    double xx = static_cast<double>(x) - CONSERVATIVE_ROUND_BIAS;
    return scalar::saturate_from(static_cast<float>(std::ceil(xx)));
}

inline int32_t round_up_to_int(float x) noexcept {
    double xx = static_cast<double>(x) + CONSERVATIVE_ROUND_BIAS;
    return scalar::saturate_from(static_cast<float>(std::floor(xx)));
}

inline std::optional<IntRect> conservative_round_to_int(const Rect& src) noexcept {
    return IntRect::from_ltrb(
        round_down_to_int(src.left()),
        round_down_to_int(src.top()),
        round_up_to_int(src.right()),
        round_up_to_int(src.bottom())
    );
}

inline AlphaU8 coverage_to_partial_alpha(uint32_t aa) noexcept {
    aa <<= (8 - 2 * SHIFT);
    return static_cast<AlphaU8>(aa);
}

inline void remove_edge(uint32_t curr_idx, Edge* edges) noexcept {
    uint32_t prev = edges[curr_idx]->prev;
    uint32_t next = edges[curr_idx]->next;
    edges[prev]->next = next;
    edges[next]->prev = prev;
}

inline void insert_edge_after(uint32_t curr_idx, uint32_t after_idx, Edge* edges) noexcept {
    uint32_t after_next = edges[after_idx]->next;
    edges[curr_idx]->prev = after_idx;
    edges[curr_idx]->next = after_next;
    edges[after_next]->prev = curr_idx;
    edges[after_idx]->next = curr_idx;
}

inline void backward_insert_edge_based_on_x(uint32_t curr_idx, Edge* edges) noexcept {
    FDot16 x = edges[curr_idx]->x;
    uint32_t prev_idx = edges[curr_idx]->prev;
    if (prev_idx == 0) return;

    uint32_t p_prev = edges[prev_idx]->prev;
    if (p_prev == 0 || edges[p_prev]->x <= x) {
        uint32_t c_next = edges[curr_idx]->next;
        edges[p_prev]->next = curr_idx;
        edges[curr_idx]->prev = p_prev;
        edges[curr_idx]->next = prev_idx;
        edges[prev_idx]->prev = curr_idx;
        edges[prev_idx]->next = c_next;
        edges[c_next]->prev = prev_idx;
        return;
    }

    uint32_t cur_walk = p_prev;
    while (cur_walk != 0 && edges[cur_walk]->x > x) {
        cur_walk = edges[cur_walk]->prev;
    }

    remove_edge(curr_idx, edges);
    insert_edge_after(curr_idx, cur_walk, edges);
}

inline uint32_t backward_insert_start(uint32_t prev_idx, FDot16 x, const Edge* edges) noexcept {
    while (prev_idx != 0) {
        prev_idx = edges[prev_idx]->prev;
        if (edges[prev_idx]->x <= x) {
            break;
        }
    }
    return prev_idx;
}

inline void insert_new_edges(uint32_t new_idx, int32_t curr_y, Edge* edges) noexcept {
    if (edges[new_idx]->first_y != curr_y) return;

    uint32_t prev_idx = edges[new_idx]->prev;
    if (edges[prev_idx]->x <= edges[new_idx]->x) return;

    uint32_t start_idx = backward_insert_start(prev_idx, edges[new_idx]->x, edges);

    while (true) {
        uint32_t next_idx = edges[new_idx]->next;
        bool keep_edge = false;
        while (true) {
            uint32_t after_idx = edges[start_idx]->next;
            if (after_idx == new_idx) {
                keep_edge = true;
                break;
            }
            if (edges[after_idx]->x >= edges[new_idx]->x) {
                break;
            }
            start_idx = after_idx;
        }

        if (!keep_edge) {
            remove_edge(new_idx, edges);
            insert_edge_after(new_idx, start_idx, edges);
        }

        start_idx = new_idx;
        new_idx = next_idx;

        if (edges[new_idx]->first_y != curr_y) {
            break;
        }
    }
}

template <typename BlitterType>
void walk_edges(
    FillRule fill_rule,
    uint32_t start_y,
    uint32_t stop_y,
    uint32_t right_clip,
    std::vector<Edge>& edges,
    BlitterType& blitter
) {
    uint32_t curr_y = start_y;
    int32_t winding_mask = (fill_rule == FillRule::EvenOdd) ? 1 : -1;
    Edge* e_data = edges.data();

    while (true) {
        int32_t w = 0;
        uint32_t left = 0;
        FDot16 prev_x = e_data[0]->x;

        uint32_t curr_idx = e_data[0]->next;
        while (e_data[curr_idx]->first_y <= static_cast<int32_t>(curr_y)) {
            LineEdge& cur_edge = e_data[curr_idx].as_line_mut();

            uint32_t x = static_cast<uint32_t>(fdot16::round_to_i32(cur_edge.x));

            if ((w & winding_mask) == 0) {
                left = x;
            }

            w += cur_edge.winding;

            if ((w & winding_mask) == 0) {
                if (x > left) {
                    blitter.blit_h(left, curr_y, LengthU32::create_unchecked(x - left));
                }
            }

            uint32_t next_idx = cur_edge.next;
            FDot16 new_x = 0;

            if (cur_edge.last_y == static_cast<int32_t>(curr_y)) {
                remove_edge(curr_idx, e_data);
            } else {
                new_x = cur_edge.x + cur_edge.dx;
                cur_edge.x = new_x;

                if (new_x < prev_x) {
                    backward_insert_edge_based_on_x(curr_idx, e_data);
                } else {
                    prev_x = new_x;
                }
            }

            curr_idx = next_idx;
        }

        if ((w & winding_mask) != 0) {
            if (right_clip > left) {
                blitter.blit_h(left, curr_y, LengthU32::create_unchecked(right_clip - left));
            }
        }

        curr_y += 1;
        if (curr_y >= stop_y) {
            break;
        }

        insert_new_edges(curr_idx, static_cast<int32_t>(curr_y), e_data);
    }
}

class SuperBlitter : public Blitter {
public:
    SuperBlitter(const IntRect& bounds, const ScreenIntRect& clip_rect, Blitter& blitter)
        : real_blitter_(blitter) {
        auto sect = to_screen_int_rect(*bounds.intersect(clip_rect.to_int_rect()));
        if (!sect) return;

        valid_ = true;
        width_ = sect->width_safe();
        left_ = sect->left();
        super_left_ = sect->left() << SHIFT;
        top_ = static_cast<int32_t>(sect->top());
        curr_iy_ = top_ - 1;

        if (tl_cov_.size() < width_.get()) {
            tl_cov_.resize(std::max(tl_cov_.size() * 2, static_cast<size_t>(width_.get()) + 64), 0);
        }
        min_x_ = width_.get();
        max_x_ = 0;
    }

    ~SuperBlitter() override {
        flush();
    }

    bool is_valid() const noexcept { return valid_; }

    void blit_h(uint32_t x, uint32_t y, LengthU32 width) override {
        int32_t iy = static_cast<int32_t>(y >> SHIFT);

        if (x >= super_left_) {
            x -= super_left_;
        } else {
            width = LengthU32::create_unchecked(x + width.get());
            x = 0;
        }

        if (iy != curr_iy_) {
            flush();
            curr_iy_ = iy;
        }

        uint32_t start = x;
        uint32_t stop = x + width.get();
        uint32_t w_limit = width_.get();
        uint32_t x0 = start >> SHIFT;
        uint32_t x1 = stop >> SHIFT;

        if (x0 >= w_limit) return;
        if (x1 > w_limit) x1 = w_limit;

        uint32_t end_x = std::min(x1 + 1, w_limit);
        min_x_ = std::min(min_x_, x0);
        max_x_ = std::max(max_x_, end_x);

        if (!overflow_ranges_) {
            if (num_ranges_ < MAX_RANGES) {
                ranges_[num_ranges_++] = {x0, end_x};
            } else {
                overflow_ranges_ = true;
            }
        }

        uint8_t* cov_ptr = tl_cov_.data();

        if (x0 == x1) {
            uint32_t c = stop - start;
            cov_ptr[x0] = static_cast<uint8_t>(std::min(255u, static_cast<uint32_t>(cov_ptr[x0]) + (c << 4)));
            return;
        }

        uint32_t fb = SCALE - (start & MASK);
        cov_ptr[x0] = static_cast<uint8_t>(std::min(255u, static_cast<uint32_t>(cov_ptr[x0]) + (fb << 4)));

        uint32_t len = x1 - (x0 + 1);
        if (len > 0) {
            uint8_t* p = cov_ptr + (x0 + 1);
#if defined(NISABA_HAS_AVX2)
            __m256i add_val = _mm256_set1_epi8(64);
            while (len >= 32) {
                __m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(p), _mm256_adds_epu8(v, add_val));
                p += 32;
                len -= 32;
            }
#endif
#if defined(NISABA_HAS_SSE2) || defined(NISABA_HAS_AVX2)
            if (len >= 16) {
                __m128i add_val128 = _mm_set1_epi8(64);
                __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
                _mm_storeu_si128(reinterpret_cast<__m128i*>(p), _mm_adds_epu8(v, add_val128));
                p += 16;
                len -= 16;
            }
            if (len >= 8) {
                __m128i add_val128 = _mm_set1_epi8(64);
                __m128i v = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(p));
                v = _mm_adds_epu8(v, add_val128);
                _mm_storel_epi64(reinterpret_cast<__m128i*>(p), v);
                p += 8;
                len -= 8;
            }
#endif
            while (len > 0) {
                *p = static_cast<uint8_t>(std::min(255u, static_cast<uint32_t>(*p) + 64u));
                ++p;
                --len;
            }
        }

        if (x1 < w_limit) {
            uint32_t fe = stop & MASK;
            if (fe > 0) {
                cov_ptr[x1] = static_cast<uint8_t>(std::min(255u, static_cast<uint32_t>(cov_ptr[x1]) + (fe << 4)));
            }
        }
    }

    void flush() {
        if (curr_iy_ >= top_ && max_x_ > min_x_) {
            uint8_t* cov_ptr = tl_cov_.data();
            if (!overflow_ranges_ && num_ranges_ > 0) {
                std::sort(ranges_, ranges_ + num_ranges_, [](const Range& a, const Range& b) {
                    return a.x0 < b.x0;
                });
                size_t out_r = 0;
                for (size_t k = 1; k < num_ranges_; ++k) {
                    if (ranges_[k].x0 <= ranges_[out_r].x1) {
                        ranges_[out_r].x1 = std::max(ranges_[out_r].x1, ranges_[k].x1);
                    } else {
                        ++out_r;
                        ranges_[out_r] = ranges_[k];
                    }
                }
                size_t total_merged = out_r + 1;
                for (size_t k = 0; k < total_merged; ++k) {
                    uint32_t rx0 = ranges_[k].x0;
                    uint32_t rlen = ranges_[k].x1 - rx0;
                    real_blitter_.blit_span_coverage(left_ + rx0, static_cast<uint32_t>(curr_iy_), cov_ptr + rx0, rlen);
                    std::memset(cov_ptr + rx0, 0, rlen);
                }
            } else {
                uint32_t count = max_x_ - min_x_;
                real_blitter_.blit_span_coverage(
                    left_ + min_x_,
                    static_cast<uint32_t>(curr_iy_),
                    cov_ptr + min_x_,
                    count
                );
                std::memset(cov_ptr + min_x_, 0, count);
            }
            min_x_ = width_.get();
            max_x_ = 0;
            num_ranges_ = 0;
            overflow_ranges_ = false;
            curr_iy_ = top_ - 1;
        }
    }

private:
    struct Range {
        uint32_t x0;
        uint32_t x1;
    };
    static constexpr size_t MAX_RANGES = 64;
    Range ranges_[MAX_RANGES];
    size_t num_ranges_{0};
    bool overflow_ranges_{false};

    Blitter& real_blitter_;
    bool valid_{false};
    LengthU32 width_{LengthU32::create_unchecked(1)};
    uint32_t left_{0};
    uint32_t super_left_{0};
    int32_t curr_iy_{-1};
    int32_t top_{0};
    uint32_t min_x_{0};
    uint32_t max_x_{0};
    static inline thread_local std::vector<uint8_t> tl_cov_{};
};

template <typename BlitterType>
void fill_path_impl(
    const Path& path,
    FillRule fill_rule,
    const ScreenIntRect& clip_rect,
    int32_t start_y,
    int32_t stop_y,
    int32_t shift_edges_up,
    bool path_contained_in_clip,
    BlitterType& blitter
) {
    auto shifted_clip = ShiftedIntRect::create(clip_rect, shift_edges_up);
    if (!shifted_clip) return;

    const ShiftedIntRect* clip_ptr = path_contained_in_clip ? nullptr : &(*shifted_clip);
    auto edges_opt = BasicEdgeBuilder::build_edges_inplace(path, clip_ptr, shift_edges_up);
    if (!edges_opt) return;

    std::vector<Edge>& edges = **edges_opt;

    std::sort(edges.begin() + 1, edges.end(), [](const Edge& a, const Edge& b) {
        if (a->first_y == b->first_y) {
            return a->x < b->x;
        }
        return a->first_y < b->first_y;
    });

    for (size_t i = 1; i < edges.size(); ++i) {
        edges[i]->prev = static_cast<uint32_t>(i - 1);
        edges[i]->next = static_cast<uint32_t>(i + 1);
    }

    LineEdge tail{};
    tail.prev = static_cast<uint32_t>(edges.size() - 1);
    tail.next = 0;
    tail.first_y = std::numeric_limits<int32_t>::max();
    edges.push_back(Edge(tail));

    start_y <<= shift_edges_up;
    stop_y <<= shift_edges_up;

    int32_t top = static_cast<int32_t>(shifted_clip->shifted().y());
    if (!path_contained_in_clip && start_y < top) {
        start_y = top;
    }

    int32_t bottom = static_cast<int32_t>(shifted_clip->shifted().bottom());
    if (!path_contained_in_clip && stop_y > bottom) {
        stop_y = bottom;
    }

    if (start_y < 0 || stop_y < 0 || start_y >= stop_y) return;

    walk_edges(
        fill_rule,
        static_cast<uint32_t>(start_y),
        static_cast<uint32_t>(stop_y),
        shifted_clip->shifted().right(),
        edges,
        blitter
    );
}

} // namespace

namespace scan {

inline int fast_round_fp(float x) noexcept {
#if defined(NISABA_HAS_SSE2)
    return _mm_cvtss_si32(_mm_set_ss(x));
#else
    return static_cast<int>(x >= 0.0f ? (x + 0.5f) : (x - 0.5f));
#endif
}

struct CellRasterizerWorkspace {
    std::vector<int32_t> cells;
    std::vector<int32_t> min_x;
    std::vector<int32_t> max_x;
    std::vector<uint8_t> cov;
};

static thread_local CellRasterizerWorkspace g_cell_workspace;

class AnalyticCellRasterizer {
public:
    static void rasterize(
        const Path& path,
        FillRule fill_rule,
        const ScreenIntRect& clip,
        Blitter& blitter
    ) {
        auto b = path.bounds();
        if (b.width() <= 0.0f || b.height() <= 0.0f) return;

        int32_t bx0 = std::max(static_cast<int32_t>(clip.x()), static_cast<int32_t>(std::floor(b.left())));
        int32_t by0 = std::max(static_cast<int32_t>(clip.y()), static_cast<int32_t>(std::floor(b.top())));
        int32_t bx1 = std::min(static_cast<int32_t>(clip.right()), static_cast<int32_t>(std::ceil(b.right())));
        int32_t by1 = std::min(static_cast<int32_t>(clip.bottom()), static_cast<int32_t>(std::ceil(b.bottom())));

        if (bx1 <= bx0 || by1 <= by0) return;

        int32_t width = bx1 - bx0;
        int32_t height = by1 - by0;

        CellRasterizerWorkspace& ws = g_cell_workspace;
        size_t stride = static_cast<size_t>(width + 16);
        size_t required_cells = static_cast<size_t>(height) * stride;
        if (ws.cells.size() < required_cells) {
            ws.cells.assign(required_cells, 0);
        }
        if (ws.min_x.size() < static_cast<size_t>(height)) {
            ws.min_x.resize(height);
            ws.max_x.resize(height);
        }
        if (ws.cov.size() < stride) {
            ws.cov.resize(stride, 0);
        }

        int32_t* __restrict tl_cells = ws.cells.data();
        int32_t* __restrict tl_min_x = ws.min_x.data();
        int32_t* __restrict tl_max_x = ws.max_x.data();
        uint8_t* __restrict tl_cov = ws.cov.data();

        for (int32_t y = 0; y < height; ++y) {
            tl_min_x[y] = width;
            tl_max_x[y] = -1;
        }

        auto emit_line = [&](Point p0, Point p1) {
            float x0 = p0.x - static_cast<float>(bx0);
            float x1 = p1.x - static_cast<float>(bx0);
            float y0 = p0.y - static_cast<float>(by0);
            float y1 = p1.y - static_cast<float>(by0);

            if (y0 == y1) return;

            int dir = 1;
            if (y0 > y1) {
                std::swap(x0, x1);
                std::swap(y0, y1);
                dir = -1;
            }

            if (y1 <= 0.0f || y0 >= static_cast<float>(height)) return;

            float clamped_y0 = std::max(0.0f, y0);
            float clamped_y1 = std::min(static_cast<float>(height), y1);
            if (clamped_y0 >= clamped_y1) return;

            float dx = x1 - x0;
            float dy = y1 - y0;
            float inv_dy = 1.0f / dy;
            float dx_dy = dx * inv_dy;

            int iy_start = static_cast<int>(clamped_y0);
            int iy_end = static_cast<int>(clamped_y1 - 1e-6f);
            iy_start = std::max(0, iy_start);
            iy_end = std::min(height - 1, iy_end);

            float f_width = static_cast<float>(width);
            float cur_x = x0 + (clamped_y0 - y0) * dx_dy;
            float cur_y = clamped_y0;

            for (int iy = iy_start; iy <= iy_end; ++iy) {
                float next_y = std::min(clamped_y1, static_cast<float>(iy + 1));
                float next_x = (next_y == clamped_y1) ? (x0 + (clamped_y1 - y0) * dx_dy) : (cur_x + (next_y - cur_y) * dx_dy);

                float ya = cur_y;
                float yb = next_y;
                float xa = cur_x;
                float xb = next_x;

                cur_x = next_x;
                cur_y = next_y;

                float fy_a = ya - static_cast<float>(iy);
                float fy_b = yb - static_cast<float>(iy);
                float delta_y = (fy_b - fy_a) * dir;
                if (delta_y == 0.0f) continue;

                // Segments strictly to the right of the rasterizer bounds do not affect internal cells
                if (xa > f_width && xb > f_width) continue;

                int32_t* row_cells = &tl_cells[static_cast<size_t>(iy) * stride];

                // Segments strictly to the left of the rasterizer bounds contribute winding across the row
                if (xa <= 0.0f && xb <= 0.0f) {
                    if (xa < 0.0f || xb < 0.0f) {
                        int cover_fp = fast_round_fp(delta_y * 256.0f);
                        row_cells[0] += (cover_fp << 9);
                        tl_min_x[iy] = std::min(tl_min_x[iy], 0);
                        tl_max_x[iy] = std::max(tl_max_x[iy], width);
                        continue;
                    }
                }

                if (xa < 0.0f || xb < 0.0f) {
                    float t_zero = (0.0f - xa) / (xb - xa);
                    float dy_left = (xa < 0.0f) ? (t_zero * delta_y) : ((1.0f - t_zero) * delta_y);
                    int cover_left_fp = fast_round_fp(dy_left * 256.0f);
                    row_cells[0] += (cover_left_fp << 9);
                    tl_min_x[iy] = std::min(tl_min_x[iy], 0);
                    tl_max_x[iy] = std::max(tl_max_x[iy], width);
                }

                float c_xa = std::clamp(xa, 0.0f, f_width);
                float c_xb = std::clamp(xb, 0.0f, f_width);

                float delta_y_in = delta_y;
                if (xa < 0.0f || xb < 0.0f || xa > f_width || xb > f_width) {
                    float t_a = (c_xa - xa) / (xb - xa);
                    float t_b = (c_xb - xa) / (xb - xa);
                    delta_y_in = (t_b - t_a) * delta_y;
                }

                int ix_a = static_cast<int>(c_xa);
                int ix_b = static_cast<int>(c_xb);
                ix_a = std::clamp(ix_a, 0, width - 1);
                ix_b = std::clamp(ix_b, 0, width - 1);

                if (ix_a == ix_b) {
                    float fx_a = c_xa - static_cast<float>(ix_a);
                    float fx_b = c_xb - static_cast<float>(ix_a);
                    int cover_fp = fast_round_fp(delta_y_in * 256.0f);
                    int area_fp = fast_round_fp(delta_y_in * (fx_a + fx_b) * 65536.0f);

                    row_cells[ix_a]     += (cover_fp << 9) - area_fp;
                    row_cells[ix_a + 1] += area_fp;

                    tl_min_x[iy] = std::min(tl_min_x[iy], ix_a);
                    tl_max_x[iy] = std::max(tl_max_x[iy], ix_a + 1);
                } else {
                    int min_seg_x = std::min(ix_a, ix_b);
                    int max_seg_x = std::max(ix_a, ix_b) + 1;
                    tl_min_x[iy] = std::min(tl_min_x[iy], min_seg_x);
                    tl_max_x[iy] = std::max(tl_max_x[iy], max_seg_x);

                    float seg_dx = c_xb - c_xa;
                    float seg_inv_dx = 1.0f / seg_dx;
                    float cur_col_x = c_xa;
                    float cur_t = 0.0f;

                    if (c_xa > c_xb) {
                        for (int ix = ix_a; ix >= ix_b; --ix) {
                            float next_col_x = (ix == ix_b) ? c_xb : static_cast<float>(ix);
                            float next_t = (next_col_x - c_xa) * seg_inv_dx;
                            float col_dy = (next_t - cur_t) * delta_y_in;
                            float rel_x0 = cur_col_x - static_cast<float>(ix);
                            float rel_x1 = next_col_x - static_cast<float>(ix);

                            int cover_fp = fast_round_fp(col_dy * 256.0f);
                            int area_fp = fast_round_fp(col_dy * (rel_x0 + rel_x1) * 65536.0f);

                            row_cells[ix]     += (cover_fp << 9) - area_fp;
                            row_cells[ix + 1] += area_fp;

                            cur_col_x = next_col_x;
                            cur_t = next_t;
                        }
                    } else {
                        for (int ix = ix_a; ix <= ix_b; ++ix) {
                            float next_col_x = (ix == ix_b) ? c_xb : static_cast<float>(ix + 1);
                            float next_t = (next_col_x - c_xa) * seg_inv_dx;
                            float col_dy = (next_t - cur_t) * delta_y_in;
                            float rel_x0 = cur_col_x - static_cast<float>(ix);
                            float rel_x1 = next_col_x - static_cast<float>(ix);

                            int cover_fp = fast_round_fp(col_dy * 256.0f);
                            int area_fp = fast_round_fp(col_dy * (rel_x0 + rel_x1) * 65536.0f);

                            row_cells[ix]     += (cover_fp << 9) - area_fp;
                            row_cells[ix + 1] += area_fp;

                            cur_col_x = next_col_x;
                            cur_t = next_t;
                        }
                    }
                }
            }
        };

        auto push_quad = [&](const Point* points) {
            float ux = points[0].x - 2.0f * points[1].x + points[2].x;
            float uy = points[0].y - 2.0f * points[1].y + points[2].y;
            float max_d = std::max(std::abs(ux), std::abs(uy));
            if (max_d < 0.25f) {
                emit_line(points[0], points[2]);
                return;
            }
            int32_t n = static_cast<int32_t>(std::ceil(std::sqrt(max_d)));
            n = std::clamp(n, 2, 16);

            path_geometry::QuadCoeff coeff = path_geometry::QuadCoeff::from_points(points);
            float dt = 1.0f / static_cast<float>(n);
            Point prev = points[0];
            for (int32_t i = 1; i < n; ++i) {
                float t = static_cast<float>(i) * dt;
                f32x2 pt = coeff.eval(f32x2::splat(t));
                Point cur = Point::from_xy(pt.x(), pt.y());
                emit_line(prev, cur);
                prev = cur;
            }
            emit_line(prev, points[2]);
        };

        auto push_cubic = [&](const Point* points) {
            float u1x = points[0].x - 2.0f * points[1].x + points[2].x;
            float u1y = points[0].y - 2.0f * points[1].y + points[2].y;
            float u2x = points[1].x - 2.0f * points[2].x + points[3].x;
            float u2y = points[1].y - 2.0f * points[2].y + points[3].y;
            float max_d = std::max({std::abs(u1x), std::abs(u1y), std::abs(u2x), std::abs(u2y)});
            if (max_d < 0.25f) {
                emit_line(points[0], points[3]);
                return;
            }
            int32_t n = static_cast<int32_t>(std::ceil(std::sqrt(1.5f * max_d)));
            n = std::clamp(n, 2, 16);

            path_geometry::CubicCoeff coeff = path_geometry::CubicCoeff::from_points(points);
            float dt = 1.0f / static_cast<float>(n);
            Point prev = points[0];
            for (int32_t i = 1; i < n; ++i) {
                float t = static_cast<float>(i) * dt;
                f32x2 pt = coeff.eval(f32x2::splat(t));
                Point cur = Point::from_xy(pt.x(), pt.y());
                emit_line(prev, cur);
                prev = cur;
            }
            emit_line(prev, points[3]);
        };

        PathEdgeIter iter(path);
        while (auto edge = iter.next()) {
            switch (edge->type) {
                case PathEdge::Type::LineTo:
                    emit_line(edge->points[0], edge->points[1]);
                    break;
                case PathEdge::Type::QuadTo:
                    push_quad(edge->points.data());
                    break;
                case PathEdge::Type::CubicTo:
                    push_cubic(edge->points.data());
                    break;
            }
        }

        // Emit rows
        for (int32_t iy = 0; iy < height; ++iy) {
            if (tl_min_x[iy] > tl_max_x[iy]) continue;
            int32_t min_x = tl_min_x[iy];
            int32_t max_x = std::min(width, tl_max_x[iy] + 1);
            if (max_x <= min_x) {
                tl_min_x[iy] = width;
                tl_max_x[iy] = -1;
                continue;
            }

            int32_t* row_cells = tl_cells + (static_cast<size_t>(iy) * stride);
            int32_t accum = 0;
            uint32_t count = static_cast<uint32_t>(max_x - min_x);
            uint8_t* cov_ptr = tl_cov + min_x;

#if defined(NISABA_HAS_AVX2)
            int32_t x = min_x;
            if (fill_rule == FillRule::EvenOdd) {
                while (x < max_x) {
                    if (x + 32 <= max_x) {
                        __m256i v0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x));
                        __m256i v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x + 8));
                        __m256i v2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x + 16));
                        __m256i v3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x + 24));
                        __m256i vor = _mm256_or_si256(_mm256_or_si256(v0, v1), _mm256_or_si256(v2, v3));
                        if (_mm256_testz_si256(vor, vor)) {
                            int32_t c = (std::abs(accum) >> 9) & 511;
                            if (c > 256) c = 512 - c;
                            c = std::min(255, c);
                            _mm256_storeu_si256(reinterpret_cast<__m256i*>(tl_cov + x), _mm256_set1_epi8(static_cast<char>(c)));
                            x += 32;
                            continue;
                        }
                    }
                    if (x + 16 <= max_x) {
                        __m256i v0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x));
                        __m256i v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x + 8));
                        __m256i vor = _mm256_or_si256(v0, v1);
                        if (_mm256_testz_si256(vor, vor)) {
                            int32_t c = (std::abs(accum) >> 9) & 511;
                            if (c > 256) c = 512 - c;
                            c = std::min(255, c);
                            _mm_storeu_si128(reinterpret_cast<__m128i*>(tl_cov + x), _mm_set1_epi8(static_cast<char>(c)));
                            x += 16;
                            continue;
                        }
                    }
                    if (x + 8 <= max_x) {
                        __m256i v0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x));
                        if (_mm256_testz_si256(v0, v0)) {
                            int32_t c = (std::abs(accum) >> 9) & 511;
                            if (c > 256) c = 512 - c;
                            uint64_t v64 = static_cast<uint64_t>(static_cast<uint8_t>(std::min(255, c))) * 0x0101010101010101ULL;
                            *reinterpret_cast<uint64_t*>(tl_cov + x) = v64;
                            x += 8;
                            continue;
                        }
                    }
                    accum += row_cells[x];
                    row_cells[x] = 0;
                    int32_t c = (std::abs(accum) >> 9) & 511;
                    if (c > 256) c = 512 - c;
                    tl_cov[x] = static_cast<uint8_t>(std::min(255, c));
                    ++x;
                }
            } else {
                while (x < max_x) {
                    if (x + 32 <= max_x) {
                        __m256i v0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x));
                        __m256i v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x + 8));
                        __m256i v2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x + 16));
                        __m256i v3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x + 24));
                        __m256i vor = _mm256_or_si256(_mm256_or_si256(v0, v1), _mm256_or_si256(v2, v3));
                        if (_mm256_testz_si256(vor, vor)) {
                            int32_t c = std::min(255, std::abs(accum) >> 9);
                            _mm256_storeu_si256(reinterpret_cast<__m256i*>(tl_cov + x), _mm256_set1_epi8(static_cast<char>(c)));
                            x += 32;
                            continue;
                        }
                    }
                    if (x + 16 <= max_x) {
                        __m256i v0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x));
                        __m256i v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x + 8));
                        __m256i vor = _mm256_or_si256(v0, v1);
                        if (_mm256_testz_si256(vor, vor)) {
                            int32_t c = std::min(255, std::abs(accum) >> 9);
                            _mm_storeu_si128(reinterpret_cast<__m128i*>(tl_cov + x), _mm_set1_epi8(static_cast<char>(c)));
                            x += 16;
                            continue;
                        }
                    }
                    if (x + 8 <= max_x) {
                        __m256i v0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x));
                        if (_mm256_testz_si256(v0, v0)) {
                            uint64_t v64 = static_cast<uint64_t>(static_cast<uint8_t>(std::min(255, std::abs(accum) >> 9))) * 0x0101010101010101ULL;
                            *reinterpret_cast<uint64_t*>(tl_cov + x) = v64;
                            x += 8;
                            continue;
                        }
                    }
                    accum += row_cells[x];
                    row_cells[x] = 0;
                    int32_t c = std::abs(accum) >> 9;
                    tl_cov[x] = static_cast<uint8_t>(std::min(255, c));
                    ++x;
                }
            }
#else
            if (fill_rule == FillRule::EvenOdd) {
                for (int32_t x = min_x; x < max_x; ++x) {
                    accum += row_cells[x];
                    row_cells[x] = 0;
                    int32_t c = (std::abs(accum) >> 9) & 511;
                    if (c > 256) c = 512 - c;
                    tl_cov[x] = static_cast<uint8_t>(std::min(255, c));
                }
            } else {
                for (int32_t x = min_x; x < max_x; ++x) {
                    accum += row_cells[x];
                    row_cells[x] = 0;
                    int32_t c = std::abs(accum) >> 9;
                    tl_cov[x] = static_cast<uint8_t>(std::min(255, c));
                }
            }
#endif
            row_cells[max_x] = 0;

            tl_min_x[iy] = width;
            tl_max_x[iy] = -1;

            uint32_t screen_y = static_cast<uint32_t>(by0 + iy);
            uint32_t screen_x = static_cast<uint32_t>(bx0 + min_x);
            blitter.blit_span_coverage(screen_x, screen_y, cov_ptr, count);
        }
    }
};

} // namespace

namespace scan {

void fill_path(
    const Path& path,
    FillRule fill_rule,
    const ScreenIntRect& clip,
    bool anti_alias,
    Blitter& blitter
) {
    if (path.is_empty()) return;

    if (!anti_alias) {
        auto ir = conservative_round_to_int(path.bounds());
        if (!ir) return;

        bool contained = false;
        if (auto bounds_scr = to_screen_int_rect(*ir)) {
            contained = clip.contains(*bounds_scr);
        }

        fill_path_impl(
            path,
            fill_rule,
            clip,
            ir->y(),
            ir->bottom(),
            0,
            contained,
            blitter
        );
    } else {
        AnalyticCellRasterizer::rasterize(path, fill_rule, clip, blitter);
    }
}

void fill_path(
    const Path& path,
    FillRule fill_rule,
    const ScreenIntRect& clip,
    bool anti_alias,
    Blitter& blitter,
    const Transform& transform
) {
    if (transform.is_identity()) {
        fill_path(path, fill_rule, clip, anti_alias, blitter);
        return;
    }

    if (path.is_empty()) return;

    if (anti_alias) {
        // Transform path bounds to get the rasterization clip region
        auto b = path.bounds();
        // Transform the 4 corners of the bounding box
        Point corners[4] = {
            Point::from_xy(b.left(), b.top()),
            Point::from_xy(b.right(), b.top()),
            Point::from_xy(b.right(), b.bottom()),
            Point::from_xy(b.left(), b.bottom())
        };
        for (auto& c : corners) {
            transform.map_point(c);
        }
        float tb_left = std::min({corners[0].x, corners[1].x, corners[2].x, corners[3].x});
        float tb_top = std::min({corners[0].y, corners[1].y, corners[2].y, corners[3].y});
        float tb_right = std::max({corners[0].x, corners[1].x, corners[2].x, corners[3].x});
        float tb_bottom = std::max({corners[0].y, corners[1].y, corners[2].y, corners[3].y});

        if (tb_right <= tb_left || tb_bottom <= tb_top) return;

        int32_t bx0 = std::max(static_cast<int32_t>(clip.x()), static_cast<int32_t>(std::floor(tb_left)));
        int32_t by0 = std::max(static_cast<int32_t>(clip.y()), static_cast<int32_t>(std::floor(tb_top)));
        int32_t bx1 = std::min(static_cast<int32_t>(clip.right()), static_cast<int32_t>(std::ceil(tb_right)));
        int32_t by1 = std::min(static_cast<int32_t>(clip.bottom()), static_cast<int32_t>(std::ceil(tb_bottom)));

        if (bx1 <= bx0 || by1 <= by0) return;

        int32_t width = bx1 - bx0;
        int32_t height = by1 - by0;

        CellRasterizerWorkspace& ws = g_cell_workspace;
        size_t stride = static_cast<size_t>(width + 16);
        size_t required_cells = static_cast<size_t>(height) * stride;
        if (ws.cells.size() < required_cells) {
            ws.cells.assign(required_cells, 0);
        }
        if (ws.min_x.size() < static_cast<size_t>(height)) {
            ws.min_x.resize(height);
            ws.max_x.resize(height);
        }
        if (ws.cov.size() < stride) {
            ws.cov.resize(stride, 0);
        }

        int32_t* __restrict tl_cells = ws.cells.data();
        int32_t* __restrict tl_min_x = ws.min_x.data();
        int32_t* __restrict tl_max_x = ws.max_x.data();
        uint8_t* __restrict tl_cov = ws.cov.data();

        for (int32_t y = 0; y < height; ++y) {
            tl_min_x[y] = width;
            tl_max_x[y] = -1;
        }

        // Reuse the same emit_line from AnalyticCellRasterizer but transform points inline
        auto emit_line = [&](Point p0, Point p1) {
            // Apply transform inline
            transform.map_point(p0);
            transform.map_point(p1);

            float x0 = p0.x - static_cast<float>(bx0);
            float x1 = p1.x - static_cast<float>(bx0);
            float y0 = p0.y - static_cast<float>(by0);
            float y1 = p1.y - static_cast<float>(by0);

            if (y0 == y1) return;

            int dir = 1;
            if (y0 > y1) {
                std::swap(x0, x1);
                std::swap(y0, y1);
                dir = -1;
            }

            if (y1 <= 0.0f || y0 >= static_cast<float>(height)) return;

            float clamped_y0 = std::max(0.0f, y0);
            float clamped_y1 = std::min(static_cast<float>(height), y1);
            if (clamped_y0 >= clamped_y1) return;

            float dx = x1 - x0;
            float dy = y1 - y0;
            float inv_dy = 1.0f / dy;
            float dx_dy = dx * inv_dy;

            int iy_start = static_cast<int>(clamped_y0);
            int iy_end = static_cast<int>(clamped_y1 - 1e-6f);
            iy_start = std::max(0, iy_start);
            iy_end = std::min(height - 1, iy_end);

            float f_width = static_cast<float>(width);
            float cur_x = x0 + (clamped_y0 - y0) * dx_dy;
            float cur_y = clamped_y0;

            for (int iy = iy_start; iy <= iy_end; ++iy) {
                float next_y = std::min(clamped_y1, static_cast<float>(iy + 1));
                float next_x = (next_y == clamped_y1) ? (x0 + (clamped_y1 - y0) * dx_dy) : (cur_x + (next_y - cur_y) * dx_dy);

                float ya = cur_y;
                float yb = next_y;
                float xa = cur_x;
                float xb = next_x;

                cur_x = next_x;
                cur_y = next_y;

                float fy_a = ya - static_cast<float>(iy);
                float fy_b = yb - static_cast<float>(iy);
                float delta_y = (fy_b - fy_a) * dir;
                if (delta_y == 0.0f) continue;

                if (xa > f_width && xb > f_width) continue;

                int32_t* row_cells = &tl_cells[static_cast<size_t>(iy) * stride];

                if (xa <= 0.0f && xb <= 0.0f) {
                    if (xa < 0.0f || xb < 0.0f) {
                        int cover_fp = fast_round_fp(delta_y * 256.0f);
                        row_cells[0] += (cover_fp << 9);
                        tl_min_x[iy] = std::min(tl_min_x[iy], 0);
                        tl_max_x[iy] = std::max(tl_max_x[iy], width);
                        continue;
                    }
                }

                if (xa < 0.0f || xb < 0.0f) {
                    float t_zero = (0.0f - xa) / (xb - xa);
                    float dy_left = (xa < 0.0f) ? (t_zero * delta_y) : ((1.0f - t_zero) * delta_y);
                    int cover_left_fp = fast_round_fp(dy_left * 256.0f);
                    row_cells[0] += (cover_left_fp << 9);
                    tl_min_x[iy] = std::min(tl_min_x[iy], 0);
                    tl_max_x[iy] = std::max(tl_max_x[iy], width);
                }

                float c_xa = std::clamp(xa, 0.0f, f_width);
                float c_xb = std::clamp(xb, 0.0f, f_width);

                float delta_y_in = delta_y;
                if (xa < 0.0f || xb < 0.0f || xa > f_width || xb > f_width) {
                    float t_a = (c_xa - xa) / (xb - xa);
                    float t_b = (c_xb - xa) / (xb - xa);
                    delta_y_in = (t_b - t_a) * delta_y;
                }

                int ix_a = std::clamp(static_cast<int>(c_xa), 0, width - 1);
                int ix_b = std::clamp(static_cast<int>(c_xb), 0, width - 1);

                if (ix_a == ix_b) {
                    float fx_a = c_xa - static_cast<float>(ix_a);
                    float fx_b = c_xb - static_cast<float>(ix_a);
                    int cover_fp = fast_round_fp(delta_y_in * 256.0f);
                    int area_fp = fast_round_fp(delta_y_in * (fx_a + fx_b) * 65536.0f);
                    row_cells[ix_a]     += (cover_fp << 9) - area_fp;
                    row_cells[ix_a + 1] += area_fp;
                    tl_min_x[iy] = std::min(tl_min_x[iy], ix_a);
                    tl_max_x[iy] = std::max(tl_max_x[iy], ix_a + 1);
                } else {
                    int min_seg_x = std::min(ix_a, ix_b);
                    int max_seg_x = std::max(ix_a, ix_b) + 1;
                    tl_min_x[iy] = std::min(tl_min_x[iy], min_seg_x);
                    tl_max_x[iy] = std::max(tl_max_x[iy], max_seg_x);

                    float seg_dx = c_xb - c_xa;
                    float seg_inv_dx = 1.0f / seg_dx;
                    float cur_t = 0.0f;

                    if (c_xa > c_xb) {
                        float step_t2 = seg_inv_dx;
                        float step_dy2 = step_t2 * delta_y_in;
                        int step_cover2 = fast_round_fp(step_dy2 * 256.0f);
                        int step_area2 = fast_round_fp(step_dy2 * 65536.0f);
                        {
                            float next_col_x = (ix_a == ix_b) ? c_xb : static_cast<float>(ix_a);
                            float next_t2 = (next_col_x - c_xa) * seg_inv_dx;
                            float col_dy = next_t2 * delta_y_in;
                            float rel_x0 = c_xa - static_cast<float>(ix_a);
                            float rel_x1 = next_col_x - static_cast<float>(ix_a);
                            int cover_fp = fast_round_fp(col_dy * 256.0f);
                            int area_fp = fast_round_fp(col_dy * (rel_x0 + rel_x1) * 65536.0f);
                            row_cells[ix_a]     += (cover_fp << 9) - area_fp;
                            row_cells[ix_a + 1] += area_fp;
                            cur_t = next_t2;
                        }
                        for (int ix = ix_a - 1; ix > ix_b; --ix) {
                            row_cells[ix]     += (step_cover2 << 9) - step_area2;
                            row_cells[ix + 1] += step_area2;
                        }
                        if (ix_b < ix_a) {
                            float last_t2 = 1.0f;
                            float col_dy = (last_t2 - cur_t) * delta_y_in;
                            float fx_last = c_xb - static_cast<float>(ix_b);
                            int cover_fp = fast_round_fp(col_dy * 256.0f);
                            int area_fp = fast_round_fp(col_dy * (1.0f + fx_last) * 65536.0f);
                            row_cells[ix_b]     += (cover_fp << 9) - area_fp;
                            row_cells[ix_b + 1] += area_fp;
                        }
                    } else {
                        float step_t2 = seg_inv_dx;
                        float step_dy2 = step_t2 * delta_y_in;
                        int step_cover2 = fast_round_fp(step_dy2 * 256.0f);
                        int step_area2 = fast_round_fp(step_dy2 * 65536.0f);
                        {
                            float next_col_x = (ix_a == ix_b) ? c_xb : static_cast<float>(ix_a + 1);
                            float next_t2 = (next_col_x - c_xa) * seg_inv_dx;
                            float col_dy = next_t2 * delta_y_in;
                            float rel_x0 = c_xa - static_cast<float>(ix_a);
                            float rel_x1 = next_col_x - static_cast<float>(ix_a);
                            int cover_fp = fast_round_fp(col_dy * 256.0f);
                            int area_fp = fast_round_fp(col_dy * (rel_x0 + rel_x1) * 65536.0f);
                            row_cells[ix_a]     += (cover_fp << 9) - area_fp;
                            row_cells[ix_a + 1] += area_fp;
                            cur_t = next_t2;
                        }
                        for (int ix = ix_a + 1; ix < ix_b; ++ix) {
                            row_cells[ix]     += (step_cover2 << 9) - step_area2;
                            row_cells[ix + 1] += step_area2;
                        }
                        if (ix_b > ix_a) {
                            float last_t2 = 1.0f;
                            float col_dy = (last_t2 - cur_t) * delta_y_in;
                            float fx_last = c_xb - static_cast<float>(ix_b);
                            int cover_fp = fast_round_fp(col_dy * 256.0f);
                            int area_fp = fast_round_fp(col_dy * fx_last * 65536.0f);
                            row_cells[ix_b]     += (cover_fp << 9) - area_fp;
                            row_cells[ix_b + 1] += area_fp;
                        }
                    }
                }
            }
        };

        // Iterate path edges and transform points inline
        PathEdgeIter iter(path);
        while (auto edge = iter.next()) {
            switch (edge->type) {
                case PathEdge::Type::LineTo:
                    emit_line(edge->points[0], edge->points[1]);
                    break;
                case PathEdge::Type::QuadTo: {
                    // Flatten quad to lines with inline transform
                    const Point* pts = edge->points.data();
                    float ux = pts[0].x - 2.0f * pts[1].x + pts[2].x;
                    float uy = pts[0].y - 2.0f * pts[1].y + pts[2].y;
                    float max_d = std::max(std::abs(ux), std::abs(uy));
                    if (max_d < 0.25f) {
                        emit_line(pts[0], pts[2]);
                    } else {
                        int32_t n = std::clamp(static_cast<int32_t>(std::ceil(std::sqrt(max_d))), 2, 16);
                        path_geometry::QuadCoeff coeff = path_geometry::QuadCoeff::from_points(pts);
                        float dt = 1.0f / static_cast<float>(n);
                        Point prev = pts[0];
                        for (int32_t i = 1; i < n; ++i) {
                            f32x2 pt = coeff.eval(f32x2::splat(static_cast<float>(i) * dt));
                            Point cur = Point::from_xy(pt.x(), pt.y());
                            emit_line(prev, cur);
                            prev = cur;
                        }
                        emit_line(prev, pts[2]);
                    }
                    break;
                }
                case PathEdge::Type::CubicTo: {
                    const Point* pts = edge->points.data();
                    float u1x = pts[0].x - 2.0f * pts[1].x + pts[2].x;
                    float u1y = pts[0].y - 2.0f * pts[1].y + pts[2].y;
                    float u2x = pts[1].x - 2.0f * pts[2].x + pts[3].x;
                    float u2y = pts[1].y - 2.0f * pts[2].y + pts[3].y;
                    float max_d = std::max({std::abs(u1x), std::abs(u1y), std::abs(u2x), std::abs(u2y)});
                    if (max_d < 0.25f) {
                        emit_line(pts[0], pts[3]);
                    } else {
                        int32_t n = std::clamp(static_cast<int32_t>(std::ceil(std::sqrt(1.5f * max_d))), 2, 16);
                        path_geometry::CubicCoeff coeff = path_geometry::CubicCoeff::from_points(pts);
                        float dt = 1.0f / static_cast<float>(n);
                        Point prev = pts[0];
                        for (int32_t i = 1; i < n; ++i) {
                            f32x2 pt = coeff.eval(f32x2::splat(static_cast<float>(i) * dt));
                            Point cur = Point::from_xy(pt.x(), pt.y());
                            emit_line(prev, cur);
                            prev = cur;
                        }
                        emit_line(prev, pts[3]);
                    }
                    break;
                }
            }
        }

        // Emit rows (same as AnalyticCellRasterizer)
        for (int32_t iy = 0; iy < height; ++iy) {
            if (tl_min_x[iy] > tl_max_x[iy]) continue;
            int32_t min_x = tl_min_x[iy];
            int32_t max_x = std::min(width, tl_max_x[iy] + 1);
            if (max_x <= min_x) {
                tl_min_x[iy] = width;
                tl_max_x[iy] = -1;
                continue;
            }

            int32_t* row_cells = tl_cells + (static_cast<size_t>(iy) * stride);
            int32_t accum = 0;
            uint32_t count = static_cast<uint32_t>(max_x - min_x);
            uint8_t* cov_ptr = tl_cov + min_x;

#if defined(NISABA_HAS_AVX2)
            int32_t x = min_x;
            if (fill_rule == FillRule::EvenOdd) {
                while (x < max_x) {
                    if (x + 8 <= max_x) {
                        __m256i v0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x));
                        if (_mm256_testz_si256(v0, v0)) {
                            int32_t c = (std::abs(accum) >> 9) & 511;
                            if (c > 256) c = 512 - c;
                            uint64_t v64 = static_cast<uint64_t>(static_cast<uint8_t>(std::min(255, c))) * 0x0101010101010101ULL;
                            *reinterpret_cast<uint64_t*>(tl_cov + x) = v64;
                            x += 8;
                            continue;
                        }
                    }
                    accum += row_cells[x];
                    row_cells[x] = 0;
                    int32_t c = (std::abs(accum) >> 9) & 511;
                    if (c > 256) c = 512 - c;
                    tl_cov[x] = static_cast<uint8_t>(std::min(255, c));
                    ++x;
                }
            } else {
                while (x < max_x) {
                    if (x + 8 <= max_x) {
                        __m256i v0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(row_cells + x));
                        if (_mm256_testz_si256(v0, v0)) {
                            uint64_t v64 = static_cast<uint64_t>(static_cast<uint8_t>(std::min(255, std::abs(accum) >> 9))) * 0x0101010101010101ULL;
                            *reinterpret_cast<uint64_t*>(tl_cov + x) = v64;
                            x += 8;
                            continue;
                        }
                    }
                    accum += row_cells[x];
                    row_cells[x] = 0;
                    int32_t c = std::abs(accum) >> 9;
                    tl_cov[x] = static_cast<uint8_t>(std::min(255, c));
                    ++x;
                }
            }
#else
            if (fill_rule == FillRule::EvenOdd) {
                for (int32_t x = min_x; x < max_x; ++x) {
                    accum += row_cells[x];
                    row_cells[x] = 0;
                    int32_t c = (std::abs(accum) >> 9) & 511;
                    if (c > 256) c = 512 - c;
                    tl_cov[x] = static_cast<uint8_t>(std::min(255, c));
                }
            } else {
                for (int32_t x = min_x; x < max_x; ++x) {
                    accum += row_cells[x];
                    row_cells[x] = 0;
                    int32_t c = std::abs(accum) >> 9;
                    tl_cov[x] = static_cast<uint8_t>(std::min(255, c));
                }
            }
#endif
            row_cells[max_x] = 0;

            tl_min_x[iy] = width;
            tl_max_x[iy] = -1;

            uint32_t screen_y = static_cast<uint32_t>(by0 + iy);
            uint32_t screen_x = static_cast<uint32_t>(bx0 + min_x);
            blitter.blit_span_coverage(screen_x, screen_y, cov_ptr, count);
        }
    } else {
        // For non-AA, fall back to transforming the path
        auto transformed = path.transform(transform);
        if (!transformed) return;
        fill_path(*transformed, fill_rule, clip, anti_alias, blitter);
    }
}

} // namespace scan

SolidColorBlitter::SolidColorBlitter(PixmapMut& pixmap, Color color) noexcept
    : pixmap_(pixmap),
      color_(color.premultiply().to_color_u8()),
      src_color_(color.to_color_u8()) {}

void SolidColorBlitter::blit_h(uint32_t x, uint32_t y, LengthU32 width) {
    if (y >= pixmap_.height()) return;
    uint32_t w = pixmap_.width();
    if (x >= w) return;
    uint32_t count = std::min(width.get(), w - x);

    PremultipliedColorU8* dst = pixmap_.pixels_mut() + (static_cast<size_t>(y) * w + x);

    if (color_.is_opaque()) {
        simd::fill_solid_span(dst, color_, count);
    } else {
        simd::blend_solid_source_over(dst, color_, count);
    }
}

void SolidColorBlitter::blit_rect(const ScreenIntRect& rect) {
    uint32_t x = rect.x();
    uint32_t y = rect.y();
    uint32_t w = rect.width();
    uint32_t h = rect.height();
    if (x >= pixmap_.width() || y >= pixmap_.height() || w == 0 || h == 0) return;
    w = std::min(w, pixmap_.width() - x);
    h = std::min(h, pixmap_.height() - y);

    PremultipliedColorU8* dst = pixmap_.pixels_mut() + (static_cast<size_t>(y) * pixmap_.width() + x);
    simd::blend_solid_rect_source_over(dst, pixmap_.width(), color_, w, h);
}

void SolidColorBlitter::blit_anti_h(
    uint32_t x,
    uint32_t y,
    std::span<AlphaU8> antialias,
    std::span<AlphaRun> runs
) {
    if (y >= pixmap_.height()) return;
    uint32_t w = pixmap_.width();
    if (x >= w) return;

    size_t run_idx = 0;
    while (runs[run_idx].has_value()) {
        size_t count = *runs[run_idx];
        uint8_t coverage = antialias[run_idx];

        if (coverage > 0) {
            uint32_t px_x = x + static_cast<uint32_t>(run_idx);
            if (px_x < w) {
                uint32_t actual_count = std::min(static_cast<uint32_t>(count), w - px_x);
                PremultipliedColorU8* dst = pixmap_.pixels_mut() + (static_cast<size_t>(y) * w + px_x);

                if (coverage == 255) {
                    if (color_.is_opaque()) {
                        simd::fill_solid_span(dst, color_, actual_count);
                    } else {
                        simd::blend_solid_source_over(dst, color_, actual_count);
                    }
                } else {
                    // Modulate color by coverage
                    uint8_t sr = premultiply_u8(color_.red(), coverage);
                    uint8_t sg = premultiply_u8(color_.green(), coverage);
                    uint8_t sb = premultiply_u8(color_.blue(), coverage);
                    uint8_t sa = premultiply_u8(color_.alpha(), coverage);

                    uint8_t inv_sa = 255 - sa;
                    for (uint32_t i = 0; i < actual_count; ++i) {
                        uint8_t dr = sr + premultiply_u8(dst[i].red(), inv_sa);
                        uint8_t dg = sg + premultiply_u8(dst[i].green(), inv_sa);
                        uint8_t db = sb + premultiply_u8(dst[i].blue(), inv_sa);
                        uint8_t da = sa + premultiply_u8(dst[i].alpha(), inv_sa);
                        dst[i] = PremultipliedColorU8::from_rgba_unchecked(dr, dg, db, da);
                    }
                }
            }
        }

        run_idx += count;
    }
}

void SolidColorBlitter::blit_span_coverage(
    uint32_t x,
    uint32_t y,
    const uint8_t* coverage,
    uint32_t count
) {
    if (y >= pixmap_.height() || x >= pixmap_.width() || count == 0) return;
    uint32_t actual_count = std::min(count, pixmap_.width() - x);
    PremultipliedColorU8* dst = pixmap_.pixels_mut() + (static_cast<size_t>(y) * pixmap_.width() + x);

    simd::blend_solid_mask_span(dst, color_, coverage, actual_count);
}

} // namespace nisaba
