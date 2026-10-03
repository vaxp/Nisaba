#pragma once

/// @file json.hpp
/// @brief Sovereign, zero-dependency RFC 8259 JSON parser and DOM representation in C++20.
/// Part of the Nisaba Graphics & Animation Engine.

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include <memory>
#include <cstdint>

namespace nisaba::json {

enum class JsonType : uint8_t {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

class JsonValue;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue, std::less<>>;

class JsonValue {
public:
    using ValueData = std::variant<
        std::monostate, // Null
        bool,           // Boolean
        double,         // Number
        std::string,    // String
        JsonArray,      // Array
        JsonObject      // Object
    >;

    JsonValue() noexcept : data_(std::monostate{}) {}
    JsonValue(std::nullptr_t) noexcept : data_(std::monostate{}) {}
    JsonValue(bool b) noexcept : data_(b) {}
    JsonValue(double n) noexcept : data_(n) {}
    JsonValue(float n) noexcept : data_(static_cast<double>(n)) {}
    JsonValue(int n) noexcept : data_(static_cast<double>(n)) {}
    JsonValue(int64_t n) noexcept : data_(static_cast<double>(n)) {}
    JsonValue(uint64_t n) noexcept : data_(static_cast<double>(n)) {}
    JsonValue(std::string s) noexcept : data_(std::move(s)) {}
    JsonValue(std::string_view s) : data_(std::string(s)) {}
    JsonValue(const char* s) : data_(std::string(s ? s : "")) {}
    JsonValue(JsonArray arr) noexcept : data_(std::move(arr)) {}
    JsonValue(JsonObject obj) noexcept : data_(std::move(obj)) {}

    [[nodiscard]] JsonType type() const noexcept {
        switch (data_.index()) {
            case 0: return JsonType::Null;
            case 1: return JsonType::Boolean;
            case 2: return JsonType::Number;
            case 3: return JsonType::String;
            case 4: return JsonType::Array;
            case 5: return JsonType::Object;
            default: return JsonType::Null;
        }
    }

    [[nodiscard]] bool is_null() const noexcept { return data_.index() == 0; }
    [[nodiscard]] bool is_bool() const noexcept { return data_.index() == 1; }
    [[nodiscard]] bool is_number() const noexcept { return data_.index() == 2; }
    [[nodiscard]] bool is_string() const noexcept { return data_.index() == 3; }
    [[nodiscard]] bool is_array() const noexcept { return data_.index() == 4; }
    [[nodiscard]] bool is_object() const noexcept { return data_.index() == 5; }

    [[nodiscard]] bool to_bool(bool default_val = false) const noexcept {
        if (auto* b = std::get_if<bool>(&data_)) return *b;
        if (auto* n = std::get_if<double>(&data_)) return *n != 0.0;
        return default_val;
    }

    [[nodiscard]] bool as_bool(bool default_val = false) const noexcept {
        return to_bool(default_val);
    }

    [[nodiscard]] double to_double(double default_val = 0.0) const noexcept {
        if (auto* n = std::get_if<double>(&data_)) return *n;
        if (auto* b = std::get_if<bool>(&data_)) return *b ? 1.0 : 0.0;
        return default_val;
    }

    [[nodiscard]] float to_float(float default_val = 0.0f) const noexcept {
        return static_cast<float>(to_double(static_cast<double>(default_val)));
    }

    [[nodiscard]] int to_int(int default_val = 0) const noexcept {
        return static_cast<int>(to_double(static_cast<double>(default_val)));
    }

    [[nodiscard]] std::string_view to_string(std::string_view default_val = "") const noexcept {
        if (auto* s = std::get_if<std::string>(&data_)) return *s;
        return default_val;
    }

    [[nodiscard]] std::string_view as_string() const noexcept {
        return to_string();
    }

    [[nodiscard]] const JsonArray& as_array() const noexcept {
        static const JsonArray s_empty_arr;
        if (auto* arr = std::get_if<JsonArray>(&data_)) return *arr;
        return s_empty_arr;
    }

    [[nodiscard]] JsonArray& as_array_mut() {
        if (auto* arr = std::get_if<JsonArray>(&data_)) return *arr;
        data_ = JsonArray{};
        return std::get<JsonArray>(data_);
    }

    [[nodiscard]] const JsonObject& as_object() const noexcept {
        static const JsonObject s_empty_obj;
        if (auto* obj = std::get_if<JsonObject>(&data_)) return *obj;
        return s_empty_obj;
    }

    [[nodiscard]] JsonObject& as_object_mut() {
        if (auto* obj = std::get_if<JsonObject>(&data_)) return *obj;
        data_ = JsonObject{};
        return std::get<JsonObject>(data_);
    }

    // Object lookup helpers
    [[nodiscard]] bool has(std::string_view key) const noexcept {
        if (auto* obj = std::get_if<JsonObject>(&data_)) {
            return obj->find(key) != obj->end();
        }
        return false;
    }

    [[nodiscard]] const JsonValue* get(std::string_view key) const noexcept {
        if (auto* obj = std::get_if<JsonObject>(&data_)) {
            auto it = obj->find(key);
            if (it != obj->end()) return &(it->second);
        }
        return nullptr;
    }

    [[nodiscard]] double get_double(std::string_view key, double default_val = 0.0) const noexcept {
        if (const auto* val = get(key)) return val->to_double(default_val);
        return default_val;
    }

    [[nodiscard]] float get_float(std::string_view key, float default_val = 0.0f) const noexcept {
        if (const auto* val = get(key)) return val->to_float(default_val);
        return default_val;
    }

    [[nodiscard]] int get_int(std::string_view key, int default_val = 0) const noexcept {
        if (const auto* val = get(key)) return val->to_int(default_val);
        return default_val;
    }

    [[nodiscard]] bool get_bool(std::string_view key, bool default_val = false) const noexcept {
        if (const auto* val = get(key)) return val->to_bool(default_val);
        return default_val;
    }

    [[nodiscard]] std::string_view get_string(std::string_view key, std::string_view default_val = "") const noexcept {
        if (const auto* val = get(key)) return val->to_string(default_val);
        return default_val;
    }

    [[nodiscard]] const JsonArray& get_array(std::string_view key) const noexcept {
        static const JsonArray s_empty_arr;
        if (const auto* val = get(key)) return val->as_array();
        return s_empty_arr;
    }

    [[nodiscard]] const JsonObject& get_object(std::string_view key) const noexcept {
        static const JsonObject s_empty_obj;
        if (const auto* val = get(key)) return val->as_object();
        return s_empty_obj;
    }

    // Parsing API
    [[nodiscard]] static std::optional<JsonValue> parse(std::string_view json_str);
    [[nodiscard]] static std::optional<JsonValue> parse_file(const std::string& filepath);

private:
    ValueData data_;
};

} // namespace nisaba::json
