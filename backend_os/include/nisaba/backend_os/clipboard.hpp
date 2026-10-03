#pragma once
/// @file clipboard.hpp
/// @brief Native Clipboard abstractions and multi-MIME data container for Linux (Wayland & X11).

#include "nisaba/backend_os/types.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <cstdint>

namespace nisaba::backend_os {

/// Target clipboard selection buffer.
enum class ClipboardType {
    Clipboard,  ///< Standard clipboard (Ctrl+C / Ctrl+V).
    Primary,    ///< Mouse selection / middle-click buffer (X11 & Wayland primary selection).
};

/// Common MIME types used across desktop environments.
namespace mime {
    constexpr std::string_view TextPlainUtf8 = "text/plain;charset=utf-8";
    constexpr std::string_view TextPlain     = "text/plain";
    constexpr std::string_view TextUtf8      = "UTF8_STRING";
    constexpr std::string_view TextString    = "STRING";
    constexpr std::string_view TextUriList   = "text/uri-list";
    constexpr std::string_view ImagePng      = "image/png";
}

/// Container holding multi-format clipboard or drag data.
class ClipboardData {
public:
    ClipboardData() = default;

    /// Create clipboard data with plain text.
    explicit ClipboardData(std::string_view text) {
        setText(text);
    }

    /// Create clipboard data with URI list.
    explicit ClipboardData(const std::vector<std::string>& uris) {
        setUris(uris);
    }

    /// Check if the container has data for a specific MIME format.
    [[nodiscard]] bool hasFormat(std::string_view mime_type) const {
        return entries_.find(std::string(mime_type)) != entries_.end();
    }

    /// Get list of all offered MIME formats.
    [[nodiscard]] std::vector<std::string> formats() const {
        std::vector<std::string> result;
        result.reserve(entries_.size());
        for (const auto& [mime, _] : entries_) {
            result.push_back(mime);
        }
        return result;
    }

    /// Set plain text (populates text/plain;charset=utf-8, text/plain, and UTF8_STRING).
    void setText(std::string_view text) {
        std::vector<uint8_t> bytes(text.begin(), text.end());
        entries_[std::string(mime::TextPlainUtf8)] = bytes;
        entries_[std::string(mime::TextPlain)]     = bytes;
        entries_[std::string(mime::TextUtf8)]      = bytes;
    }

    /// Get plain text string if available.
    [[nodiscard]] std::string getText() const {
        if (auto it = entries_.find(std::string(mime::TextPlainUtf8)); it != entries_.end()) {
            return std::string(it->second.begin(), it->second.end());
        }
        if (auto it = entries_.find(std::string(mime::TextPlain)); it != entries_.end()) {
            return std::string(it->second.begin(), it->second.end());
        }
        if (auto it = entries_.find(std::string(mime::TextUtf8)); it != entries_.end()) {
            return std::string(it->second.begin(), it->second.end());
        }
        return {};
    }

    /// Check if container has text data.
    [[nodiscard]] bool hasText() const {
        return hasFormat(mime::TextPlainUtf8) || hasFormat(mime::TextPlain) || hasFormat(mime::TextUtf8);
    }

    /// Set URI list (text/uri-list).
    void setUris(const std::vector<std::string>& uris) {
        std::string payload;
        for (const auto& uri : uris) {
            if (!payload.empty()) payload += "\r\n";
            payload += uri;
        }
        setRaw(mime::TextUriList, payload);
    }

    /// Get URI list from text/uri-list.
    [[nodiscard]] std::vector<std::string> getUris() const {
        std::vector<std::string> uris;
        auto raw = getRaw(mime::TextUriList);
        if (raw.empty()) return uris;

        std::string content(raw.begin(), raw.end());
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            // Trim carriage return if present
            if (!line.empty() && line.back() == '\r') line.pop_back();
            // Skip comments per RFC 2483
            if (!line.empty() && line.front() != '#') {
                uris.push_back(line);
            }
        }
        return uris;
    }

    /// Check if container has URI list data.
    [[nodiscard]] bool hasUris() const {
        return hasFormat(mime::TextUriList);
    }

    /// Set raw payload for custom MIME type.
    void setRaw(std::string_view mime_type, const std::vector<uint8_t>& bytes) {
        entries_[std::string(mime_type)] = bytes;
    }

    /// Set raw string payload for custom MIME type.
    void setRaw(std::string_view mime_type, std::string_view payload) {
        entries_[std::string(mime_type)] = std::vector<uint8_t>(payload.begin(), payload.end());
    }

    /// Get raw bytes for a MIME type.
    [[nodiscard]] std::vector<uint8_t> getRaw(std::string_view mime_type) const {
        auto it = entries_.find(std::string(mime_type));
        if (it != entries_.end()) {
            return it->second;
        }
        return {};
    }

    /// Check if container has no data.
    [[nodiscard]] bool empty() const {
        return entries_.empty();
    }

    /// Clear all data.
    void clear() {
        entries_.clear();
    }

private:
    std::unordered_map<std::string, std::vector<uint8_t>> entries_;
};

} // namespace nisaba::backend_os
