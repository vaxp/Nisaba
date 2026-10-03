#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <optional>
#include <span>
#include "nisaba/pdf/pdf_types.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/pdf/pdf_codec.hpp"
#include "nisaba/pdf/pdf_crypto.hpp"
#include "nisaba/pdf/pdf_navigation.hpp"
#include <set>

namespace nisaba::pdf {

/// Information about a single page discovered during PDF parsing.
struct ParsedPdfPage {
    uint32_t page_index{0};
    PdfRef page_ref{};
    Rect media_box = Rect::from_xywh(0.0f, 0.0f, 595.28f, 841.89f).value_or(Rect()); // [x, y, w, h]
    PdfDict resources{};
    std::vector<uint8_t> contents{}; // Combined decompressed content stream bytes
    PdfDict page_dict{};            // Raw page dictionary (containing /Annots, /Rotate, etc.)
};

/// High-performance sovereign PDF Lexer, Object Parser, and Cross-Reference Resolver.
/// Decodes PDF documents from memory with zero external dependencies.
class PdfParser {
public:
    PdfParser() = default;
    ~PdfParser() = default;

    /// Loads and parses a PDF document from an in-memory byte buffer.
    bool load(std::span<const uint8_t> pdf_data);

    /// Number of pages extracted from the page tree.
    [[nodiscard]] size_t page_count() const noexcept { return pages_.size(); }

    /// Access page info by index.
    [[nodiscard]] const ParsedPdfPage* page(size_t index) const noexcept;

    /// Trailer dictionary.
    [[nodiscard]] const PdfDict& trailer() const noexcept { return trailer_; }

    /// Resolves an indirect object reference into its parsed object and optional decompressed stream.
    [[nodiscard]] std::optional<PdfIndirectObject> resolve(PdfRef ref);

    /// Document information metadata (title, author, etc.).
    [[nodiscard]] std::string info_field(std::string_view key) const;

    /// Attempts to authenticate with user or owner password.
    bool authenticate(const std::string& password = "");

    /// Checks if the document is encrypted.
    [[nodiscard]] bool is_encrypted() const noexcept;

    /// Checks if the document has been successfully authenticated.
    [[nodiscard]] bool is_authenticated() const noexcept;

    [[nodiscard]] const std::shared_ptr<PdfSecurityHandler>& security_handler() const noexcept {
        return security_handler_;
    }

    /// Returns the Document Outline / Bookmarks hierarchy (ISO 32000-1 §12.3.3).
    [[nodiscard]] std::vector<PdfOutlineItem> read_outlines();

    /// Returns all interactive link annotations on a given page (ISO 32000-1 §12.5.6.5).
    [[nodiscard]] std::vector<PdfLinkAnnotation> read_page_links(size_t page_index);

    /// Parses an explicit or named destination from a PDF value.
    [[nodiscard]] PdfDestination parse_destination(const PdfValue& dest_val) const;

    /// Parses an action dictionary from a PDF value.
    [[nodiscard]] PdfAction parse_action(const PdfValue& action_val) const;

    /// Resolves a named destination to an explicit destination.
    [[nodiscard]] std::optional<PdfDestination> resolve_named_destination(std::string_view name) const;

    struct CompressedObjEntry {
        uint32_t objstm_id{0};
        uint32_t index_in_stream{0};
    };

    static std::vector<uint8_t> apply_predictor(
        std::span<const uint8_t> in,
        int predictor,
        int columns,
        int colors,
        int bits_per_component
    );

private:
    bool parse_trailer_and_xref();
    size_t parse_classic_xref_table(size_t xref_offset);
    size_t parse_xref_stream(size_t xref_stream_offset);
    std::optional<PdfIndirectObject> resolve_compressed_object(uint32_t obj_id, const CompressedObjEntry& entry);
    bool parse_pages();
    void collect_pages(const PdfValue& node_val, const PdfDict& inherited_res = {}, PdfRef page_ref = {});
    std::vector<PdfOutlineItem> parse_outline_level(PdfRef first_ref, std::set<uint32_t>& visited, int depth);
    void fallback_scan_objects();

    // Lexer helpers
    void skip_whitespace_and_comments();
    std::string read_token();
    PdfValue parse_value();
    PdfDict parse_dict();
    PdfArray parse_array();
    std::string parse_literal_string();
    std::string parse_hex_string();
    std::string parse_name();

    std::span<const uint8_t> data_{};
    size_t cursor_{0};

    std::map<uint32_t, size_t> xref_offsets_{}; // obj_id -> byte offset
    std::map<uint32_t, CompressedObjEntry> compressed_objects_{}; // obj_id -> (objstm_id, index)
    std::map<uint32_t, std::map<uint32_t, size_t>> objstm_offsets_cache_{}; // objstm_id -> (obj_id -> offset)
    PdfDict trailer_{};
    PdfRef root_ref_{};
    std::optional<PdfRef> info_ref_{std::nullopt};
    std::vector<ParsedPdfPage> pages_{};
    std::map<PdfRef, int> page_ref_to_index_{};
    std::shared_ptr<PdfSecurityHandler> security_handler_{nullptr};
};

} // namespace nisaba::pdf
