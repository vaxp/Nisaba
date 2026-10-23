#pragma once

/// @file orionjs.hpp
/// @brief OrionJS - High-performance sovereign data parser and DOM representation in C++20.

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include <memory>
#include <cstdint>

namespace orionjs {

enum class Type : uint8_t {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

class Value;
using Array = std::vector<Value>;
using Object = std::map<std::string, Value, std::less<>>;

class Value {
public:
    using Data = std::variant<
        std::monostate, // Null
        bool,           // Boolean
        double,         // Number
        std::string,    // String
        Array,          // Array
        Object          // Object
    >;

    Value() noexcept : data_(std::monostate{}) {}
    Value(std::nullptr_t) noexcept : data_(std::monostate{}) {}
    Value(bool b) noexcept : data_(b) {}
    Value(double n) noexcept : data_(n) {}
    Value(float n) noexcept : data_(static_cast<double>(n)) {}
    Value(int n) noexcept : data_(static_cast<double>(n)) {}
    Value(int64_t n) noexcept : data_(static_cast<double>(n)) {}
    Value(uint64_t n) noexcept : data_(static_cast<double>(n)) {}
    Value(std::string s) noexcept : data_(std::move(s)) {}
    Value(std::string_view s) : data_(std::string(s)) {}
    Value(const char* s) : data_(std::string(s ? s : "")) {}
    Value(Array arr) noexcept : data_(std::move(arr)) {}
    Value(Object obj) noexcept : data_(std::move(obj)) {}

    [[nodiscard]] Type type() const noexcept {
        switch (data_.index()) {
            case 0: return Type::Null;
            case 1: return Type::Boolean;
            case 2: return Type::Number;
            case 3: return Type::String;
            case 4: return Type::Array;
            case 5: return Type::Object;
            default: return Type::Null;
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

    [[nodiscard]] const Array& as_array() const noexcept {
        static const Array s_empty_arr;
        if (auto* arr = std::get_if<Array>(&data_)) return *arr;
        return s_empty_arr;
    }

    [[nodiscard]] Array& as_array_mut() {
        if (auto* arr = std::get_if<Array>(&data_)) return *arr;
        data_ = Array{};
        return std::get<Array>(data_);
    }

    [[nodiscard]] const Object& as_object() const noexcept {
        static const Object s_empty_obj;
        if (auto* obj = std::get_if<Object>(&data_)) return *obj;
        return s_empty_obj;
    }

    [[nodiscard]] Object& as_object_mut() {
        if (auto* obj = std::get_if<Object>(&data_)) return *obj;
        data_ = Object{};
        return std::get<Object>(data_);
    }

    // Object lookup helpers
    [[nodiscard]] bool has(std::string_view key) const noexcept {
        if (auto* obj = std::get_if<Object>(&data_)) {
            return obj->find(key) != obj->end();
        }
        return false;
    }

    [[nodiscard]] const Value* get(std::string_view key) const noexcept {
        if (auto* obj = std::get_if<Object>(&data_)) {
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

    [[nodiscard]] const Array& get_array(std::string_view key) const noexcept {
        static const Array s_empty_arr;
        if (const auto* val = get(key)) return val->as_array();
        return s_empty_arr;
    }

    [[nodiscard]] const Object& get_object(std::string_view key) const noexcept {
        static const Object s_empty_obj;
        if (const auto* val = get(key)) return val->as_object();
        return s_empty_obj;
    }

    // Parsing API
    [[nodiscard]] static std::optional<Value> parse(std::string_view input);
    [[nodiscard]] static std::optional<Value> parse_file(const std::string& filepath);

private:
    Data data_;
};

} // namespace orionjs
