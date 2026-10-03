#include "nisaba/pdf/pdf_codec.hpp"
#include "nisaba/image/image_io.hpp"
#include "nisaba/image/deflate.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>

namespace nisaba::pdf {

namespace {

inline bool is_ws(uint8_t c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\0';
}

inline int hex_digit(uint8_t c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// -----------------------------------------------------------------------------
// CCITT Fax 1D & 2D Modified Huffman Tables (ITU-T T.4 / T.6)
// -----------------------------------------------------------------------------
struct HuffmanEntry {
    uint16_t code;
    uint8_t len;
    uint16_t run;
};

// Common White Terminating Codes (runs 0..63)
static const HuffmanEntry kWhiteTerm[] = {
    {0x0035, 8, 0},   {0x0007, 6, 1},   {0x0007, 4, 2},   {0x0008, 4, 3},
    {0x000B, 4, 4},   {0x000C, 4, 5},   {0x000E, 4, 6},   {0x000F, 4, 7},
    {0x0013, 5, 8},   {0x0014, 5, 9},   {0x0007, 5, 10},  {0x0008, 5, 11},
    {0x0008, 6, 12},  {0x0003, 6, 13},  {0x0034, 6, 14},  {0x0035, 6, 15},
    {0x002A, 6, 16},  {0x002B, 6, 17},  {0x0027, 7, 18},  {0x000C, 7, 19},
    {0x0008, 7, 20},  {0x0017, 7, 21},  {0x0003, 7, 22},  {0x0004, 7, 23},
    {0x0028, 7, 24},  {0x002B, 7, 25},  {0x0013, 7, 26},  {0x0024, 7, 27},
    {0x0018, 8, 28},  {0x0002, 8, 29},  {0x0003, 8, 30},  {0x001A, 8, 31},
    {0x001B, 8, 32},  {0x0012, 8, 33},  {0x0013, 8, 34},  {0x0014, 8, 35},
    {0x0015, 8, 36},  {0x0016, 8, 37},  {0x0017, 8, 38},  {0x0028, 8, 39},
    {0x0029, 8, 40},  {0x002A, 8, 41},  {0x002B, 8, 42},  {0x002C, 8, 43},
    {0x002D, 8, 44},  {0x0004, 8, 45},  {0x0005, 8, 46},  {0x000A, 8, 47},
    {0x000B, 8, 48},  {0x0052, 8, 49},  {0x0053, 8, 50},  {0x0054, 8, 51},
    {0x0055, 8, 52},  {0x0024, 8, 53},  {0x0025, 8, 54},  {0x0058, 8, 55},
    {0x0059, 8, 56},  {0x005A, 8, 57},  {0x005B, 8, 58},  {0x004A, 8, 59},
    {0x004B, 8, 60},  {0x0032, 8, 61},  {0x0033, 8, 62},  {0x0034, 8, 63}
};

// White Make-Up Codes (runs 64, 128, ... 2560)
static const HuffmanEntry kWhiteMakeup[] = {
    {0x001B, 5, 64},   {0x0012, 5, 128},  {0x0017, 6, 192},  {0x0037, 7, 256},
    {0x0036, 8, 320},  {0x0037, 8, 384},  {0x0064, 8, 448},  {0x0065, 8, 512},
    {0x0068, 8, 576},  {0x0067, 8, 640},  {0x00CC, 9, 704},  {0x00CD, 9, 768},
    {0x00D2, 9, 832},  {0x00D3, 9, 896},  {0x00D4, 9, 960},  {0x00D5, 9, 1024},
    {0x00D6, 9, 1088}, {0x00D7, 9, 1152}, {0x00D8, 9, 1216}, {0x00D9, 9, 1280},
    {0x00DA, 9, 1344}, {0x00DB, 9, 1408}, {0x0098, 9, 1472}, {0x0099, 9, 1536},
    {0x009A, 9, 1600}, {0x0018, 6, 1664}, {0x009B, 9, 1728}
};

// Black Terminating Codes (runs 0..63)
static const HuffmanEntry kBlackTerm[] = {
    {0x0037, 10, 0},  {0x0002, 3, 1},    {0x0003, 2, 2},    {0x0002, 2, 3},
    {0x0003, 3, 4},   {0x0002, 4, 5},    {0x0003, 4, 6},    {0x0003, 5, 7},
    {0x0005, 6, 8},   {0x0004, 6, 9},    {0x0004, 7, 10},   {0x0005, 7, 11},
    {0x0007, 7, 12},  {0x0004, 8, 13},   {0x0007, 8, 14},   {0x0018, 9, 15},
    {0x0017, 10, 16}, {0x0018, 10, 17},  {0x0008, 10, 18},  {0x0067, 11, 19},
    {0x0068, 11, 20}, {0x006C, 11, 21},  {0x0037, 11, 22},  {0x0028, 11, 23},
    {0x0017, 11, 24}, {0x0018, 11, 25},  {0x00CA, 12, 26},  {0x00CB, 12, 27},
    {0x00CC, 12, 28}, {0x00CD, 12, 29},  {0x0068, 12, 30},  {0x0069, 12, 31},
    {0x006A, 12, 32}, {0x006B, 12, 33},  {0x00D2, 12, 34},  {0x00D3, 12, 35},
    {0x00D4, 12, 36}, {0x00D5, 12, 37},  {0x00D6, 12, 38},  {0x00D7, 12, 39},
    {0x006C, 12, 40}, {0x006D, 12, 41},  {0x00DA, 12, 42},  {0x00DB, 12, 43},
    {0x0054, 12, 44}, {0x0055, 12, 45},  {0x0056, 12, 46},  {0x0057, 12, 47},
    {0x0064, 12, 48}, {0x0065, 12, 49},  {0x0052, 12, 50},  {0x0053, 12, 51},
    {0x0024, 12, 52}, {0x0037, 12, 53},  {0x0038, 12, 54},  {0x0027, 12, 55},
    {0x0028, 12, 56}, {0x0058, 12, 57},  {0x0059, 12, 58},  {0x002B, 12, 59},
    {0x002C, 12, 60}, {0x005A, 12, 61},  {0x0066, 12, 62},  {0x0067, 12, 63}
};

// Black Make-Up Codes (runs 64, 128, ... 2560)
static const HuffmanEntry kBlackMakeup[] = {
    {0x000F, 10, 64},  {0x00C8, 12, 128}, {0x00C9, 12, 192}, {0x005B, 12, 256},
    {0x0033, 12, 320}, {0x0034, 12, 384}, {0x0035, 12, 448}, {0x006C, 13, 512},
    {0x006D, 13, 576}, {0x004A, 13, 640}, {0x004B, 13, 704}, {0x004C, 13, 768},
    {0x004D, 13, 832}, {0x0072, 13, 896}, {0x0073, 13, 960}, {0x0074, 13, 1024},
    {0x0075, 13, 1088},{0x0076, 13, 1152},{0x0077, 13, 1216},{0x0052, 13, 1280},
    {0x0053, 13, 1344},{0x0054, 13, 1408},{0x0055, 13, 1472},{0x005A, 13, 1536},
    {0x005B, 13, 1600},{0x0064, 13, 1664},{0x0065, 13, 1728}
};

class BitStream {
public:
    explicit BitStream(std::span<const uint8_t> data) : data_(data) {}

    bool eof() const noexcept { return byte_idx_ >= data_.size(); }

    int read_bit() noexcept {
        if (byte_idx_ >= data_.size()) return -1;
        int b = (data_[byte_idx_] >> (7 - bit_idx_)) & 1;
        bit_idx_++;
        if (bit_idx_ == 8) {
            bit_idx_ = 0;
            byte_idx_++;
        }
        return b;
    }

    uint32_t peek_bits(int n) noexcept {
        size_t b_idx = byte_idx_;
        int bi_idx = bit_idx_;
        uint32_t val = 0;
        for (int i = 0; i < n; ++i) {
            if (b_idx >= data_.size()) break;
            val = (val << 1) | ((data_[b_idx] >> (7 - bi_idx)) & 1);
            bi_idx++;
            if (bi_idx == 8) {
                bi_idx = 0;
                b_idx++;
            }
        }
        return val;
    }

    void consume_bits(int n) noexcept {
        for (int i = 0; i < n; ++i) {
            read_bit();
        }
    }

private:
    std::span<const uint8_t> data_;
    size_t byte_idx_{0};
    int bit_idx_{0};
};

static int decode_run(BitStream& bs, bool is_white) {
    int total_run = 0;
    while (!bs.eof()) {
        const auto* makeups = is_white ? kWhiteMakeup : kBlackMakeup;
        size_t num_makeups = is_white ? (sizeof(kWhiteMakeup)/sizeof(kWhiteMakeup[0])) : (sizeof(kBlackMakeup)/sizeof(kBlackMakeup[0]));

        bool matched_makeup = false;
        for (size_t i = 0; i < num_makeups; ++i) {
            uint32_t bits = bs.peek_bits(makeups[i].len);
            if (bits == makeups[i].code) {
                bs.consume_bits(makeups[i].len);
                total_run += makeups[i].run;
                matched_makeup = true;
                break;
            }
        }
        if (matched_makeup) continue;

        // Try terminating code
        const auto* terms = is_white ? kWhiteTerm : kBlackTerm;
        size_t num_terms = is_white ? (sizeof(kWhiteTerm)/sizeof(kWhiteTerm[0])) : (sizeof(kBlackTerm)/sizeof(kBlackTerm[0]));

        for (size_t i = 0; i < num_terms; ++i) {
            uint32_t bits = bs.peek_bits(terms[i].len);
            if (bits == terms[i].code) {
                bs.consume_bits(terms[i].len);
                total_run += terms[i].run;
                return total_run;
            }
        }

        // Unrecognized code or corruption
        bs.read_bit();
        break;
    }
    return total_run;
}

} // namespace

std::vector<uint8_t> decode_ascii_hex(std::span<const uint8_t> input) {
    std::vector<uint8_t> out;
    out.reserve(input.size() / 2);

    int high_nibble = -1;
    for (uint8_t c : input) {
        if (is_ws(c)) continue;
        if (c == '>') break; // End of hex stream

        int val = hex_digit(c);
        if (val < 0) continue;

        if (high_nibble < 0) {
            high_nibble = val;
        } else {
            out.push_back(static_cast<uint8_t>((high_nibble << 4) | val));
            high_nibble = -1;
        }
    }

    if (high_nibble >= 0) {
        out.push_back(static_cast<uint8_t>(high_nibble << 4));
    }

    return out;
}

std::vector<uint8_t> decode_ascii_85(std::span<const uint8_t> input) {
    std::vector<uint8_t> out;
    out.reserve((input.size() * 4) / 5);

    uint32_t tuple = 0;
    int count = 0;

    for (size_t i = 0; i < input.size(); ++i) {
        uint8_t c = input[i];
        if (is_ws(c)) continue;

        if (c == '<' && i + 1 < input.size() && input[i + 1] == '~') {
            i++;
            continue;
        }

        if (c == '~') {
            if (i + 1 < input.size() && input[i + 1] == '>') {
                break; // End of data marker ~>
            }
        }

        if (c == 'z' && count == 0) {
            out.push_back(0);
            out.push_back(0);
            out.push_back(0);
            out.push_back(0);
            continue;
        }

        if (c >= '!' && c <= 'u') {
            tuple = tuple * 85 + (c - '!');
            count++;
            if (count == 5) {
                out.push_back(static_cast<uint8_t>((tuple >> 24) & 0xFF));
                out.push_back(static_cast<uint8_t>((tuple >> 16) & 0xFF));
                out.push_back(static_cast<uint8_t>((tuple >> 8) & 0xFF));
                out.push_back(static_cast<uint8_t>(tuple & 0xFF));
                tuple = 0;
                count = 0;
            }
        }
    }

    if (count > 1) {
        for (int i = count; i < 5; ++i) {
            tuple = tuple * 85 + ('u' - '!');
        }
        for (int i = 0; i < count - 1; ++i) {
            out.push_back(static_cast<uint8_t>((tuple >> (24 - 8 * i)) & 0xFF));
        }
    }

    return out;
}

std::vector<uint8_t> decode_run_length(std::span<const uint8_t> input) {
    std::vector<uint8_t> out;
    out.reserve(input.size() * 2);

    size_t i = 0;
    while (i < input.size()) {
        uint8_t len = input[i++];
        if (len == 128) break; // EOD

        if (len < 128) {
            // Literal run of len + 1 bytes
            size_t count = len + 1;
            while (count > 0 && i < input.size()) {
                out.push_back(input[i++]);
                count--;
            }
        } else {
            // Repeat run of (257 - len) bytes
            size_t count = 257 - len;
            if (i < input.size()) {
                uint8_t b = input[i++];
                out.insert(out.end(), count, b);
            }
        }
    }

    return out;
}

std::vector<uint8_t> decode_ccitt_fax(
    std::span<const uint8_t> input,
    const PdfDict& parms
) {
    int k = 0; // Default: Group 3 1D
    int columns = 1728;
    int rows = 0;
    bool black_is_1 = false;

    auto it_k = parms.find("K");
    if (it_k != parms.end()) k = static_cast<int>(it_k->second.as_int(0));
    auto it_c = parms.find("Columns");
    if (it_c != parms.end()) columns = static_cast<int>(it_c->second.as_int(1728));
    auto it_r = parms.find("Rows");
    if (it_r != parms.end()) rows = static_cast<int>(it_r->second.as_int(0));
    auto it_b = parms.find("BlackIs1");
    if (it_b != parms.end()) black_is_1 = it_b->second.as_bool(false);

    if (columns <= 0 || columns > 65536) columns = 1728;
    size_t row_stride = (static_cast<size_t>(columns) + 7) / 8;

    std::vector<uint8_t> out;
    if (rows > 0) {
        out.reserve(row_stride * rows);
    }

    BitStream bs(input);
    std::vector<uint8_t> ref_line(columns, 0); // 0 = white, 1 = black
    std::vector<uint8_t> cur_line(columns, 0);

    int cur_row = 0;
    while (!bs.eof() && (rows == 0 || cur_row < rows)) {
        std::fill(cur_line.begin(), cur_line.end(), 0);

        if (k < 0) {
            // Pure Group 4 2D T.6 Decoding
            int a0 = -1;
            bool a0_color = false; // White

            auto find_changing_element = [](const std::vector<uint8_t>& line, int start, bool target_color) -> int {
                int n = static_cast<int>(line.size());
                int idx = std::max(0, start);
                while (idx < n && (line[idx] != (target_color ? 1 : 0))) {
                    idx++;
                }
                return idx;
            };

            while (a0 < columns && !bs.eof()) {
                // Determine b1 and b2 from ref_line
                int b1 = find_changing_element(ref_line, a0 + 1, !a0_color);
                int b2 = find_changing_element(ref_line, b1, a0_color);

                // Check 2D mode codes
                if (bs.peek_bits(4) == 0x01) { // 0001: Pass mode
                    bs.consume_bits(4);
                    int fill_to = std::min(columns, b2);
                    for (int x = std::max(0, a0); x < fill_to; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    a0 = b2;
                } else if (bs.peek_bits(3) == 0x01) { // 001: Horizontal mode
                    bs.consume_bits(3);
                    int run1 = decode_run(bs, !a0_color);
                    int run2 = decode_run(bs, a0_color);

                    int a1 = std::min(columns, std::max(0, a0) + run1);
                    for (int x = std::max(0, a0); x < a1; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    int a2 = std::min(columns, a1 + run2);
                    for (int x = a1; x < a2; ++x) {
                        cur_line[x] = (!a0_color) ? 1 : 0;
                    }
                    a0 = a2;
                } else if (bs.peek_bits(1) == 0x01) { // 1: V(0)
                    bs.consume_bits(1);
                    int a1 = std::min(columns, b1);
                    for (int x = std::max(0, a0); x < a1; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    a0 = a1;
                    a0_color = !a0_color;
                } else if (bs.peek_bits(3) == 0x03) { // 011: VR(1)
                    bs.consume_bits(3);
                    int a1 = std::min(columns, b1 + 1);
                    for (int x = std::max(0, a0); x < a1; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    a0 = a1;
                    a0_color = !a0_color;
                } else if (bs.peek_bits(3) == 0x02) { // 010: VL(1)
                    bs.consume_bits(3);
                    int a1 = std::min(columns, std::max(0, b1 - 1));
                    for (int x = std::max(0, a0); x < a1; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    a0 = a1;
                    a0_color = !a0_color;
                } else if (bs.peek_bits(6) == 0x03) { // 000011: VR(2)
                    bs.consume_bits(6);
                    int a1 = std::min(columns, b1 + 2);
                    for (int x = std::max(0, a0); x < a1; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    a0 = a1;
                    a0_color = !a0_color;
                } else if (bs.peek_bits(6) == 0x02) { // 000010: VL(2)
                    bs.consume_bits(6);
                    int a1 = std::min(columns, std::max(0, b1 - 2));
                    for (int x = std::max(0, a0); x < a1; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    a0 = a1;
                    a0_color = !a0_color;
                } else if (bs.peek_bits(7) == 0x03) { // 0000011: VR(3)
                    bs.consume_bits(7);
                    int a1 = std::min(columns, b1 + 3);
                    for (int x = std::max(0, a0); x < a1; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    a0 = a1;
                    a0_color = !a0_color;
                } else if (bs.peek_bits(7) == 0x02) { // 0000010: VL(3)
                    bs.consume_bits(7);
                    int a1 = std::min(columns, std::max(0, b1 - 3));
                    for (int x = std::max(0, a0); x < a1; ++x) {
                        cur_line[x] = a0_color ? 1 : 0;
                    }
                    a0 = a1;
                    a0_color = !a0_color;
                } else {
                    // Unknown or EOFB
                    bs.read_bit();
                    break;
                }
            }
        } else {
            // Group 3 1D Modified Huffman Decoding
            int x = 0;
            bool is_white = true;
            while (x < columns && !bs.eof()) {
                int run = decode_run(bs, is_white);
                int end_x = std::min(columns, x + run);
                for (int i = x; i < end_x; ++i) {
                    cur_line[i] = is_white ? 0 : 1;
                }
                x = end_x;
                is_white = !is_white;
            }
        }

        // Pack cur_line into output row
        for (size_t b = 0; b < row_stride; ++b) {
            uint8_t byte_val = 0;
            for (int bit = 0; bit < 8; ++bit) {
                int px = static_cast<int>(b * 8 + bit);
                if (px < columns) {
                    uint8_t bit_val = cur_line[px];
                    if (!black_is_1) {
                        bit_val = 1 - bit_val; // Invert: 1 = white, 0 = black
                    }
                    byte_val |= (bit_val << (7 - bit));
                }
            }
            out.push_back(byte_val);
        }

        ref_line = cur_line;
        cur_row++;
    }

    return out;
}

std::vector<uint8_t> decode_stream_filters(
    std::span<const uint8_t> input,
    const PdfValue& filter_val,
    const PdfValue& decode_parms_val
) {
    if (input.empty()) return {};

    std::vector<std::string> filters;
    std::vector<PdfDict> parms_list;

    if (filter_val.is_name()) {
        std::string n = filter_val.as_name();
        if (!n.empty() && n.front() == '/') n = n.substr(1);
        filters.push_back(n);

        PdfDict p_dict;
        if (decode_parms_val.is_dict()) p_dict = decode_parms_val.as_dict();
        parms_list.push_back(p_dict);
    } else if (filter_val.is_array()) {
        for (const auto& elem : filter_val.as_array()) {
            if (elem.is_name()) {
                std::string n = elem.as_name();
                if (!n.empty() && n.front() == '/') n = n.substr(1);
                filters.push_back(n);
            }
        }
        if (decode_parms_val.is_array()) {
            for (const auto& elem : decode_parms_val.as_array()) {
                if (elem.is_dict()) parms_list.push_back(elem.as_dict());
                else parms_list.push_back(PdfDict{});
            }
        }
        while (parms_list.size() < filters.size()) {
            parms_list.push_back(PdfDict{});
        }
    }

    if (filters.empty()) {
        return std::vector<uint8_t>(input.begin(), input.end());
    }

    std::vector<uint8_t> current(input.begin(), input.end());

    // Apply filters in sequence
    for (size_t f = 0; f < filters.size(); ++f) {
        const auto& filter_name = filters[f];
        const auto& parms = parms_list[f];

        if (filter_name == "FlateDecode" || filter_name == "Fl") {
            auto decomp = image::zlib_decompress(current);
            if (decomp.has_value()) {
                current = std::move(*decomp.value);
            } else {
                auto raw_decomp = image::deflate_decompress(current);
                if (raw_decomp.has_value()) {
                    current = std::move(*raw_decomp.value);
                }
            }
        } else if (filter_name == "ASCIIHexDecode" || filter_name == "AHx") {
            current = decode_ascii_hex(current);
        } else if (filter_name == "ASCII85Decode" || filter_name == "A85") {
            current = decode_ascii_85(current);
        } else if (filter_name == "RunLengthDecode" || filter_name == "RL") {
            current = decode_run_length(current);
        } else if (filter_name == "CCITTFaxDecode" || filter_name == "CCF") {
            current = decode_ccitt_fax(current, parms);
        } else if (filter_name == "DCTDecode" || filter_name == "DCT") {
            // JPEG pass-through (handled by image reader)
        }
    }

    return current;
}

} // namespace nisaba::pdf
