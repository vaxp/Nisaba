#pragma once

#include "nisaba/gpu/types.hpp"
#include "nisaba/gpu/gpu_types.hpp"
#include <span>
#include <vector>

namespace nisaba::gpu {

// -------------------------------------------------------------
// Vertex (GPU vertex buffer layout)
// -------------------------------------------------------------
struct Vertex {
	float x{0.0f};
	float y{0.0f};
	float u{0.0f};
	float v{0.0f};
	uint32_t color{0xffffffff};

	constexpr Vertex() = default;
	constexpr Vertex(float x_, float y_, float u_, float v_, uint32_t color_ = 0xffffffff)
		: x(x_), y(y_), u(u_), v(v_), color(color_) {}
};

// -------------------------------------------------------------
// RenderPath: Segment slice in pre-allocated contour buffer passed from Tessellator to Renderer
// -------------------------------------------------------------
struct RenderPath {
	int first{0};
	int count{0};
	bool closed{false};
	int nbevel{0};
	const Vertex* fill{nullptr};
	int fillCount{0};
	const Vertex* stroke{nullptr};
	int strokeCount{0};
	Winding winding{Winding::CounterClockwise};
	bool convex{false};
};

using Path = RenderPath;

enum class TextureType {
	Alpha,
	RGBA
};

// -------------------------------------------------------------
// Abstract GPU Renderer Interface
// -------------------------------------------------------------
class Renderer {
public:
	virtual ~Renderer() = default;

	virtual bool init() = 0;
	virtual void shutdown() = 0;

	virtual bool edgeAntiAlias() const = 0;

	virtual int createTexture(TextureType type, int width, int height, int imageFlags, const unsigned char* data) = 0;
	virtual bool deleteTexture(int image) = 0;
	virtual bool updateTexture(int image, int x, int y, int width, int height, const unsigned char* data) = 0;
	virtual bool getTextureSize(int image, int& outWidth, int& outHeight) = 0;
	virtual int createTextureFromNativeHandle(uint64_t handle, int w, int h, int imageFlags) {
		(void)handle; (void)w; (void)h; (void)imageFlags;
		return 0;
	}
	virtual GpuBackendType backendType() const {
		return GpuBackendType::OpenGL;
	}

	virtual void viewport(float width, float height, float devicePixelRatio) = 0;
	virtual void cancel() = 0;
	virtual void flush() = 0;

	virtual void renderFill(const Paint& paint, const CompositeOperationState& compOp,
	                       const Scissor& scissor, float fringe, const float* bounds,
	                       const RenderPath* paths, int npaths) = 0;

	virtual void renderStroke(const Paint& paint, const CompositeOperationState& compOp,
	                         const Scissor& scissor, float fringe, float strokeWidth,
	                         const RenderPath* paths, int npaths) = 0;

	virtual void renderTriangles(const Paint& paint, const CompositeOperationState& compOp,
	                            const Scissor& scissor, const Vertex* verts, int nverts,
	                            float fringe, int shaderType = 3) = 0;
};

} // namespace nisaba::gpu
