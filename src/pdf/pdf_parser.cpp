#include "nisaba/pdf/pdf_parser.hpp"
#include "nisaba/image/deflate.hpp"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <cmath>

namespace nisaba::pdf {

namespace {

inline bool is_pdf_whitespace(uint8_t c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\0';
}

inline bool is_pdf_delimiter(uint8_t c) {
    return c == '(' || c == ')' || c == '<' || c == '>' ||
           c == '[' || c == ']' || c == '{' || c == '}' ||
           c == '/' || c == '%';
}

uint8_t hex_to_byte(char c) {
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
    return 0;
}

} // namespace

bool PdfParser::load(std::span<const uint8_t> pdf_data) {
    if (pdf_data.size() < 16) return false;
    data_ = pdf_data;
    cursor_ = 0;
    xref_offsets_.clear();
    trailer_ = PdfDict{};
    pages_.clear();
    security_handler_ = nullptr;

    // Verify PDF header %PDF-
    std::string_view header(reinterpret_cast<const char*>(data_.data()), std::min<size_t>(10, data_.size()));
    if (header.find("%PDF-") == std::string_view::npos) {
        return false;
    }

    if (!parse_trailer_and_xref()) {
        fallback_scan_objects();
    }

    // Check for encryption in trailer
    auto it_enc = trailer_.find("Encrypt");
    if (it_enc != trailer_.end()) {
        PdfDict enc_dict;
        if (it_enc->second.is_dict()) {
            enc_dict = it_enc->second.as_dict();
        } else if (it_enc->second.is_ref()) {
            auto enc_obj = resolve(it_enc->second.as_ref());
            if (enc_obj && enc_obj->value.is_dict()) enc_dict = enc_obj->value.as_dict();
        }

        PdfArray id_arr;
        auto it_id = trailer_.find("ID");
        if (it_id != trailer_.end() && it_id->second.is_array()) {
            id_arr = it_id->second.as_array();
        }

        if (!enc_dict.empty()) {
            security_handler_ = std::make_shared<PdfSecurityHandler>(enc_dict, id_arr);
        }
    }

    return parse_pages();
}

bool PdfParser::authenticate(const std::string& password) {
    if (!security_handler_) return true;
    bool ok = security_handler_->authenticate(password);
    if (ok) {
        pages_.clear();
        parse_pages();
    }
    return ok;
}

bool PdfParser::is_encrypted() const noexcept {
    return security_handler_ && security_handler_->is_encrypted();
}

bool PdfParser::is_authenticated() const noexcept {
    return !security_handler_ || security_handler_->is_authenticated();
}

const ParsedPdfPage* PdfParser::page(size_t index) const noexcept {
    if (index >= pages_.size()) return nullptr;
    return &pages_[index];
}

std::string PdfParser::info_field(std::string_view key) const {
    if (!info_ref_.has_value()) return "";
    PdfParser* non_const = const_cast<PdfParser*>(this);
    auto info_obj = non_const->resolve(*info_ref_);
    if (!info_obj.has_value() || !info_obj->value.is_dict()) return "";

    const auto* val = info_obj->value.find(key);
    if (val && val->is_string()) {
        return val->as_string();
    }
    return "";
}

void PdfParser::skip_whitespace_and_comments() {
    while (cursor_ < data_.size()) {
        uint8_t c = data_[cursor_];
        if (is_pdf_whitespace(c)) {
            cursor_++;
        } else if (c == '%') {
            // Comment extends to end of line
            while (cursor_ < data_.size() && data_[cursor_] != '\r' && data_[cursor_] != '\n') {
                cursor_++;
            }
        } else {
            break;
        }
    }
}

std::string PdfParser::read_token() {
    skip_whitespace_and_comments();
    if (cursor_ >= data_.size()) return "";

    uint8_t c = data_[cursor_];

    // Check 2-char delimiters << or >>
    if (cursor_ + 1 < data_.size()) {
        if (c == '<' && data_[cursor_ + 1] == '<') {
            cursor_ += 2;
            return "<<";
        }
        if (c == '>' && data_[cursor_ + 1] == '>') {
            cursor_ += 2;
            return ">>";
        }
    }

    // Single character delimiters
    if (c == '[' || c == ']') {
        cursor_++;
        return std::string(1, static_cast<char>(c));
    }

    // Name token
    if (c == '/') {
        return parse_name();
    }

    // General token (keyword, number, etc.)
    size_t start = cursor_;
    while (cursor_ < data_.size()) {
        uint8_t ch = data_[cursor_];
        if (is_pdf_whitespace(ch) || is_pdf_delimiter(ch)) {
            break;
        }
        cursor_++;
    }

    return std::string(
        reinterpret_cast<const char*>(data_.data() + start),
        cursor_ - start
    );
}

std::string PdfParser::parse_name() {
    if (cursor_ >= data_.size() || data_[cursor_] != '/') return "";
    cursor_++; // skip '/'

    std::string name;
    while (cursor_ < data_.size()) {
        uint8_t c = data_[cursor_];
        if (is_pdf_whitespace(c) || is_pdf_delimiter(c)) break;
        if (c == '#' && cursor_ + 2 < data_.size()) {
            // Hex escape #xx
            uint8_t h1 = hex_to_byte(static_cast<char>(data_[cursor_ + 1]));
            uint8_t h2 = hex_to_byte(static_cast<char>(data_[cursor_ + 2]));
            name += static_cast<char>((h1 << 4) | h2);
            cursor_ += 3;
        } else {
            name += static_cast<char>(c);
            cursor_++;
        }
    }
    return "/" + name;
}

std::string PdfParser::parse_literal_string() {
    if (cursor_ >= data_.size() || data_[cursor_] != '(') return "";
    cursor_++; // skip '('

    std::string str;
    int depth = 1;

    while (cursor_ < data_.size() && depth > 0) {
        char c = static_cast<char>(data_[cursor_++]);
        if (c == '\\') {
            if (cursor_ < data_.size()) {
                char next_c = static_cast<char>(data_[cursor_++]);
                switch (next_c) {
                    case 'n': str += '\n'; break;
                    case 'r': str += '\r'; break;
                    case 't': str += '\t'; break;
                    case 'b': str += '\b'; break;
                    case 'f': str += '\f'; break;
                    case '(': str += '('; break;
                    case ')': str += ')'; break;
                    case '\\': str += '\\'; break;
                    default: str += next_c; break;
                }
            }
        } else if (c == '(') {
            depth++;
            str += c;
        } else if (c == ')') {
            depth--;
            if (depth > 0) str += c;
        } else {
            str += c;
        }
    }
    return str;
}

std::string PdfParser::parse_hex_string() {
    if (cursor_ >= data_.size() || data_[cursor_] != '<') return "";
    cursor_++; // skip '<'

    std::string out;
    std::string hex_chars;
    while (cursor_ < data_.size()) {
        char c = static_cast<char>(data_[cursor_++]);
        if (c == '>') break;
        if (!is_pdf_whitespace(static_cast<uint8_t>(c))) {
            hex_chars += c;
        }
    }
    if (hex_chars.size() % 2 != 0) {
        hex_chars += '0';
    }
    for (size_t i = 0; i < hex_chars.size(); i += 2) {
        uint8_t h1 = hex_to_byte(hex_chars[i]);
        uint8_t h2 = hex_to_byte(hex_chars[i + 1]);
        out += static_cast<char>((h1 << 4) | h2);
    }
    return out;
}

PdfDict PdfParser::parse_dict() {
    PdfDict dict;
    while (cursor_ < data_.size() && dict.size() < 4096) {
        skip_whitespace_and_comments();
        if (cursor_ + 1 < data_.size() && data_[cursor_] == '>' && data_[cursor_ + 1] == '>') {
            cursor_ += 2;
            break;
        }
        std::string key_token = read_token();
        if (key_token.empty() || key_token == ">>") break;
        if (key_token.front() != '/') continue;

        std::string key = key_token.substr(1);
        PdfValue val = parse_value();
        dict[key] = std::move(val);
    }
    return dict;
}

PdfArray PdfParser::parse_array() {
    PdfArray arr;
    while (cursor_ < data_.size() && arr.size() < 32768) {
        skip_whitespace_and_comments();
        if (cursor_ < data_.size() && data_[cursor_] == ']') {
            cursor_++;
            break;
        }
        PdfValue val = parse_value();
        if (val.is_null() && cursor_ < data_.size() && data_[cursor_] == ']') {
            cursor_++;
            break;
        }
        arr.push_back(std::move(val));
    }
    return arr;
}

PdfValue PdfParser::parse_value() {
    skip_whitespace_and_comments();
    if (cursor_ >= data_.size()) return PdfValue();

    uint8_t c = data_[cursor_];

    if (c == '<') {
        if (cursor_ + 1 < data_.size() && data_[cursor_ + 1] == '<') {
            cursor_ += 2;
            return PdfValue(parse_dict());
        }
        return PdfValue(parse_hex_string());
    }

    if (c == '(') {
        return PdfValue(parse_literal_string());
    }

    if (c == '[') {
        cursor_++;
        return PdfValue(parse_array());
    }

    if (c == '/') {
        std::string name = parse_name();
        return PdfValue(PdfName(name.substr(1)));
    }

    std::string token = read_token();

    if (token == "true") return PdfValue(true);
    if (token == "false") return PdfValue(false);
    if (token == "null") return PdfValue();

    // Check if token is a number
    char* endptr = nullptr;
    int64_t int_val = std::strtoll(token.c_str(), &endptr, 10);
    if (endptr && *endptr == '\0') {
        // Lookahead: could this be an indirect reference "id gen R"?
        size_t lookahead = cursor_;
        skip_whitespace_and_comments();
        std::string tok2 = read_token();
        char* endptr2 = nullptr;
        int64_t gen_val = std::strtoll(tok2.c_str(), &endptr2, 10);
        if (endptr2 && *endptr2 == '\0') {
            skip_whitespace_and_comments();
            std::string tok3 = read_token();
            if (tok3 == "R") {
                return PdfValue(PdfRef(static_cast<uint32_t>(int_val), static_cast<uint16_t>(gen_val)));
            }
        }
        // Not a reference, backtrack
        cursor_ = lookahead;
        return PdfValue(int_val);
    }

    double dbl_val = std::strtod(token.c_str(), &endptr);
    if (endptr && *endptr == '\0') {
        return PdfValue(dbl_val);
    }

    return PdfValue();
}

std::vector<uint8_t> PdfParser::apply_predictor(
    std::span<const uint8_t> in,
    int predictor,
    int columns,
    int colors,
    int bits_per_component
) {
    if (predictor <= 1 || in.empty()) {
        return std::vector<uint8_t>(in.begin(), in.end());
    }

    if (columns < 1) columns = 1;
    if (colors < 1) colors = 1;
    if (bits_per_component < 1) bits_per_component = 8;

    int bpp = (colors * bits_per_component + 7) / 8;
    if (bpp < 1) bpp = 1;

    size_t row_bytes = static_cast<size_t>((columns * colors * bits_per_component + 7) / 8);
    if (row_bytes == 0) return std::vector<uint8_t>(in.begin(), in.end());

    if (predictor == 2) {
        // TIFF Predictor 2 (horizontal differencing)
        std::vector<uint8_t> out(in.begin(), in.end());
        size_t num_rows = out.size() / row_bytes;
        for (size_t y = 0; y < num_rows; ++y) {
            uint8_t* row = out.data() + y * row_bytes;
            for (size_t x = static_cast<size_t>(bpp); x < row_bytes; ++x) {
                row[x] = static_cast<uint8_t>(row[x] + row[x - bpp]);
            }
        }
        return out;
    }

    if (predictor >= 10 && predictor <= 15) {
        // PNG Predictors
        size_t src_stride = row_bytes + 1;
        if (src_stride == 0) return std::vector<uint8_t>(in.begin(), in.end());
        size_t num_rows = in.size() / src_stride;
        if (num_rows == 0) return std::vector<uint8_t>(in.begin(), in.end());

        std::vector<uint8_t> out(num_rows * row_bytes);

        for (size_t y = 0; y < num_rows; ++y) {
            const uint8_t* src_row = in.data() + y * src_stride;
            uint8_t filter_type = src_row[0];
            const uint8_t* in_data = src_row + 1;
            uint8_t* curr = out.data() + y * row_bytes;
            const uint8_t* prior = (y > 0) ? (out.data() + (y - 1) * row_bytes) : nullptr;

            std::memcpy(curr, in_data, row_bytes);

            switch (filter_type) {
                case 0: // None
                    break;
                case 1: { // Sub
                    for (size_t x = static_cast<size_t>(bpp); x < row_bytes; ++x) {
                        curr[x] = static_cast<uint8_t>(curr[x] + curr[x - bpp]);
                    }
                    break;
                }
                case 2: { // Up
                    if (prior) {
                        for (size_t x = 0; x < row_bytes; ++x) {
                            curr[x] = static_cast<uint8_t>(curr[x] + prior[x]);
                        }
                    }
                    break;
                }
                case 3: { // Average
                    for (size_t x = 0; x < row_bytes; ++x) {
                        int left = (x >= static_cast<size_t>(bpp)) ? curr[x - bpp] : 0;
                        int above = prior ? prior[x] : 0;
                        curr[x] = static_cast<uint8_t>(curr[x] + ((left + above) >> 1));
                    }
                    break;
                }
                case 4: { // Paeth
                    for (size_t x = 0; x < row_bytes; ++x) {
                        int a = (x >= static_cast<size_t>(bpp)) ? curr[x - bpp] : 0;
                        int b = prior ? prior[x] : 0;
                        int c = (prior && x >= static_cast<size_t>(bpp)) ? prior[x - bpp] : 0;
                        int p = a + b - c;
                        int pa = std::abs(p - a);
                        int pb = std::abs(p - b);
                        int pc = std::abs(p - c);
                        int pr = (pa <= pb && pa <= pc) ? a : ((pb <= pc) ? b : c);
                        curr[x] = static_cast<uint8_t>(curr[x] + pr);
                    }
                    break;
                }
                default:
                    break;
            }
        }
        return out;
    }

    return std::vector<uint8_t>(in.begin(), in.end());
}

size_t PdfParser::parse_classic_xref_table(size_t xref_offset) {
    cursor_ = xref_offset;
    skip_whitespace_and_comments();
    std::string xref_tok = read_token();
    if (xref_tok != "xref") return 0;

    bool saw_trailer = false;
    while (cursor_ < data_.size()) {
        skip_whitespace_and_comments();
        size_t save_c = cursor_;
        std::string first_tok = read_token();
        if (first_tok == "trailer") {
            saw_trailer = true;
            break;
        }
        if (first_tok.empty()) break;

        char* endptr = nullptr;
        uint32_t first_id = static_cast<uint32_t>(std::strtoul(first_tok.c_str(), &endptr, 10));
        if (!endptr || *endptr != '\0') {
            cursor_ = save_c;
            break;
        }

        std::string count_tok = read_token();
        if (count_tok.empty()) break;
        uint32_t count = static_cast<uint32_t>(std::strtoul(count_tok.c_str(), nullptr, 10));

        for (uint32_t i = 0; i < count; ++i) {
            std::string off_str = read_token();
            std::string gen_str = read_token();
            std::string status = read_token();
            if (status == "n") {
                size_t off = static_cast<size_t>(std::strtoull(off_str.c_str(), nullptr, 10));
                uint32_t obj_id = first_id + i;
                if (xref_offsets_.find(obj_id) == xref_offsets_.end()) {
                    xref_offsets_[obj_id] = off;
                }
            }
        }
    }

    if (!saw_trailer) {
        skip_whitespace_and_comments();
        if (read_token() == "trailer") {
            saw_trailer = true;
        }
    }

    size_t prev_offset = 0;
    if (saw_trailer) {
        skip_whitespace_and_comments();
        if (cursor_ + 1 < data_.size() && data_[cursor_] == '<' && data_[cursor_ + 1] == '<') {
            cursor_ += 2;
            PdfDict tr = parse_dict();
            if (trailer_.empty()) {
                trailer_ = tr;
                auto it_root = trailer_.find("Root");
                if (it_root != trailer_.end() && it_root->second.is_ref()) {
                    root_ref_ = it_root->second.as_ref();
                }
                auto it_info = trailer_.find("Info");
                if (it_info != trailer_.end() && it_info->second.is_ref()) {
                    info_ref_ = it_info->second.as_ref();
                }
            }

            auto it_prev = tr.find("Prev");
            if (it_prev != tr.end() && it_prev->second.is_int()) {
                prev_offset = static_cast<size_t>(it_prev->second.as_int());
            }
        }
    }

    return prev_offset;
}

size_t PdfParser::parse_xref_stream(size_t xref_stream_offset) {
    cursor_ = xref_stream_offset;
    skip_whitespace_and_comments();

    std::string id_tok = read_token();
    std::string gen_tok = read_token();
    std::string obj_tok = read_token();

    if (obj_tok != "obj") return 0;

    PdfValue val = parse_value();
    if (!val.is_dict()) return 0;

    std::string type_name;
    if (const auto* t = val.find("Type")) {
        if (t->is_name()) type_name = t->as_name();
    }
    if (type_name != "XRef") return 0;

    if (trailer_.empty()) {
        trailer_ = val.as_dict();
        if (const auto* r = val.find("Root")) {
            if (r->is_ref()) root_ref_ = r->as_ref();
        }
        if (const auto* info = val.find("Info")) {
            if (info->is_ref()) info_ref_ = info->as_ref();
        }
    }

    size_t prev_offset = 0;
    if (const auto* prev_val = val.find("Prev")) {
        if (prev_val->is_int()) {
            prev_offset = static_cast<size_t>(prev_val->as_int());
        }
    }

    const auto* w_val = val.find("W");
    if (!w_val || !w_val->is_array()) return prev_offset;
    const auto& w_arr = w_val->as_array();
    if (w_arr.size() < 3) return prev_offset;

    size_t w0 = static_cast<size_t>(w_arr[0].as_int(1));
    size_t w1 = static_cast<size_t>(w_arr[1].as_int(0));
    size_t w2 = static_cast<size_t>(w_arr[2].as_int(0));
    size_t entry_len = w0 + w1 + w2;
    if (entry_len == 0) return prev_offset;

    std::vector<std::pair<uint32_t, uint32_t>> index_subsections;
    if (const auto* idx_val = val.find("Index")) {
        if (idx_val->is_array()) {
            const auto& arr = idx_val->as_array();
            for (size_t i = 0; i + 1 < arr.size(); i += 2) {
                uint32_t s = static_cast<uint32_t>(arr[i].as_int(0));
                uint32_t c = static_cast<uint32_t>(arr[i + 1].as_int(0));
                index_subsections.push_back({s, c});
            }
        }
    }
    if (index_subsections.empty()) {
        int sz = 0;
        if (const auto* sz_val = val.find("Size")) {
            sz = sz_val->as_int(0);
        }
        if (sz > 0) {
            index_subsections.push_back({0, static_cast<uint32_t>(sz)});
        }
    }

    skip_whitespace_and_comments();
    std::string strm_tok = read_token();
    if (strm_tok != "stream") return prev_offset;

    if (cursor_ < data_.size() && data_[cursor_] == '\r') cursor_++;
    if (cursor_ < data_.size() && data_[cursor_] == '\n') cursor_++;

    size_t stream_start = cursor_;
    size_t stream_len = 0;
    if (const auto* len_val = val.find("Length")) {
        if (len_val->is_int()) stream_len = static_cast<size_t>(len_val->as_int());
    }

    std::span<const uint8_t> raw_stream;
    if (stream_len > 0 && stream_start + stream_len <= data_.size()) {
        raw_stream = data_.subspan(stream_start, stream_len);
    } else {
        std::string_view rem(reinterpret_cast<const char*>(data_.data() + stream_start), data_.size() - stream_start);
        size_t end_pos = rem.find("endstream");
        if (end_pos != std::string_view::npos) {
            size_t actual_len = end_pos;
            while (actual_len > 0 && (rem[actual_len - 1] == '\r' || rem[actual_len - 1] == '\n')) {
                actual_len--;
            }
            raw_stream = data_.subspan(stream_start, actual_len);
        }
    }

    std::vector<uint8_t> decompressed_bytes;
    bool is_flate = false;
    if (const auto* flt = val.find("Filter")) {
        if (flt->is_name() && flt->as_name() == "FlateDecode") is_flate = true;
    }

    if (is_flate && !raw_stream.empty()) {
        auto decomp = image::zlib_decompress(raw_stream);
        if (decomp.has_value()) {
            decompressed_bytes = std::move(*decomp.value);
        } else {
            auto raw_decomp = image::deflate_decompress(raw_stream);
            if (raw_decomp.has_value()) {
                decompressed_bytes = std::move(*raw_decomp.value);
            } else {
                decompressed_bytes.assign(raw_stream.begin(), raw_stream.end());
            }
        }
    } else {
        decompressed_bytes.assign(raw_stream.begin(), raw_stream.end());
    }

    int predictor = 1;
    int columns = static_cast<int>(entry_len);
    int colors = 1;
    int bpc = 8;
    if (const auto* parms = val.find("DecodeParms")) {
        if (const auto* p = parms->find("Predictor")) predictor = p->as_int(1);
        if (const auto* c = parms->find("Columns")) columns = c->as_int(columns);
        if (const auto* col = parms->find("Colors")) colors = col->as_int(1);
        if (const auto* b = parms->find("BitsPerComponent")) bpc = b->as_int(8);
    }

    if (predictor > 1) {
        decompressed_bytes = apply_predictor(decompressed_bytes, predictor, columns, colors, bpc);
    }

    auto read_field = [](const uint8_t* p, size_t len) -> uint64_t {
        uint64_t val = 0;
        for (size_t i = 0; i < len; ++i) {
            val = (val << 8) | p[i];
        }
        return val;
    };

    size_t stream_pos = 0;
    for (const auto& [start_id, count] : index_subsections) {
        for (uint32_t i = 0; i < count; ++i) {
            if (stream_pos + entry_len > decompressed_bytes.size()) break;

            const uint8_t* ptr = decompressed_bytes.data() + stream_pos;
            uint32_t type = (w0 > 0) ? static_cast<uint32_t>(read_field(ptr, w0)) : 1;
            uint64_t field1 = (w1 > 0) ? read_field(ptr + w0, w1) : 0;
            uint32_t field2 = (w2 > 0) ? static_cast<uint32_t>(read_field(ptr + w0 + w1, w2)) : 0;

            uint32_t obj_id = start_id + i;
            if (type == 1) {
                if (xref_offsets_.find(obj_id) == xref_offsets_.end()) {
                    xref_offsets_[obj_id] = static_cast<size_t>(field1);
                }
            } else if (type == 2) {
                if (compressed_objects_.find(obj_id) == compressed_objects_.end()) {
                    compressed_objects_[obj_id] = CompressedObjEntry{
                        .objstm_id = static_cast<uint32_t>(field1),
                        .index_in_stream = field2
                    };
                }
            }

            stream_pos += entry_len;
        }
    }

    return prev_offset;
}

bool PdfParser::parse_trailer_and_xref() {
    xref_offsets_.clear();
    compressed_objects_.clear();
    objstm_offsets_cache_.clear();
    trailer_.clear();

    size_t search_start = data_.size() > 2048 ? data_.size() - 2048 : 0;
    std::string_view tail(reinterpret_cast<const char*>(data_.data() + search_start), data_.size() - search_start);

    size_t pos = tail.rfind("startxref");
    if (pos == std::string_view::npos) return false;

    size_t startxref_pos = search_start + pos + 9;
    cursor_ = startxref_pos;
    skip_whitespace_and_comments();

    std::string offset_tok = read_token();
    if (offset_tok.empty()) return false;
    size_t cur_xref_offset = static_cast<size_t>(std::strtoull(offset_tok.c_str(), nullptr, 10));

    std::vector<size_t> visited;

    while (cur_xref_offset > 0 && cur_xref_offset < data_.size()) {
        if (std::find(visited.begin(), visited.end(), cur_xref_offset) != visited.end()) {
            break;
        }
        visited.push_back(cur_xref_offset);

        cursor_ = cur_xref_offset;
        skip_whitespace_and_comments();
        std::string tok = read_token();

        size_t prev_off = 0;
        if (tok == "xref") {
            prev_off = parse_classic_xref_table(cur_xref_offset);
        } else {
            prev_off = parse_xref_stream(cur_xref_offset);
        }

        if (prev_off == 0) break;
        cur_xref_offset = prev_off;
    }

    return !trailer_.empty() && root_ref_.id != 0;
}

std::optional<PdfIndirectObject> PdfParser::resolve_compressed_object(
    uint32_t obj_id,
    const CompressedObjEntry& entry
) {
    auto it_cache = objstm_offsets_cache_.find(entry.objstm_id);
    if (it_cache == objstm_offsets_cache_.end()) {
        auto stm_obj = resolve(PdfRef(entry.objstm_id, 0));
        if (!stm_obj || !stm_obj->stream_data.has_value() || !stm_obj->value.is_dict()) {
            return std::nullopt;
        }

        const auto& stream_bytes = *stm_obj->stream_data;

        int n_objs = 0;
        if (const auto* n_val = stm_obj->value.find("N")) n_objs = static_cast<int>(n_val->as_int(0));
        size_t first_offset = 0;
        if (const auto* first_val = stm_obj->value.find("First")) {
            first_offset = static_cast<size_t>(first_val->as_int(0));
        }

        if (n_objs <= 0 || first_offset >= stream_bytes.size()) {
            return std::nullopt;
        }

        std::map<uint32_t, size_t> offsets_map;
        size_t hdr_pos = 0;

        auto skip_hdr_ws = [&]() {
            while (hdr_pos < first_offset && is_pdf_whitespace(stream_bytes[hdr_pos])) {
                hdr_pos++;
            }
        };

        auto read_hdr_num = [&]() -> uint32_t {
            skip_hdr_ws();
            if (hdr_pos >= first_offset) return 0;
            size_t s = hdr_pos;
            while (hdr_pos < first_offset && std::isdigit(stream_bytes[hdr_pos])) {
                hdr_pos++;
            }
            if (s == hdr_pos) return 0;
            std::string tok(reinterpret_cast<const char*>(stream_bytes.data() + s), hdr_pos - s);
            return static_cast<uint32_t>(std::strtoul(tok.c_str(), nullptr, 10));
        };

        for (int i = 0; i < n_objs; ++i) {
            uint32_t id = read_hdr_num();
            uint32_t rel_off = read_hdr_num();
            size_t abs_off = first_offset + rel_off;
            if (abs_off < stream_bytes.size()) {
                offsets_map[id] = abs_off;
            }
        }

        objstm_offsets_cache_[entry.objstm_id] = std::move(offsets_map);
        it_cache = objstm_offsets_cache_.find(entry.objstm_id);
    }

    auto it_off = it_cache->second.find(obj_id);
    if (it_off == it_cache->second.end()) {
        return std::nullopt;
    }

    size_t target_offset = it_off->second;

    auto stm_obj = resolve(PdfRef(entry.objstm_id, 0));
    if (!stm_obj || !stm_obj->stream_data.has_value()) return std::nullopt;
    const auto& stream_bytes = *stm_obj->stream_data;
    if (target_offset >= stream_bytes.size()) return std::nullopt;

    std::span<const uint8_t> saved_data = data_;
    size_t saved_cursor = cursor_;

    data_ = std::span<const uint8_t>(stream_bytes.data(), stream_bytes.size());
    cursor_ = target_offset;
    skip_whitespace_and_comments();

    PdfValue parsed_val = parse_value();

    data_ = saved_data;
    cursor_ = saved_cursor;

    return PdfIndirectObject{
        .id = obj_id,
        .gen = 0,
        .value = std::move(parsed_val),
        .stream_data = std::nullopt
    };
}

void PdfParser::fallback_scan_objects() {
    xref_offsets_.clear();
    cursor_ = 0;

    std::string_view content(reinterpret_cast<const char*>(data_.data()), data_.size());
    size_t search_pos = 0;

    while (search_pos < content.size()) {
        size_t obj_pos = content.find("obj", search_pos);
        if (obj_pos == std::string_view::npos) break;

        // Trace backwards to find "<id> <gen> obj"
        if (obj_pos > 4 && is_pdf_whitespace(static_cast<uint8_t>(content[obj_pos - 1]))) {
            size_t p = obj_pos - 1;
            while (p > 0 && is_pdf_whitespace(static_cast<uint8_t>(content[p]))) p--;
            // Read gen
            size_t gen_end = p + 1;
            while (p > 0 && std::isdigit(content[p])) p--;
            size_t gen_start = p + 1;

            if (gen_start < gen_end && is_pdf_whitespace(static_cast<uint8_t>(content[p]))) {
                while (p > 0 && is_pdf_whitespace(static_cast<uint8_t>(content[p]))) p--;
                size_t id_end = p + 1;
                while (p > 0 && std::isdigit(content[p])) p--;
                size_t id_start = (p == 0 && std::isdigit(content[p])) ? 0 : p + 1;

                if (id_start < id_end) {
                    std::string id_str(content.substr(id_start, id_end - id_start));
                    uint32_t id = static_cast<uint32_t>(std::strtoul(id_str.c_str(), nullptr, 10));
                    xref_offsets_[id] = id_start;
                }
            }
        }
        search_pos = obj_pos + 3;
    }

    // Try finding trailer manually
    size_t trailer_pos = content.rfind("trailer");
    if (trailer_pos != std::string_view::npos) {
        cursor_ = trailer_pos + 7;
        skip_whitespace_and_comments();
        if (cursor_ + 1 < data_.size() && data_[cursor_] == '<' && data_[cursor_ + 1] == '<') {
            cursor_ += 2;
            trailer_ = parse_dict();
            auto it_root = trailer_.find("Root");
            if (it_root != trailer_.end() && it_root->second.is_ref()) {
                root_ref_ = it_root->second.as_ref();
            }
            auto it_info = trailer_.find("Info");
            if (it_info != trailer_.end() && it_info->second.is_ref()) {
                info_ref_ = it_info->second.as_ref();
            }
        }
    }
}

std::optional<PdfIndirectObject> PdfParser::resolve(PdfRef ref) {
    if (ref.id == 0) return std::nullopt;

    // Detect circular reference!
    static thread_local std::vector<uint32_t> resolving_stack;
    if (std::find(resolving_stack.begin(), resolving_stack.end(), ref.id) != resolving_stack.end()) {
        return std::nullopt;
    }
    resolving_stack.push_back(ref.id);
    struct StackGuard {
        std::vector<uint32_t>& st;
        ~StackGuard() { if (!st.empty()) st.pop_back(); }
    } guard{resolving_stack};

    // Check if object is compressed in an Object Stream (/ObjStm)
    auto it_comp = compressed_objects_.find(ref.id);
    if (it_comp != compressed_objects_.end()) {
        return resolve_compressed_object(ref.id, it_comp->second);
    }

    auto it = xref_offsets_.find(ref.id);
    if (it == xref_offsets_.end()) return std::nullopt;

    size_t saved_cursor = cursor_;
    struct CursorGuard {
        size_t& c;
        size_t saved;
        ~CursorGuard() { c = saved; }
    } cursor_guard{cursor_, saved_cursor};

    cursor_ = it->second;
    skip_whitespace_and_comments();

    std::string id_tok = read_token();
    std::string gen_tok = read_token();
    std::string obj_tok = read_token();

    if (obj_tok != "obj") return std::nullopt;

    PdfValue val = parse_value();
    std::optional<std::vector<uint8_t>> stream_bytes = std::nullopt;

    skip_whitespace_and_comments();
    std::string strm_tok = read_token();

    if (strm_tok == "stream") {
        // Skip single \r\n or \n
        if (cursor_ < data_.size() && data_[cursor_] == '\r') cursor_++;
        if (cursor_ < data_.size() && data_[cursor_] == '\n') cursor_++;

        size_t stream_start = cursor_;
        size_t stream_len = 0;

        // Check if length is declared in dict
        if (val.is_dict()) {
            if (const auto* len_val = val.find("Length")) {
                if (len_val->is_int()) {
                    stream_len = static_cast<size_t>(len_val->as_int());
                } else if (len_val->is_ref()) {
                    auto len_obj = resolve(len_val->as_ref());
                    if (len_obj && len_obj->value.is_int()) {
                        stream_len = static_cast<size_t>(len_obj->value.as_int());
                    }
                }
            }
        }

        if (stream_start + stream_len > data_.size()) {
            stream_len = 0;
        }

        std::span<const uint8_t> raw_stream;
        if (stream_len > 0) {
            raw_stream = data_.subspan(stream_start, stream_len);
            cursor_ = stream_start + stream_len;
        } else {
            // Find "endstream"
            std::string_view rem(
                reinterpret_cast<const char*>(data_.data() + stream_start),
                data_.size() - stream_start
            );
            size_t end_pos = rem.find("endstream");
            if (end_pos != std::string_view::npos) {
                // Trim trailing \r\n
                size_t actual_len = end_pos;
                while (actual_len > 0 && (rem[actual_len - 1] == '\r' || rem[actual_len - 1] == '\n')) {
                    actual_len--;
                }
                raw_stream = data_.subspan(stream_start, actual_len);
                cursor_ = stream_start + end_pos;
            }
        }

        std::vector<uint8_t> processed_stream;
        if (security_handler_ && security_handler_->is_authenticated() && !raw_stream.empty()) {
            processed_stream = security_handler_->decrypt_stream(ref.id, ref.gen, raw_stream);
        } else {
            processed_stream.assign(raw_stream.begin(), raw_stream.end());
        }

        PdfValue filter_val;
        PdfValue decode_parms_val;
        if (val.is_dict()) {
            if (const auto* flt = val.find("Filter")) filter_val = *flt;
            if (const auto* parms = val.find("DecodeParms")) decode_parms_val = *parms;
        }

        if (!processed_stream.empty()) {
            processed_stream = decode_stream_filters(processed_stream, filter_val, decode_parms_val);
        }

        // Apply predictor if specified in DecodeParms
        if (!processed_stream.empty() && val.is_dict()) {
            if (const auto* parms = val.find("DecodeParms")) {
                int predictor = 1;
                if (const auto* p = parms->find("Predictor")) predictor = static_cast<int>(p->as_int(1));
                if (predictor > 1) {
                    int cols = 1;
                    int colors = 1;
                    int bpc = 8;
                    if (const auto* c = parms->find("Columns")) cols = static_cast<int>(c->as_int(1));
                    if (const auto* col = parms->find("Colors")) colors = static_cast<int>(col->as_int(1));
                    if (const auto* b = parms->find("BitsPerComponent")) bpc = static_cast<int>(b->as_int(8));
                    processed_stream = apply_predictor(processed_stream, predictor, cols, colors, bpc);
                }
            }
        }
        stream_bytes = std::move(processed_stream);
    }

    return PdfIndirectObject{
        .id = ref.id,
        .gen = ref.gen,
        .value = std::move(val),
        .stream_data = std::move(stream_bytes)
    };
}

bool PdfParser::parse_pages() {
    if (root_ref_.id == 0) return false;

    pages_.clear();
    page_ref_to_index_.clear();

    auto catalog_obj = resolve(root_ref_);
    if (!catalog_obj.has_value() || !catalog_obj->value.is_dict()) return false;

    const auto* pages_val = catalog_obj->value.find("Pages");
    if (!pages_val || !pages_val->is_ref()) return false;

    auto pages_root_obj = resolve(pages_val->as_ref());
    if (!pages_root_obj.has_value() || !pages_root_obj->value.is_dict()) return false;

    collect_pages(pages_root_obj->value, {}, pages_val->as_ref());
    return !pages_.empty();
}

void PdfParser::collect_pages(const PdfValue& node_val, const PdfDict& inherited_res, PdfRef page_ref) {
    if (!node_val.is_dict()) return;

    PdfDict current_res = inherited_res;
    if (const auto* res = node_val.find("Resources")) {
        if (res->is_dict()) {
            current_res = res->as_dict();
        } else if (res->is_ref()) {
            auto res_obj = resolve(res->as_ref());
            if (res_obj && res_obj->value.is_dict()) {
                current_res = res_obj->value.as_dict();
            }
        }
    }

    const auto* type_val = node_val.find("Type");
    std::string type = (type_val && type_val->is_name()) ? type_val->as_name() : "";

    if (type == "Pages") {
        const auto* kids_val = node_val.find("Kids");
        if (kids_val && kids_val->is_array()) {
            for (const auto& kid : kids_val->as_array()) {
                if (kid.is_ref()) {
                    auto child_obj = resolve(kid.as_ref());
                    if (child_obj.has_value()) {
                        collect_pages(child_obj->value, current_res, kid.as_ref());
                    }
                }
            }
        }
    } else if (type == "Page" || type.empty()) {
        ParsedPdfPage page;
        page.page_index = static_cast<uint32_t>(pages_.size());
        page.page_ref = page_ref;
        if (page_ref.id != 0) {
            page_ref_to_index_[page_ref] = static_cast<int>(page.page_index);
        }
        page.page_dict = node_val.as_dict();

        // MediaBox
        if (const auto* mb = node_val.find("MediaBox")) {
            if (mb->is_array() && mb->as_array().size() >= 4) {
                const auto& arr = mb->as_array();
                float x0 = arr[0].as_float(0.0f);
                float y0 = arr[1].as_float(0.0f);
                float x1 = arr[2].as_float(595.28f);
                float y1 = arr[3].as_float(841.89f);
                page.media_box = Rect::from_xywh(x0, y0, x1 - x0, y1 - y0).value_or(Rect());
            }
        }

        // Resources
        page.resources = current_res;

        // Contents stream: handle direct stream, ref to stream, or (in)direct array of stream refs
        auto extract_stream = [&](const auto& self, const PdfValue& val) -> void {
            if (val.is_ref()) {
                auto c_obj = resolve(val.as_ref());
                if (c_obj) {
                    if (c_obj->stream_data.has_value()) {
                        page.contents.insert(
                            page.contents.end(),
                            c_obj->stream_data->begin(),
                            c_obj->stream_data->end()
                        );
                        page.contents.push_back('\n');
                    } else if (c_obj->value.is_array()) {
                        self(self, c_obj->value);
                    }
                }
            } else if (val.is_array()) {
                for (const auto& c_item : val.as_array()) {
                    self(self, c_item);
                }
            }
        };

        if (const auto* cont = node_val.find("Contents")) {
            extract_stream(extract_stream, *cont);
        }

        pages_.push_back(std::move(page));
    }
}

namespace {
static const PdfValue* find_dict_key(const PdfDict& d, std::string_view k) {
    auto it = d.find(std::string(k));
    if (it != d.end()) return &it->second;
    return nullptr;
}
} // namespace

PdfDestination PdfParser::parse_destination(const PdfValue& dest_val) const {
    PdfDestination dest;
    if (dest_val.is_null()) return dest;

    // Direct destination array: [page /XYZ left top zoom] or [page /Fit] etc.
    if (dest_val.is_array()) {
        const auto& arr = dest_val.as_array();
        if (arr.empty()) return dest;

        // Element 0: Page target
        if (arr[0].is_ref()) {
            auto it = page_ref_to_index_.find(arr[0].as_ref());
            if (it != page_ref_to_index_.end()) {
                dest.page_index = it->second;
            }
        } else if (arr[0].is_int()) {
            dest.page_index = static_cast<int>(arr[0].as_int());
        }

        if (arr.size() > 1 && arr[1].is_name()) {
            std::string type_name = arr[1].as_name();
            if (type_name == "XYZ") {
                dest.type = DestinationType::XYZ;
                if (arr.size() > 2 && !arr[2].is_null()) dest.left = arr[2].as_float();
                if (arr.size() > 3 && !arr[3].is_null()) dest.top = arr[3].as_float();
                if (arr.size() > 4 && !arr[4].is_null()) dest.zoom = arr[4].as_float();
            } else if (type_name == "Fit") {
                dest.type = DestinationType::Fit;
            } else if (type_name == "FitH") {
                dest.type = DestinationType::FitH;
                if (arr.size() > 2 && !arr[2].is_null()) dest.top = arr[2].as_float();
            } else if (type_name == "FitV") {
                dest.type = DestinationType::FitV;
                if (arr.size() > 2 && !arr[2].is_null()) dest.left = arr[2].as_float();
            } else if (type_name == "FitR") {
                dest.type = DestinationType::FitR;
                if (arr.size() > 2 && !arr[2].is_null()) dest.left = arr[2].as_float();
                if (arr.size() > 3 && !arr[3].is_null()) dest.bottom = arr[3].as_float();
                if (arr.size() > 4 && !arr[4].is_null()) dest.right = arr[4].as_float();
                if (arr.size() > 5 && !arr[5].is_null()) dest.top = arr[5].as_float();
            } else if (type_name == "FitB") {
                dest.type = DestinationType::FitB;
            } else if (type_name == "FitBH") {
                dest.type = DestinationType::FitBH;
                if (arr.size() > 2 && !arr[2].is_null()) dest.top = arr[2].as_float();
            } else if (type_name == "FitBV") {
                dest.type = DestinationType::FitBV;
                if (arr.size() > 2 && !arr[2].is_null()) dest.left = arr[2].as_float();
            }
        }
        return dest;
    }

    // Named destination: name or string
    if (dest_val.is_name() || dest_val.is_string()) {
        dest.type = DestinationType::Named;
        dest.named_dest = dest_val.is_string() ? dest_val.as_string() : dest_val.as_name();
        auto resolved = resolve_named_destination(dest.named_dest);
        if (resolved) {
            return *resolved;
        }
        return dest;
    }

    // Dictionary with /D entry
    if (dest_val.is_dict()) {
        if (const auto* d = dest_val.find("D")) {
            return parse_destination(*d);
        }
    }

    return dest;
}

PdfAction PdfParser::parse_action(const PdfValue& action_val) const {
    PdfAction act;
    if (action_val.is_null()) return act;

    const PdfValue* val_ptr = &action_val;
    std::optional<PdfIndirectObject> resolved_obj;
    if (action_val.is_ref()) {
        resolved_obj = const_cast<PdfParser*>(this)->resolve(action_val.as_ref());
        if (resolved_obj) val_ptr = &resolved_obj->value;
    }

    if (!val_ptr->is_dict()) return act;
    const auto& dict = val_ptr->as_dict();

    auto it_s = dict.find("S");
    if (it_s == dict.end() || !it_s->second.is_name()) return act;
    std::string s_type = it_s->second.as_name();

    if (s_type == "GoTo") {
        act.type = PdfActionType::GoTo;
        auto it_d = dict.find("D");
        if (it_d != dict.end()) {
            act.destination = parse_destination(it_d->second);
        }
    } else if (s_type == "URI") {
        act.type = PdfActionType::URI;
        auto it_uri = dict.find("URI");
        if (it_uri != dict.end()) {
            act.uri = it_uri->second.is_string() ? it_uri->second.as_string() : "";
        }
    } else if (s_type == "Named") {
        act.type = PdfActionType::Named;
        auto it_n = dict.find("N");
        if (it_n != dict.end()) {
            act.named_action = it_n->second.is_name() ? it_n->second.as_name() : "";
        }
    } else if (s_type == "GoToR") {
        act.type = PdfActionType::GoToR;
        auto it_f = dict.find("F");
        if (it_f != dict.end()) {
            act.file_path = it_f->second.is_string() ? it_f->second.as_string() : "";
        }
        auto it_d = dict.find("D");
        if (it_d != dict.end()) {
            act.destination = parse_destination(it_d->second);
        }
    } else if (s_type == "Launch") {
        act.type = PdfActionType::Launch;
        auto it_f = dict.find("F");
        if (it_f != dict.end()) {
            act.file_path = it_f->second.is_string() ? it_f->second.as_string() : "";
        }
    }

    return act;
}

std::optional<PdfDestination> PdfParser::resolve_named_destination(std::string_view name) const {
    if (root_ref_.id == 0) return std::nullopt;
    auto cat_obj = const_cast<PdfParser*>(this)->resolve(root_ref_);
    if (!cat_obj || !cat_obj->value.is_dict()) return std::nullopt;
    const auto& cat_dict = cat_obj->value.as_dict();

    // 1. Direct /Dests dictionary in Catalog
    auto it_dests = cat_dict.find("Dests");
    if (it_dests != cat_dict.end()) {
        const PdfValue* dests_val = &it_dests->second;
        std::optional<PdfIndirectObject> d_obj;
        if (dests_val->is_ref()) {
            d_obj = const_cast<PdfParser*>(this)->resolve(dests_val->as_ref());
            if (d_obj) dests_val = &d_obj->value;
        }
        if (dests_val->is_dict()) {
            auto it_match = dests_val->find(name);
            if (it_match != nullptr) {
                return parse_destination(*it_match);
            }
        }
    }

    // 2. /Names -> /Dests name tree in Catalog
    auto it_names = cat_dict.find("Names");
    if (it_names != cat_dict.end()) {
        const PdfValue* names_val = &it_names->second;
        std::optional<PdfIndirectObject> n_obj;
        if (names_val->is_ref()) {
            n_obj = const_cast<PdfParser*>(this)->resolve(names_val->as_ref());
            if (n_obj) names_val = &n_obj->value;
        }
        if (names_val->is_dict()) {
            if (const auto* nd = names_val->find("Dests")) {
                const PdfValue* nd_val = nd;
                std::optional<PdfIndirectObject> nd_obj;
                if (nd_val->is_ref()) {
                    nd_obj = const_cast<PdfParser*>(this)->resolve(nd_val->as_ref());
                    if (nd_obj) nd_val = &nd_obj->value;
                }
                if (nd_val->is_dict()) {
                    if (const auto* names_arr = nd_val->find("Names")) {
                        if (names_arr->is_array()) {
                            const auto& arr = names_arr->as_array();
                            for (size_t i = 0; i + 1 < arr.size(); i += 2) {
                                std::string key = arr[i].is_string() ? arr[i].as_string() : (arr[i].is_name() ? arr[i].as_name() : "");
                                if (key == name) {
                                    return parse_destination(arr[i + 1]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return std::nullopt;
}

std::vector<PdfOutlineItem> PdfParser::read_outlines() {
    std::vector<PdfOutlineItem> result;
    if (root_ref_.id == 0) return result;

    auto cat_obj = resolve(root_ref_);
    if (!cat_obj || !cat_obj->value.is_dict()) return result;

    const auto* outlines_val = cat_obj->value.find("Outlines");
    if (!outlines_val) return result;

    PdfRef outlines_ref{};
    if (outlines_val->is_ref()) {
        outlines_ref = outlines_val->as_ref();
    }
    auto outlines_obj = resolve(outlines_ref);
    if (!outlines_obj || !outlines_obj->value.is_dict()) return result;

    const auto* first_val = outlines_obj->value.find("First");
    if (!first_val || !first_val->is_ref()) return result;

    std::set<uint32_t> visited;
    return parse_outline_level(first_val->as_ref(), visited, 0);
}

std::vector<PdfOutlineItem> PdfParser::parse_outline_level(PdfRef first_ref, std::set<uint32_t>& visited, int depth) {
    std::vector<PdfOutlineItem> items;
    if (depth > 32 || first_ref.id == 0) return items;

    PdfRef current_ref = first_ref;
    while (current_ref.id != 0 && visited.find(current_ref.id) == visited.end()) {
        visited.insert(current_ref.id);
        auto item_obj = resolve(current_ref);
        if (!item_obj || !item_obj->value.is_dict()) break;

        const auto& dict = item_obj->value.as_dict();
        PdfOutlineItem item;

        // /Title
        if (const auto* t = find_dict_key(dict, "Title")) {
            std::string raw_title = t->is_string() ? t->as_string() : "";
            item.title = decode_pdf_doc_string(raw_title);
        }

        // /Dest or /A
        if (const auto* dest = find_dict_key(dict, "Dest")) {
            item.destination = parse_destination(*dest);
        }
        if (const auto* act = find_dict_key(dict, "A")) {
            item.action = parse_action(*act);
            if (item.destination.page_index < 0 && item.action.destination.page_index >= 0) {
                item.destination = item.action.destination;
            }
        }

        // /C (color [r g b])
        if (const auto* c = find_dict_key(dict, "C")) {
            if (c->is_array() && c->as_array().size() >= 3) {
                float r = c->as_array()[0].as_float(0.0f);
                float g = c->as_array()[1].as_float(0.0f);
                float b = c->as_array()[2].as_float(0.0f);
                item.color = Color::from_rgba(r, g, b, 1.0f).value_or(Color::BLACK);
            }
        }

        // /F (flags: bit 1 italic, bit 2 bold)
        if (const auto* f = find_dict_key(dict, "F")) {
            int flags = static_cast<int>(f->as_int(0));
            item.italic = (flags & 1) != 0;
            item.bold = (flags & 2) != 0;
        }

        // Children /First
        if (const auto* child_first = find_dict_key(dict, "First")) {
            if (child_first->is_ref()) {
                item.children = parse_outline_level(child_first->as_ref(), visited, depth + 1);
            }
        }

        items.push_back(std::move(item));

        // Sibling /Next
        if (const auto* next_val = find_dict_key(dict, "Next")) {
            if (next_val->is_ref()) {
                current_ref = next_val->as_ref();
            } else {
                break;
            }
        } else {
            break;
        }
    }

    return items;
}

std::vector<PdfLinkAnnotation> PdfParser::read_page_links(size_t page_index) {
    std::vector<PdfLinkAnnotation> links;
    if (page_index >= pages_.size()) return links;

    const auto& page = pages_[page_index];
    const auto* annots_val = find_dict_key(page.page_dict, "Annots");
    if (!annots_val) return links;

    const PdfValue* arr_val = annots_val;
    std::optional<PdfIndirectObject> annots_obj;
    if (annots_val->is_ref()) {
        annots_obj = resolve(annots_val->as_ref());
        if (annots_obj) arr_val = &annots_obj->value;
    }

    if (!arr_val->is_array()) return links;

    for (const auto& item_val : arr_val->as_array()) {
        const PdfValue* dict_val = &item_val;
        std::optional<PdfIndirectObject> item_obj;
        if (item_val.is_ref()) {
            item_obj = resolve(item_val.as_ref());
            if (item_obj) dict_val = &item_obj->value;
        }

        if (!dict_val->is_dict()) continue;
        const auto& dict = dict_val->as_dict();

        auto it_st = dict.find("Subtype");
        if (it_st == dict.end() || !it_st->second.is_name() || it_st->second.as_name() != "Link") {
            continue;
        }

        PdfLinkAnnotation link;

        // /Rect [llx lly urx ury]
        if (const auto* r = find_dict_key(dict, "Rect")) {
            if (r->is_array() && r->as_array().size() >= 4) {
                const auto& arr = r->as_array();
                float x0 = arr[0].as_float(0.0f);
                float y0 = arr[1].as_float(0.0f);
                float x1 = arr[2].as_float(0.0f);
                float y1 = arr[3].as_float(0.0f);
                link.rect = Rect::from_xywh(
                    std::min(x0, x1),
                    std::min(y0, y1),
                    std::abs(x1 - x0),
                    std::abs(y1 - y0)
                ).value_or(Rect());
            }
        }

        // /Dest
        if (const auto* dest = find_dict_key(dict, "Dest")) {
            link.destination = parse_destination(*dest);
            link.target_page = link.destination.page_index;
        }

        // /A
        if (const auto* act = find_dict_key(dict, "A")) {
            link.action = parse_action(*act);
            if (link.action.type == PdfActionType::URI) {
                link.uri = link.action.uri;
            } else if (link.action.type == PdfActionType::GoTo) {
                if (link.destination.page_index < 0) {
                    link.destination = link.action.destination;
                    link.target_page = link.destination.page_index;
                }
            }
        }

        links.push_back(std::move(link));
    }

    return links;
}

} // namespace nisaba::pdf
