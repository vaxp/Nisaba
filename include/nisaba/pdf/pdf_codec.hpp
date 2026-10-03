#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <string>
#include <optional>
#include "nisaba/pdf/pdf_types.hpp"

namespace nisaba::pdf {

/// Decodes an ASCIIHex-encoded byte stream (ISO 32000-1 §7.4.2).
std::vector<uint8_t> decode_ascii_hex(std::span<const uint8_t> input);

/// Decodes an ASCII85-encoded byte stream (ISO 32000-1 §7.4.3).
std::vector<uint8_t> decode_ascii_85(std::span<const uint8_t> input);

/// Decodes a RunLength-encoded byte stream (ISO 32000-1 §7.4.5).
std::vector<uint8_t> decode_run_length(std::span<const uint8_t> input);

/// Decodes a CCITTFax-encoded byte stream (ISO 32000-1 §7.4.6, Group 3 / Group 4).
std::vector<uint8_t> decode_ccitt_fax(
    std::span<const uint8_t> input,
    const PdfDict& parms = PdfDict{}
);

/// Decodes a stream through its chain of PDF filters (Filter & DecodeParms).
std::vector<uint8_t> decode_stream_filters(
    std::span<const uint8_t> input,
    const PdfValue& filter_val,
    const PdfValue& decode_parms_val
);

} // namespace nisaba::pdf
