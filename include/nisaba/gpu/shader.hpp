#pragma once

#include <memory>

namespace nisaba::gpu {

/**
 * @brief Base polymorphic Shader interface for GPU pipelines.
 */
class Shader {
public:
    virtual ~Shader() = default;
    virtual void* getNativeHandle() const = 0;
    virtual bool isOpaque() const { return false; }
};

/// 1:1 drop-in compatibility alias for ENKI framework
using SkShader = Shader;

} // namespace nisaba::gpu
