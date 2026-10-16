#pragma once

#include "nisaba/gpu/shader.hpp"
#include "nisaba/gpu/types.hpp"
#include "nisaba/math/transform.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <type_traits>
#include <cstring>

namespace nisaba::gpu {

class RuntimeEffect : public std::enable_shared_from_this<RuntimeEffect> {
public:
    struct Uniform {
        enum class Type {
            Float,
            Float2,
            Float3,
            Float4,
            Mat3,
            Mat4,
            Int
        };

        std::string name;
        size_t offset{0};
        size_t sizeInBytes{0};
        Type type{Type::Float};
        int location{-1};
    };

    struct Result {
        std::shared_ptr<RuntimeEffect> effect{nullptr};
        std::string errorText{};
        explicit operator bool() const noexcept { return effect != nullptr; }
    };

    static Result MakeForShader(std::string_view sksl);

    ~RuntimeEffect();

    [[nodiscard]] const Uniform* findUniform(const char* name) const noexcept;
    [[nodiscard]] const std::vector<Uniform>& uniforms() const noexcept { return uniforms_; }
    [[nodiscard]] size_t uniformSize() const noexcept { return uniform_size_; }
    [[nodiscard]] uint32_t programId() const noexcept { return program_; }
    [[nodiscard]] int localMatrixLoc() const noexcept { return loc_local_mat_; }
    [[nodiscard]] int viewSizeLoc() const noexcept { return loc_view_size_; }

private:
    RuntimeEffect() = default;
    uint32_t program_{0};
    uint32_t vert_shader_{0};
    uint32_t frag_shader_{0};
    std::vector<Uniform> uniforms_{};
    size_t uniform_size_{0};
    int loc_local_mat_{-1};
    int loc_view_size_{-1};

    friend class RuntimeEffectParser;
};

class RuntimeShader : public Shader {
public:
    RuntimeShader(std::shared_ptr<RuntimeEffect> effect,
                  std::vector<uint8_t> uniformData,
                  Transform2D localMatrix = Transform2D::identity());

    void* getNativeHandle() const override { return const_cast<RuntimeShader*>(this); }
    [[nodiscard]] bool isOpaque() const override { return false; }

    [[nodiscard]] const std::shared_ptr<RuntimeEffect>& effect() const noexcept { return effect_; }
    [[nodiscard]] const std::vector<uint8_t>& uniformData() const noexcept { return uniform_data_; }
    [[nodiscard]] const Transform2D& localMatrix() const noexcept { return local_matrix_; }

    void bind(const float* viewSize) const;

private:
    std::shared_ptr<RuntimeEffect> effect_;
    std::vector<uint8_t> uniform_data_;
    Transform2D local_matrix_;
    float inv_m3_[9]{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
};

class RuntimeShaderBuilder {
public:
    struct BuilderUniform {
        RuntimeShaderBuilder* owner{nullptr};
        const RuntimeEffect::Uniform* var{nullptr};

        template <typename T>
        BuilderUniform& operator=(const T& val) {
            static_assert(std::is_trivially_copyable_v<T>, "Value must be trivially copyable");
            if (var && owner) {
                owner->writeUniformData(var->offset, &val, sizeof(T));
            }
            return *this;
        }

        template <typename T>
        bool set(const T val[], int count) {
            static_assert(std::is_trivially_copyable_v<T>, "Value must be trivially copyable");
            if (var && owner && sizeof(T) * count <= var->sizeInBytes) {
                owner->writeUniformData(var->offset, val, sizeof(T) * count);
                return true;
            }
            return false;
        }
    };

    explicit RuntimeShaderBuilder(std::shared_ptr<RuntimeEffect> effect);

    BuilderUniform uniform(const char* name) {
        return BuilderUniform{this, effect_ ? effect_->findUniform(name) : nullptr};
    }

    std::shared_ptr<Shader> makeShader(const Transform2D* localMatrix = nullptr);
    std::shared_ptr<Shader> makeShader(const nisaba::Transform* localMatrix);

    void writeUniformData(size_t offset, const void* data, size_t size);

private:
    std::shared_ptr<RuntimeEffect> effect_;
    std::vector<uint8_t> uniform_data_;
};

// Aliases for framework drop-in compatibility
using SkRuntimeEffect = RuntimeEffect;
using SkRuntimeShaderBuilder = RuntimeShaderBuilder;

} // namespace nisaba::gpu
