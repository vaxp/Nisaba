#include "nisaba/pdf/pdf_writer.hpp"
#include "nisaba/image/deflate.hpp"
#include <fstream>
#include <cstring>
#include <cstdio>
#include <algorithm>

namespace nisaba::pdf {

PdfWriter::PdfWriter() = default;

void PdfWriter::add_object(uint32_t id, PdfValue value, uint16_t gen) {
    if (id >= next_id_) {
        next_id_ = id + 1;
    }
    // Check if replacing existing
    for (auto& obj : objects_) {
        if (obj.id == id && obj.gen == gen) {
            obj.value = std::move(value);
            obj.stream_data = std::nullopt;
            return;
        }
    }
    objects_.emplace_back(id, gen, std::move(value), std::nullopt);
}

void PdfWriter::add_stream_object(
    uint32_t id,
    PdfDict dict,
    std::span<const uint8_t> stream_data,
    bool compress,
    uint16_t gen
) {
    if (id >= next_id_) {
        next_id_ = id + 1;
    }

    std::vector<uint8_t> final_data;
    if (compress && !stream_data.empty()) {
        auto comp_res = image::zlib_compress(stream_data, 6);
        if (comp_res.has_value() && comp_res.value->size() < stream_data.size()) {
            final_data = std::move(*comp_res.value);
            dict["Filter"] = PdfName("FlateDecode");
        } else {
            final_data.assign(stream_data.begin(), stream_data.end());
        }
    } else {
        final_data.assign(stream_data.begin(), stream_data.end());
    }

    dict["Length"] = static_cast<int64_t>(final_data.size());

    // Check if replacing existing
    for (auto& obj : objects_) {
        if (obj.id == id && obj.gen == gen) {
            obj.value = std::move(dict);
            obj.stream_data = std::move(final_data);
            return;
        }
    }

    objects_.emplace_back(id, gen, std::move(dict), std::move(final_data));
}

std::vector<uint8_t> PdfWriter::to_bytes() {
    std::vector<uint8_t> buffer;
    buffer.reserve(32768);

    auto append_str = [&buffer](std::string_view s) {
        buffer.insert(buffer.end(), s.begin(), s.end());
    };

    // 1. PDF Header
    append_str("%PDF-1.4\n%\xE2\xE3\xCF\xD3\n");

    // Sort objects by ID for clean serialization
    std::sort(objects_.begin(), objects_.end(), [](const auto& a, const auto& b) {
        if (a.id != b.id) return a.id < b.id;
        return a.gen < b.gen;
    });

    uint32_t max_id = 0;
    for (const auto& obj : objects_) {
        if (obj.id > max_id) max_id = obj.id;
    }

    std::vector<size_t> offsets(max_id + 1, 0);

    // 2. Body Objects
    for (const auto& obj : objects_) {
        offsets[obj.id] = buffer.size();

        std::string obj_header = std::to_string(obj.id) + " " + std::to_string(obj.gen) + " obj\n";
        append_str(obj_header);

        std::string val_str;
        serialize_pdf_value(obj.value, val_str);
        append_str(val_str);

        if (obj.stream_data.has_value()) {
            append_str("\nstream\n");
            buffer.insert(buffer.end(), obj.stream_data->begin(), obj.stream_data->end());
            append_str("\nendstream");
        }

        append_str("\nendobj\n");
    }

    // 3. Cross-Reference Table
    size_t xref_offset = buffer.size();
    std::string xref_header = "xref\n0 " + std::to_string(max_id + 1) + "\n";
    append_str(xref_header);

    // Entry 0 is always free
    append_str("0000000000 65535 f \n");

    for (uint32_t i = 1; i <= max_id; ++i) {
        char entry[32];
        if (offsets[i] != 0) {
            snprintf(entry, sizeof(entry), "%010zu 00000 n \n", offsets[i]);
        } else {
            snprintf(entry, sizeof(entry), "0000000000 00000 f \n");
        }
        append_str(entry);
    }

    // 4. Trailer
    append_str("trailer\n<<\n");
    std::string size_entry = "  /Size " + std::to_string(max_id + 1) + "\n";
    append_str(size_entry);

    if (root_ref_.has_value()) {
        std::string root_entry = "  /Root " + std::to_string(root_ref_->id) + " " +
                                 std::to_string(root_ref_->gen) + " R\n";
        append_str(root_entry);
    }

    if (info_ref_.has_value()) {
        std::string info_entry = "  /Info " + std::to_string(info_ref_->id) + " " +
                                 std::to_string(info_ref_->gen) + " R\n";
        append_str(info_entry);
    }

    append_str(">>\nstartxref\n");
    append_str(std::to_string(xref_offset) + "\n%%EOF\n");

    return buffer;
}

bool PdfWriter::write_to_file(const std::string& path) {
    auto data = to_bytes();
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    return out.good();
}

} // namespace nisaba::pdf
