#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include <span>

namespace nisaba::pdf {

/// PDF Name object (e.g. /Type, /Page, /Resources)
struct PdfName {
    std::string name;

    PdfName() = default;
    explicit PdfName(std::string_view n) : name(n) {}
    bool operator==(const PdfName& o) const noexcept { return name == o.name; }
    bool operator<(const PdfName& o) const noexcept { return name < o.name; }
};

/// PDF Indirect Object Reference (e.g. 5 0 R)
struct PdfRef {
    uint32_t id{0};
    uint16_t gen{0};

    constexpr PdfRef() noexcept = default;
    constexpr PdfRef(uint32_t obj_id, uint16_t generation = 0) noexcept
        : id(obj_id), gen(generation) {}

    constexpr bool operator==(const PdfRef& o) const noexcept {
        return id == o.id && gen == o.gen;
    }
    constexpr bool operator<(const PdfRef& o) const noexcept {
        if (id != o.id) return id < o.id;
        return gen < o.gen;
    }
};

struct PdfValue;
using PdfArray = std::vector<PdfValue>;
using PdfDict = std::map<std::string, PdfValue>;

/// Represents any basic PDF object value according to ISO 32000-1 (PDF specification).
struct PdfValue {
    using Storage = std::variant<
        std::monostate, // null
        bool,           // boolean
        int64_t,        // integer
        double,         // real
        std::string,    // literal / hex string
        PdfName,        // name
        PdfRef,         // indirect reference
        PdfArray,       // array
        PdfDict         // dictionary
    >;

    Storage data;

    PdfValue() : data(std::monostate{}) {}
    PdfValue(bool b) : data(b) {}
    PdfValue(int i) : data(static_cast<int64_t>(i)) {}
    PdfValue(int64_t i) : data(i) {}
    PdfValue(size_t i) : data(static_cast<int64_t>(i)) {}
    PdfValue(float f) : data(static_cast<double>(f)) {}
    PdfValue(double d) : data(d) {}
    PdfValue(std::string_view s) : data(std::string(s)) {}
    PdfValue(const char* s) : data(std::string(s)) {}
    PdfValue(std::string s) : data(std::move(s)) {}
    PdfValue(PdfName name) : data(std::move(name)) {}
    PdfValue(PdfRef ref) : data(ref) {}
    PdfValue(PdfArray arr) : data(std::move(arr)) {}
    PdfValue(PdfDict dict) : data(std::move(dict)) {}

    [[nodiscard]] bool is_null() const noexcept { return std::holds_alternative<std::monostate>(data); }
    [[nodiscard]] bool is_bool() const noexcept { return std::holds_alternative<bool>(data); }
    [[nodiscard]] bool is_int() const noexcept { return std::holds_alternative<int64_t>(data); }
    [[nodiscard]] bool is_double() const noexcept { return std::holds_alternative<double>(data); }
    [[nodiscard]] bool is_number() const noexcept { return is_int() || is_double(); }
    [[nodiscard]] bool is_string() const noexcept { return std::holds_alternative<std::string>(data); }
    [[nodiscard]] bool is_name() const noexcept { return std::holds_alternative<PdfName>(data); }
    [[nodiscard]] bool is_ref() const noexcept { return std::holds_alternative<PdfRef>(data); }
    [[nodiscard]] bool is_array() const noexcept { return std::holds_alternative<PdfArray>(data); }
    [[nodiscard]] bool is_dict() const noexcept { return std::holds_alternative<PdfDict>(data); }

    [[nodiscard]] bool as_bool(bool def = false) const noexcept {
        return is_bool() ? std::get<bool>(data) : def;
    }
    [[nodiscard]] int64_t as_int(int64_t def = 0) const noexcept {
        if (is_int()) return std::get<int64_t>(data);
        if (is_double()) return static_cast<int64_t>(std::get<double>(data));
        return def;
    }
    [[nodiscard]] double as_double(double def = 0.0) const noexcept {
        if (is_double()) return std::get<double>(data);
        if (is_int()) return static_cast<double>(std::get<int64_t>(data));
        return def;
    }
    [[nodiscard]] float as_float(float def = 0.0f) const noexcept {
        return static_cast<float>(as_double(static_cast<double>(def)));
    }
    [[nodiscard]] const std::string& as_string() const {
        return std::get<std::string>(data);
    }
    [[nodiscard]] const std::string& as_name() const {
        return std::get<PdfName>(data).name;
    }
    [[nodiscard]] PdfRef as_ref() const {
        return std::get<PdfRef>(data);
    }
    [[nodiscard]] const PdfArray& as_array() const {
        return std::get<PdfArray>(data);
    }
    [[nodiscard]] PdfArray& as_array_mut() {
        return std::get<PdfArray>(data);
    }
    [[nodiscard]] const PdfDict& as_dict() const {
        return std::get<PdfDict>(data);
    }
    [[nodiscard]] PdfDict& as_dict_mut() {
        return std::get<PdfDict>(data);
    }

    /// Fast lookup inside a dictionary (returns nullptr if not found or not a dict)
    [[nodiscard]] const PdfValue* find(std::string_view key) const noexcept;
};

/// Represents an indirect PDF object definition (id gen obj ... endobj)
struct PdfIndirectObject {
    uint32_t id{0};
    uint16_t gen{0};
    PdfValue value{};
    std::optional<std::vector<uint8_t>> stream_data{std::nullopt};
};

/// Serializes any PdfValue to valid PDF syntax string.
void serialize_pdf_value(const PdfValue& value, std::string& out);

/// Escapes a literal string according to PDF specification (\(, \), \\).
std::string escape_pdf_string(std::string_view str);

} // namespace nisaba::pdf
