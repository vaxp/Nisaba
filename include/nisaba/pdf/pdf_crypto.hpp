#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <string>
#include <string_view>
#include <array>
#include <memory>
#include "nisaba/pdf/pdf_types.hpp"

namespace nisaba::pdf {

/// Computes standard MD5 128-bit hash (RFC 1321).
std::array<uint8_t, 16> md5_hash(std::span<const uint8_t> data);

/// Computes standard SHA-256 256-bit hash (FIPS 180-4).
std::array<uint8_t, 32> sha256_hash(std::span<const uint8_t> data);

/// Encrypts or decrypts bytes using RC4 stream cipher (ISO 32000-1 §7.6.2).
std::vector<uint8_t> rc4_crypt(
    std::span<const uint8_t> key,
    std::span<const uint8_t> input
);

/// Decrypts bytes using AES in CBC mode (128-bit or 256-bit) with 16-byte IV.
std::vector<uint8_t> aes_decrypt_cbc(
    std::span<const uint8_t> key,
    std::span<const uint8_t> input,
    std::span<const uint8_t, 16> iv
);

/// Standard Security Handler implementing ISO 32000-1 §7.6.3 and ISO 32000-2 encryption.
class PdfSecurityHandler {
public:
    PdfSecurityHandler(const PdfDict& encrypt_dict, const PdfArray& id_array);
    ~PdfSecurityHandler() = default;

    /// Returns true if document is encrypted.
    bool is_encrypted() const noexcept { return is_encrypted_; }

    /// Returns true if password has been authenticated.
    bool is_authenticated() const noexcept { return is_authenticated_; }

    /// Attempts to authenticate with user or owner password.
    bool authenticate(const std::string& password);

    /// Decrypts an indirect stream object.
    std::vector<uint8_t> decrypt_stream(
        uint32_t obj_id,
        uint32_t gen,
        std::span<const uint8_t> data
    ) const;

    /// Decrypts a literal or hex string object.
    std::string decrypt_string(
        uint32_t obj_id,
        uint32_t gen,
        std::string_view str
    ) const;

private:
    std::vector<uint8_t> compute_object_key(uint32_t obj_id, uint32_t gen, bool is_aes) const;

    bool is_encrypted_{false};
    bool is_authenticated_{false};
    int version_{1};
    int revision_{2};
    size_t key_len_bytes_{5}; // 40-bit default for V1
    int64_t permissions_{-1};
    bool encrypt_metadata_{true};

    std::string o_string_{};
    std::string u_string_{};
    std::string id_first_{};

    bool is_aes_{false};
    bool is_aes256_{false};
    std::vector<uint8_t> encryption_key_{};
};

} // namespace nisaba::pdf
