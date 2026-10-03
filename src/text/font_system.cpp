#include "nisaba/text/font_system.hpp"
#include <algorithm>
#include <cctype>

namespace nisaba::text {

namespace {

bool str_case_contains(std::string_view haystack, std::string_view needle) noexcept {
    if (needle.empty()) return true;
    if (haystack.size() < needle.size()) return false;

    auto it = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char ch1, char ch2) {
            return std::tolower(static_cast<unsigned char>(ch1)) ==
                   std::tolower(static_cast<unsigned char>(ch2));
        }
    );
    return it != haystack.end();
}

} // namespace

FontSystem::FontSystem() = default;
FontSystem::~FontSystem() = default;

FontSystem::FontSystem(FontSystem&&) noexcept = default;
FontSystem& FontSystem::operator=(FontSystem&&) noexcept = default;

std::optional<uint32_t> FontSystem::load_font_file(std::string_view path) {
    auto font = TtfFont::from_file(path);
    if (!font || !font->is_valid()) {
        return std::nullopt;
    }

    uint32_t id = next_id_++;
    if (!default_font_id_.has_value()) {
        default_font_id_ = id;
    }

    fonts_.push_back(FontRecord{id, std::move(font)});
    return id;
}

std::optional<uint32_t> FontSystem::load_font_data(std::span<const uint8_t> data) {
    auto font = TtfFont::from_bytes(data);
    if (!font || !font->is_valid()) {
        return std::nullopt;
    }

    uint32_t id = next_id_++;
    if (!default_font_id_.has_value()) {
        default_font_id_ = id;
    }

    fonts_.push_back(FontRecord{id, std::move(font)});
    return id;
}

const TtfFont* FontSystem::get_font(uint32_t font_id) const noexcept {
    for (const auto& rec : fonts_) {
        if (rec.id == font_id) {
            return rec.font.get();
        }
    }
    return nullptr;
}

std::pair<uint32_t, const TtfFont*> FontSystem::match_font(
    const Family& family,
    Weight weight,
    Style style
) const noexcept {
    if (fonts_.empty()) {
        return {0, nullptr};
    }
    if (fonts_.size() == 1) {
        return {fonts_[0].id, fonts_[0].font.get()};
    }

    const FontRecord* best_match = nullptr;
    int best_score = -1000;

    for (const auto& rec : fonts_) {
        int score = 0;
        const auto* f = rec.font.get();

        if (family.kind == Family::Kind::Name) {
            if (str_case_contains(f->family_name(), family.name)) {
                score += 1000;
            }
        } else if (family.kind == Family::Kind::Monospace) {
            if (str_case_contains(f->family_name(), "Mono") ||
                str_case_contains(f->family_name(), "Code")) {
                score += 500;
            }
        } else if (family.kind == Family::Kind::Serif) {
            if (str_case_contains(f->family_name(), "Serif")) {
                score += 500;
            }
        }

        // Match weight distance
        int w_diff = std::abs(static_cast<int>(f->weight()) - static_cast<int>(weight));
        score -= w_diff / 10;

        // Match style
        if (f->style() == style) {
            score += 50;
        }

        if (default_font_id_.has_value() && rec.id == *default_font_id_) {
            score += 10;
        }

        if (score > best_score) {
            best_score = score;
            best_match = &rec;
        }
    }

    if (best_match) {
        return {best_match->id, best_match->font.get()};
    }

    return {fonts_[0].id, fonts_[0].font.get()};
}

std::pair<uint32_t, uint16_t> FontSystem::find_glyph_or_fallback(
    uint32_t primary_font_id,
    char32_t codepoint
) const noexcept {
    const TtfFont* primary = get_font(primary_font_id);
    if (primary) {
        uint16_t gid = primary->glyph_index(codepoint);
        if (gid != 0) {
            return {primary_font_id, gid};
        }
    }

    // Search across all loaded fallback fonts
    for (const auto& rec : fonts_) {
        if (rec.id == primary_font_id) continue;
        uint16_t gid = rec.font->glyph_index(codepoint);
        if (gid != 0) {
            return {rec.id, gid};
        }
    }

    return {primary_font_id, 0};
}

} // namespace nisaba::text
