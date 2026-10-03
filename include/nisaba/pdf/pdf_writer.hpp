#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <span>
#include "nisaba/pdf/pdf_types.hpp"

namespace nisaba::pdf {

/// Low-level PDF Document Writer and Object Table Assembler.
/// Generates compliant ISO 32000-1 PDF documents with compressed Flate streams and xref tables.
class PdfWriter {
public:
    PdfWriter();
    ~PdfWriter() = default;

    /// Allocates a new unique indirect object ID.
    uint32_t allocate_id() noexcept {
        return next_id_++;
    }

    /// Sets the Document Catalog root reference.
    void set_root(PdfRef root_ref) noexcept {
        root_ref_ = root_ref;
    }

    /// Sets the optional Document Information dictionary reference.
    void set_info(PdfRef info_ref) noexcept {
        info_ref_ = info_ref;
    }

    /// Adds or updates an indirect object value.
    void add_object(uint32_t id, PdfValue value, uint16_t gen = 0);

    /// Adds an indirect stream object with optional RFC 1950 zlib Flate compression.
    void add_stream_object(
        uint32_t id,
        PdfDict dict,
        std::span<const uint8_t> stream_data,
        bool compress = true,
        uint16_t gen = 0
    );

    /// Convenience helper: allocates ID and adds object.
    PdfRef add_new_object(PdfValue value) {
        uint32_t id = allocate_id();
        add_object(id, std::move(value));
        return PdfRef(id, 0);
    }

    /// Convenience helper: allocates ID and adds stream.
    PdfRef add_new_stream(PdfDict dict, std::span<const uint8_t> stream_data, bool compress = true) {
        uint32_t id = allocate_id();
        add_stream_object(id, std::move(dict), stream_data, compress);
        return PdfRef(id, 0);
    }

    /// Assembles the complete PDF binary payload.
    [[nodiscard]] std::vector<uint8_t> to_bytes();

    /// Writes the assembled PDF document to a file on disk.
    bool write_to_file(const std::string& path);

private:
    uint32_t next_id_{1};
    std::optional<PdfRef> root_ref_{std::nullopt};
    std::optional<PdfRef> info_ref_{std::nullopt};
    std::vector<PdfIndirectObject> objects_{};
};

} // namespace nisaba::pdf
