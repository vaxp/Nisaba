#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include "nisaba/math/rect.hpp"
#include "nisaba/color/color.hpp"

namespace nisaba::pdf {

/// Destination type as specified in ISO 32000-1 §12.3.2.
enum class DestinationType {
    XYZ,    ///< [page /XYZ left top zoom]
    Fit,    ///< [page /Fit]
    FitH,   ///< [page /FitH top]
    FitV,   ///< [page /FitV left]
    FitR,   ///< [page /FitR left bottom right top]
    FitB,   ///< [page /FitB]
    FitBH,  ///< [page /FitBH top]
    FitBV,  ///< [page /FitBV left]
    Named   ///< Named destination string
};

/// Represents an explicit or named destination in a PDF document.
struct PdfDestination {
    DestinationType type{DestinationType::XYZ};
    int page_index{-1}; // 0-based page index (-1 if unresolved)
    std::string named_dest{}; // If named destination string/name

    std::optional<float> left{std::nullopt};
    std::optional<float> top{std::nullopt};
    std::optional<float> right{std::nullopt};
    std::optional<float> bottom{std::nullopt};
    std::optional<float> zoom{std::nullopt};

    [[nodiscard]] bool is_valid() const noexcept {
        return page_index >= 0 || !named_dest.empty();
    }
};

/// Action type as specified in ISO 32000-1 §12.6.
enum class PdfActionType {
    None,
    GoTo,     ///< Go to destination in the current document (§12.6.4.2)
    GoToR,    ///< Go to destination in another document (§12.6.4.3)
    Launch,   ///< Launch an application or file (§12.6.4.5)
    URI,      ///< Open web URL (§12.6.4.7)
    Named,    ///< Predefined named action (§12.6.4.11): NextPage, PrevPage, FirstPage, LastPage
    Other
};

/// Represents an interactive PDF Action.
struct PdfAction {
    PdfActionType type{PdfActionType::None};
    PdfDestination destination{};
    std::string uri{};
    std::string named_action{}; // e.g. "NextPage", "PrevPage", "FirstPage", "LastPage"
    std::string file_path{};    // For GoToR or Launch

    [[nodiscard]] bool is_valid() const noexcept {
        return type != PdfActionType::None;
    }
};

/// Represents an item in the Document Outline / Bookmarks hierarchy (ISO 32000-1 §12.3.3).
struct PdfOutlineItem {
    std::string title{};
    PdfAction action{};
    PdfDestination destination{};
    Color color{Color::BLACK};
    bool bold{false};
    bool italic{false};
    std::vector<PdfOutlineItem> children{};

    [[nodiscard]] int target_page() const noexcept {
        if (action.destination.page_index >= 0) return action.destination.page_index;
        return destination.page_index;
    }
};

/// Represents a clickable Link Annotation on a page (ISO 32000-1 §12.5.6.5).
struct PdfLinkAnnotation {
    Rect rect{};                  ///< Hotspot bounding box in page coordinates (points)
    PdfAction action{};           ///< Associated action (URI, GoTo, etc.)
    PdfDestination destination{}; ///< Associated destination if direct /Dest
    std::string uri{};            ///< Shortcut if action is URI
    int target_page{-1};          ///< Target page index (0-based) if internal link
};

/// Decodes PDF text string (UTF-16BE BOM, UTF-8 BOM, or PDFDocEncoding) to clean UTF-8 string.
std::string decode_pdf_doc_string(std::string_view raw);

} // namespace nisaba::pdf
