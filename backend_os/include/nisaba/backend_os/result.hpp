#pragma once
/// @file result.hpp
/// @brief Result<T, E> error handling for nisaba::backend_os.

#include <variant>
#include <string>
#include <string_view>
#include <functional>
#include <stdexcept>
#include <utility>
#include <cassert>

namespace nisaba::backend_os {

/// Error codes used throughout backend_os.
enum class ErrorCode {
    None = 0,
    OutOfMemory,
    InvalidArgument,
    NotFound,
    AlreadyExists,
    PlatformError,
    WindowError,
    RenderingError,
    IOError,
    NotInitialized,
    NotSupported,
};

/// Structured error with code and message.
struct Error {
    ErrorCode code = ErrorCode::None;
    std::string message;

    Error() = default;
    Error(ErrorCode c, std::string msg = "")
        : code(c), message(std::move(msg)) {}

    [[nodiscard]] bool isNone() const { return code == ErrorCode::None; }

    [[nodiscard]] std::string_view codeString() const {
        switch (code) {
            case ErrorCode::None:            return "None";
            case ErrorCode::OutOfMemory:     return "OutOfMemory";
            case ErrorCode::InvalidArgument: return "InvalidArgument";
            case ErrorCode::NotFound:        return "NotFound";
            case ErrorCode::AlreadyExists:   return "AlreadyExists";
            case ErrorCode::PlatformError:   return "PlatformError";
            case ErrorCode::WindowError:     return "WindowError";
            case ErrorCode::RenderingError:  return "RenderingError";
            case ErrorCode::IOError:         return "IOError";
            case ErrorCode::NotInitialized:  return "NotInitialized";
            case ErrorCode::NotSupported:    return "NotSupported";
        }
        return "Unknown";
    }

    bool operator==(const Error& o) const { return code == o.code; }
};

/// A result type that holds either a value of type T or an Error.
template<typename T>
class Result {
public:
    static Result ok(T value) {
        Result r;
        r.data_.template emplace<0>(std::move(value));
        return r;
    }

    static Result err(ErrorCode code, std::string message = "") {
        Result r;
        r.data_.template emplace<1>(Error{code, std::move(message)});
        return r;
    }

    static Result err(Error error) {
        Result r;
        r.data_.template emplace<1>(std::move(error));
        return r;
    }

    [[nodiscard]] bool isOk() const { return data_.index() == 0; }
    [[nodiscard]] bool isErr() const { return data_.index() == 1; }

    [[nodiscard]] T& value() & {
        assert(isOk() && "Attempted to access value of an error Result");
        return std::get<0>(data_);
    }

    [[nodiscard]] const T& value() const& {
        assert(isOk() && "Attempted to access value of an error Result");
        return std::get<0>(data_);
    }

    [[nodiscard]] T&& value() && {
        assert(isOk() && "Attempted to access value of an error Result");
        return std::get<0>(std::move(data_));
    }

    [[nodiscard]] T valueOr(T default_value) const {
        if (isOk()) return std::get<0>(data_);
        return default_value;
    }

    [[nodiscard]] const Error& error() const {
        assert(isErr() && "Attempted to access error of a successful Result");
        return std::get<1>(data_);
    }

    template<typename F>
    auto map(F&& f) const -> Result<std::invoke_result_t<F, const T&>> {
        using U = std::invoke_result_t<F, const T&>;
        if (isOk()) {
            return Result<U>::ok(f(std::get<0>(data_)));
        }
        return Result<U>::err(std::get<1>(data_));
    }

    template<typename F>
    auto andThen(F&& f) const -> std::invoke_result_t<F, const T&> {
        if (isOk()) {
            return f(std::get<0>(data_));
        }
        using U = std::invoke_result_t<F, const T&>;
        return U::err(std::get<1>(data_));
    }

    explicit operator bool() const { return isOk(); }

private:
    Result() = default;
    std::variant<T, Error> data_;
};

/// Specialization for void.
template<>
class Result<void> {
public:
    static Result ok() {
        Result r;
        r.is_ok_ = true;
        return r;
    }

    static Result err(ErrorCode code, std::string message = "") {
        Result r;
        r.is_ok_ = false;
        r.error_ = Error{code, std::move(message)};
        return r;
    }

    static Result err(Error error) {
        Result r;
        r.is_ok_ = false;
        r.error_ = std::move(error);
        return r;
    }

    [[nodiscard]] bool isOk() const { return is_ok_; }
    [[nodiscard]] bool isErr() const { return !is_ok_; }
    [[nodiscard]] const Error& error() const {
        assert(isErr());
        return error_;
    }

    explicit operator bool() const { return is_ok_; }

private:
    Result() = default;
    bool is_ok_ = false;
    Error error_;
};

#define NISABA_BACKEND_OS_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            assert(false && (msg)); \
        } \
    } while (0)

#define NISABA_BACKEND_OS_CHECK(cond, code, msg) \
    do { \
        if (!(cond)) { \
            return Result<void>::err(code, msg); \
        } \
    } while (0)

}  // namespace nisaba::backend_os
