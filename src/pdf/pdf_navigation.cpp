#include "nisaba/pdf/pdf_navigation.hpp"
#include <array>

namespace nisaba::pdf {

namespace {

// Standard PDFDocEncoding to Unicode table for bytes 0x80 to 0xFF (ISO 32000-1 Annex D).
static const uint16_t kPdfDocEncodingToUnicode[128] = {
    0x2022, 0x2020, 0x2021, 0x2026, 0x2014, 0x2013, 0x0192, 0x2044,
    0x2039, 0x203A, 0x2212, 0x2030, 0x201E, 0x201C, 0x201D, 0x2018,
    0x2019, 0x201A, 0x2122, 0xFB01, 0xFB02, 0x0141, 0x0152, 0x0160,
    0x0178, 0x017D, 0x0131, 0x0142, 0x0153, 0x0161, 0x017E, 0x0000,
    0x20AC, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

static void append_utf8(std::string& out, uint32_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x110000) {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

} // namespace

std::string decode_pdf_doc_string(std::string_view raw) {
    if (raw.empty()) return {};

    // 1. Check for UTF-16BE Byte Order Mark (\xFE\xFF)
    if (raw.size() >= 2 &&
        static_cast<uint8_t>(raw[0]) == 0xFE &&
        static_cast<uint8_t>(raw[1]) == 0xFF) {
        std::string out;
        out.reserve((raw.size() - 2));

        for (size_t i = 2; i + 1 < raw.size(); i += 2) {
            uint16_t u = (static_cast<uint8_t>(raw[i]) << 8) | static_cast<uint8_t>(raw[i + 1]);

            // Handle surrogate pairs
            if (u >= 0xD800 && u <= 0xDBFF && i + 3 < raw.size()) {
                uint16_t low = (static_cast<uint8_t>(raw[i + 2]) << 8) | static_cast<uint8_t>(raw[i + 3]);
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    uint32_t cp = 0x10000 + (((u & 0x3FF) << 10) | (low & 0x3FF));
                    append_utf8(out, cp);
                    i += 2;
                    continue;
                }
            }

            append_utf8(out, u);
        }
        return out;
    }

    // 2. Check for UTF-8 Byte Order Mark (\xEF\xBB\xBF)
    if (raw.size() >= 3 &&
        static_cast<uint8_t>(raw[0]) == 0xEF &&
        static_cast<uint8_t>(raw[1]) == 0xBB &&
        static_cast<uint8_t>(raw[2]) == 0xBF) {
        return std::string(raw.substr(3));
    }

    // 3. Standard PDFDocEncoding
    std::string out;
    out.reserve(raw.size());
    for (uint8_t c : raw) {
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
        } else {
            uint16_t cp = kPdfDocEncodingToUnicode[c - 0x80];
            if (cp != 0) {
                append_utf8(out, cp);
            }
        }
    }
    return out;
}

} // namespace nisaba::pdf
