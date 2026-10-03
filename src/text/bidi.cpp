#include "nisaba/text/bidi.hpp"
#include <algorithm>

namespace nisaba::text {

namespace {

struct ArabicLetterForms {
    char32_t isolated{0};
    char32_t final_form{0};
    char32_t initial{0};
    char32_t medial{0};
    bool right_joining{false}; // Only connects to previous (Alif, Dal, Ra, Waw, etc.)
    bool dual_joining{false};  // Connects to both previous and next (Ba, Jim, Sin, etc.)
};

ArabicLetterForms get_arabic_forms(char32_t c) noexcept {
    switch (c) {
        case 0x0622: return {0xFE81, 0xFE82, 0, 0, true, false}; // آ
        case 0x0623: return {0xFE83, 0xFE84, 0, 0, true, false}; // أ
        case 0x0624: return {0xFE85, 0xFE86, 0, 0, true, false}; // ؤ
        case 0x0625: return {0xFE87, 0xFE88, 0, 0, true, false}; // إ
        case 0x0626: return {0xFE89, 0xFE8A, 0xFE8B, 0xFE8C, false, true}; // ئ
        case 0x0627: return {0xFE8D, 0xFE8E, 0, 0, true, false}; // ا
        case 0x0628: return {0xFE8F, 0xFE90, 0xFE91, 0xFE92, false, true}; // ب
        case 0x0629: return {0xFE93, 0xFE94, 0, 0, true, false}; // ة
        case 0x062A: return {0xFE95, 0xFE96, 0xFE97, 0xFE98, false, true}; // ت
        case 0x062B: return {0xFE99, 0xFE9A, 0xFE9B, 0xFE9C, false, true}; // ث
        case 0x062C: return {0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0, false, true}; // ج
        case 0x062D: return {0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4, false, true}; // ح
        case 0x062E: return {0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8, false, true}; // خ
        case 0x062F: return {0xFEA9, 0xFEAA, 0, 0, true, false}; // د
        case 0x0630: return {0xFEAB, 0xFEAC, 0, 0, true, false}; // ذ
        case 0x0631: return {0xFEAD, 0xFEAE, 0, 0, true, false}; // ر
        case 0x0632: return {0xFEAF, 0xFEB0, 0, 0, true, false}; // ز
        case 0x0633: return {0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4, false, true}; // س
        case 0x0634: return {0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8, false, true}; // ش
        case 0x0635: return {0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC, false, true}; // ص
        case 0x0636: return {0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0, false, true}; // ض
        case 0x0637: return {0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4, false, true}; // ط
        case 0x0638: return {0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8, false, true}; // ظ
        case 0x0639: return {0xFEC9, 0xFECA, 0xFECB, 0xFECC, false, true}; // ع
        case 0x063A: return {0xFECD, 0xFECE, 0xFECF, 0xFED0, false, true}; // غ
        case 0x0641: return {0xFED1, 0xFED2, 0xFED3, 0xFED4, false, true}; // ف
        case 0x0642: return {0xFED5, 0xFED6, 0xFED7, 0xFED8, false, true}; // ق
        case 0x0643: return {0xFED9, 0xFEDA, 0xFEDB, 0xFEDC, false, true}; // ك
        case 0x0644: return {0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0, false, true}; // ل
        case 0x0645: return {0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4, false, true}; // م
        case 0x0646: return {0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8, false, true}; // ن
        case 0x0647: return {0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC, false, true}; // ه
        case 0x0648: return {0xFEED, 0xFEEE, 0, 0, true, false}; // و
        case 0x0649: return {0xFEEF, 0xFEF0, 0, 0, true, false}; // ى
        case 0x064A: return {0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4, false, true}; // ي
        case 0x0621: return {0xFE80, 0, 0, 0, false, false}; // ء (Non-joining)
        default: break;
    }
    return {c, 0, 0, 0, false, false};
}

char32_t get_lam_alif_ligature(char32_t alif, bool is_final) noexcept {
    switch (alif) {
        case 0x0622: return is_final ? 0xFEF6 : 0xFEF5; // لآ
        case 0x0623: return is_final ? 0xFEF8 : 0xFEF7; // لأ
        case 0x0625: return is_final ? 0xFEFA : 0xFEF9; // لإ
        case 0x0627: return is_final ? 0xFEFC : 0xFEFB; // لا
        default: return 0;
    }
}

} // namespace

std::vector<std::pair<char32_t, size_t>> Bidi::decode_utf8(std::string_view text) {
    std::vector<std::pair<char32_t, size_t>> result;
    size_t i = 0;
    const size_t len = text.size();

    while (i < len) {
        size_t byte_pos = i;
        uint8_t b0 = static_cast<uint8_t>(text[i++]);
        char32_t cp = 0;

        if (b0 < 0x80) {
            cp = b0;
        } else if ((b0 & 0xE0) == 0xC0) {
            if (i < len) {
                uint8_t b1 = static_cast<uint8_t>(text[i++]);
                cp = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
            }
        } else if ((b0 & 0xF0) == 0xE0) {
            if (i + 1 < len) {
                uint8_t b1 = static_cast<uint8_t>(text[i++]);
                uint8_t b2 = static_cast<uint8_t>(text[i++]);
                cp = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
            }
        } else if ((b0 & 0xF8) == 0xF0) {
            if (i + 2 < len) {
                uint8_t b1 = static_cast<uint8_t>(text[i++]);
                uint8_t b2 = static_cast<uint8_t>(text[i++]);
                uint8_t b3 = static_cast<uint8_t>(text[i++]);
                cp = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
            }
        }
        result.emplace_back(cp, byte_pos);
    }
    return result;
}

CharType Bidi::classify_char(char32_t c) noexcept {
    if (is_arabic(c)) {
        return CharType::ArabicLetter;
    }
    if ((c >= 0x0590 && c <= 0x05FF) || (c >= 0xFB1D && c <= 0xFB4F)) {
        return CharType::RTL; // Hebrew
    }
    if (c >= '0' && c <= '9') {
        return CharType::EuropeanNumber;
    }
    if (c >= 0x0660 && c <= 0x0669) {
        return CharType::ArabicNumber;
    }
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        return CharType::Whitespace;
    }
    if ((c >= '!' && c <= '/') || (c >= ':' && c <= '@') ||
        (c >= '[' && c <= '`') || (c >= '{' && c <= '~') ||
        c == 0x060C || c == 0x061B || c == 0x061F) {
        return CharType::Neutral;
    }
    return CharType::LTR;
}

bool Bidi::is_arabic(char32_t c) noexcept {
    return (c >= 0x0600 && c <= 0x06FF) ||
           (c >= 0x0750 && c <= 0x077F) ||
           (c >= 0x08A0 && c <= 0x08FF) ||
           (c >= 0xFB50 && c <= 0xFDFF) ||
           (c >= 0xFE70 && c <= 0xFEFF);
}

bool Bidi::is_arabic_diacritic(char32_t c) noexcept {
    return (c >= 0x064B && c <= 0x065F) || (c == 0x0670);
}

bool Bidi::is_cjk(char32_t c) noexcept {
    return (c >= 0x4E00 && c <= 0x9FFF) ||  // CJK Unified Ideographs
           (c >= 0x3400 && c <= 0x4DBF) ||  // CJK Extension A
           (c >= 0x20000 && c <= 0x2A6DF) || // CJK Extension B
           (c >= 0x3040 && c <= 0x309F) ||  // Hiragana
           (c >= 0x30A0 && c <= 0x30FF) ||  // Katakana
           (c >= 0x31F0 && c <= 0x31FF) ||  // Katakana Extensions
           (c >= 0xAC00 && c <= 0xD7AF) ||  // Hangul Syllables
           (c >= 0x1100 && c <= 0x11FF) ||  // Hangul Jamo
           (c >= 0x3000 && c <= 0x303F) ||  // CJK Symbols and Punctuation
           (c >= 0xFF00 && c <= 0xFFEF);    // Fullwidth Forms
}

bool Bidi::is_cjk_no_break_before(char32_t c) noexcept {
    // Closing brackets, quotes, and punctuation that must NOT start a line
    switch (c) {
        case 0x3001: // 、
        case 0x3002: // 。
        case 0x300D: // 」
        case 0x300F: // 』
        case 0x30FB: // ・
        case 0xFF09: // ）
        case 0xFF0C: // ，
        case 0xFF0E: // ．
        case 0xFF01: // ！
        case 0xFF1F: // ？
        case 0xFF1A: // ：
        case 0xFF1B: // ；
        case ')': case ']': case '}': case '>':
        case '!': case '?': case ':': case ';':
        case '.': case ',':
            return true;
        default:
            return false;
    }
}

bool Bidi::is_cjk_no_break_after(char32_t c) noexcept {
    // Opening brackets and quotes that must NOT end a line
    switch (c) {
        case 0x300C: // 「
        case 0x300E: // 『
        case 0xFF08: // （
        case '(': case '[': case '{': case '<':
            return true;
        default:
            return false;
    }
}

Direction Bidi::detect_base_direction(std::string_view text) noexcept {
    auto chars = decode_utf8(text);
    for (const auto& [cp, _] : chars) {
        CharType type = classify_char(cp);
        if (type == CharType::ArabicLetter || type == CharType::RTL) {
            return Direction::RightToLeft;
        }
        if (type == CharType::LTR) {
            return Direction::LeftToRight;
        }
    }
    return Direction::LeftToRight;
}

std::vector<BidiRun> Bidi::segment_runs(std::string_view text, std::optional<Direction> base_dir) {
    std::vector<BidiRun> runs;
    if (text.empty()) return runs;

    Direction base = base_dir.value_or(detect_base_direction(text));
    uint8_t base_level = (base == Direction::RightToLeft) ? 1 : 0;

    auto chars = decode_utf8(text);
    if (chars.empty()) return runs;

    size_t run_start_byte = 0;
    uint8_t current_level = base_level;
    bool in_run = false;

    for (size_t idx = 0; idx < chars.size(); ++idx) {
        char32_t cp = chars[idx].first;
        size_t byte_pos = chars[idx].second;
        CharType type = classify_char(cp);

        uint8_t char_level = current_level;
        if (type == CharType::ArabicLetter || type == CharType::RTL) {
            char_level = 1;
        } else if (type == CharType::LTR) {
            char_level = 0;
        } else if (type == CharType::EuropeanNumber) {
            // Numbers inside RTL context remain level 2 (LTR run inside RTL)
            char_level = (current_level == 1) ? 2 : 0;
        } else {
            // Neutral/whitespace inherits current level
            char_level = current_level;
        }

        if (!in_run) {
            run_start_byte = byte_pos;
            current_level = char_level;
            in_run = true;
        } else if (char_level != current_level && type != CharType::Neutral && type != CharType::Whitespace) {
            runs.push_back(BidiRun{run_start_byte, byte_pos, current_level});
            run_start_byte = byte_pos;
            current_level = char_level;
        }
    }

    if (in_run && run_start_byte < text.size()) {
        runs.push_back(BidiRun{run_start_byte, text.size(), current_level});
    }

    return runs;
}

std::vector<ShapedChar> Bidi::shape_text(std::string_view text, const std::vector<BidiRun>& runs) {
    std::vector<ShapedChar> result;
    auto chars = decode_utf8(text);
    if (chars.empty()) return result;

    for (const auto& run : runs) {
        // Collect characters in this run
        std::vector<std::pair<char32_t, size_t>> run_chars;
        for (const auto& c : chars) {
            if (c.second >= run.start_byte && c.second < run.end_byte) {
                run_chars.push_back(c);
            }
        }
        if (run_chars.empty()) continue;

        if (run.is_rtl()) {
            // Arabic contextual shaping
            std::vector<ShapedChar> shaped_run;
            shaped_run.reserve(run_chars.size());

            for (size_t i = 0; i < run_chars.size(); ++i) {
                char32_t cur = run_chars[i].first;
                size_t byte_idx = run_chars[i].second;

                if (!is_arabic(cur) || is_arabic_diacritic(cur)) {
                    shaped_run.push_back(ShapedChar{cur, byte_idx, run.level, true});
                    continue;
                }

                // Check for Lam-Alif ligature
                if (cur == 0x0644 && (i + 1) < run_chars.size()) { // Lam
                    char32_t next_ch = run_chars[i + 1].first;
                    bool prev_connects = false;
                    if (i > 0) {
                        auto prev_forms = get_arabic_forms(run_chars[i - 1].first);
                        prev_connects = prev_forms.dual_joining;
                    }
                    char32_t lig = get_lam_alif_ligature(next_ch, prev_connects);
                    if (lig != 0) {
                        shaped_run.push_back(ShapedChar{lig, byte_idx, run.level, true});
                        ++i; // consume Alif
                        continue;
                    }
                }

                auto cur_forms = get_arabic_forms(cur);

                // Look backwards for connecting letter (skipping diacritics)
                bool prev_connects = false;
                for (int p = static_cast<int>(i) - 1; p >= 0; --p) {
                    if (is_arabic_diacritic(run_chars[p].first)) continue;
                    auto p_forms = get_arabic_forms(run_chars[p].first);
                    if (p_forms.dual_joining) {
                        prev_connects = true;
                    }
                    break;
                }

                // Look forwards for connecting letter (skipping diacritics)
                bool next_connects = false;
                for (size_t n = i + 1; n < run_chars.size(); ++n) {
                    if (is_arabic_diacritic(run_chars[n].first)) continue;
                    auto n_forms = get_arabic_forms(run_chars[n].first);
                    if (n_forms.dual_joining || n_forms.right_joining) {
                        next_connects = true;
                    }
                    break;
                }

                char32_t shaped_cp = cur;

                if (cur_forms.dual_joining) {
                    if (prev_connects && next_connects) {
                        shaped_cp = cur_forms.medial ? cur_forms.medial : cur;
                    } else if (prev_connects) {
                        shaped_cp = cur_forms.final_form ? cur_forms.final_form : cur;
                    } else if (next_connects) {
                        shaped_cp = cur_forms.initial ? cur_forms.initial : cur;
                    } else {
                        shaped_cp = cur_forms.isolated ? cur_forms.isolated : cur;
                    }
                } else if (cur_forms.right_joining) {
                    if (prev_connects) {
                        shaped_cp = cur_forms.final_form ? cur_forms.final_form : cur;
                    } else {
                        shaped_cp = cur_forms.isolated ? cur_forms.isolated : cur;
                    }
                } else {
                    shaped_cp = cur_forms.isolated ? cur_forms.isolated : cur;
                }

                shaped_run.push_back(ShapedChar{shaped_cp, byte_idx, run.level, true});
            }

            // Reverse the RTL run for visual left-to-right rendering order
            std::reverse(shaped_run.begin(), shaped_run.end());

            for (auto& sc : shaped_run) {
                result.push_back(sc);
            }

        } else {
            // LTR run - preserve original visual order
            for (const auto& [cp, byte_idx] : run_chars) {
                result.push_back(ShapedChar{cp, byte_idx, run.level, false});
            }
        }
    }

    return result;
}

} // namespace nisaba::text
