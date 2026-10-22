// -----------------------------------------------------------------------------
// Nisaba Sovereign 2D Graphics Engine - OpenGL 3.2 Core Pipeline Renderer
//
// Features:
// - Direct hardware rasterization with subpixel coverage anti-aliasing
// - High-throughput batch coalescing (merging consecutive primitive states)
// - Std140 uniform buffer object (UBO) streaming architecture
// - Analytical signed-distance field (SDF) evaluation for rounded rectangles
// - Hardware scissor acceleration and subpixel feathering
// -----------------------------------------------------------------------------

#include "nisaba/gpu/gl3_renderer.hpp"
#include "nisaba/gpu/runtime_effect.hpp"

#include <GL/glew.h>
#include <GL/gl.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstddef>

namespace nisaba::gpu {

namespace {

enum UniformLoc {
	LocViewSize = 0,
	LocTex      = 1,
	LocParams   = 2,
	MaxLocs     = 3
};

enum ShaderType {
	ShaderFillGrad      = 0,
	ShaderFillImg       = 1,
	ShaderSimple        = 2,
	ShaderImg           = 3,
	ShaderSolid         = 4,
	ShaderLinearGrad    = 5,
	ShaderRadialGrad    = 6,
	ShaderCircle        = 7,
	ShaderRect          = 8,
	ShaderRRect         = 9,
	ShaderRing          = 10,
	ShaderRRectGradient = 11,
	MaxShaders          = 12
};

constexpr int ParamsBinding = 0;

struct GLShader {
	GLuint prog{0};
	GLuint frag{0};
	GLuint vert{0};
	GLint loc[MaxLocs]{0, 0, 0};
};

struct Texture {
	int id{0};
	GLuint tex{0};
	int width{0};
	int height{0};
	TextureType type{TextureType::RGBA};
	int flags{0};
};

struct Blend {
	GLenum srcRGB{GL_INVALID_ENUM};
	GLenum dstRGB{GL_INVALID_ENUM};
	GLenum srcAlpha{GL_INVALID_ENUM};
	GLenum dstAlpha{GL_INVALID_ENUM};
};

enum class CallType {
	None = 0,
	Fill,
	ConvexFill,
	Stroke,
	Triangles,
	Clip,
	RestoreClip
};

struct Call {
	CallType type{CallType::None};
	int image{0};
	int pathOffset{0};
	int pathCount{0};
	int triangleOffset{0};
	int triangleCount{0};
	int uniformOffset{0};
	Blend blendFunc;
	bool useStencil{false};
	int scissorRect[4]{-1, -1, -1, -1};
	int clipDepth{0};
	Winding winding{Winding::CounterClockwise};
	std::shared_ptr<nisaba::gpu::Shader> customShader{nullptr};
};

struct GLPath {
	int fillOffset{0};
	int fillCount{0};
	int strokeOffset{0};
	int strokeCount{0};
};

// std140 layout matching GLSL layout(std140) uniform NisabaShaderParams
struct NisabaShaderUniforms {
	float scissorMat[12]; // 3 x vec4
	float paintMat[12];   // 3 x vec4
	Color innerCol;       // 4 floats (premultiplied)
	Color outerCol;       // 4 floats (premultiplied)
	float scissorExt[2];
	float scissorScale[2];
	float extent[2];
	float radius;
	float feather;
	float strokeMult;
	float strokeThr;
	int texType;
	int type;
};

using FragUniforms = NisabaShaderUniforms;

static void dumpShaderError(GLuint shader, const char* name, const char* type) {
	GLchar str[512 + 1];
	GLsizei len = 0;
	glGetShaderInfoLog(shader, 512, &len, str);
	if (len > 512) len = 512;
	str[len] = '\0';
	std::fprintf(stderr, "Shader %s/%s error:\n%s\n", name, type, str);
}

static void dumpProgramError(GLuint prog, const char* name) {
	GLchar str[512 + 1];
	GLsizei len = 0;
	glGetProgramInfoLog(prog, 512, &len, str);
	if (len > 512) len = 512;
	str[len] = '\0';
	std::fprintf(stderr, "Program %s error:\n%s\n", name, str);
}

static void checkGLError(int flags, const char* str) {
	if ((flags & CreateFlags::Debug) == 0) return;
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::fprintf(stderr, "OpenGL Error 0x%08x after %s\n", err, str);
	}
}

static bool createShader(GLShader& shader, const char* name, const char* header, const char* opts,
                         const char* vshader, const char* fshader) {
	GLint status = 0;
	const char* str[3];
	str[0] = header;
	str[1] = (opts != nullptr) ? opts : "";

	shader = GLShader{};

	GLuint prog = glCreateProgram();
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

	str[2] = vshader;
	glShaderSource(vert, 3, str, nullptr);
	str[2] = fshader;
	glShaderSource(frag, 3, str, nullptr);

	glCompileShader(vert);
	glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		dumpShaderError(vert, name, "vert");
		glDeleteShader(vert);
		glDeleteShader(frag);
		glDeleteProgram(prog);
		return false;
	}

	glCompileShader(frag);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		dumpShaderError(frag, name, "frag");
		glDeleteShader(vert);
		glDeleteShader(frag);
		glDeleteProgram(prog);
		return false;
	}

	glAttachShader(prog, vert);
	glAttachShader(prog, frag);

	glBindAttribLocation(prog, 0, "vertex");
	glBindAttribLocation(prog, 1, "tcoord");

	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		dumpProgramError(prog, name);
		glDeleteShader(vert);
		glDeleteShader(frag);
		glDeleteProgram(prog);
		return false;
	}

	shader.prog = prog;
	shader.vert = vert;
	shader.frag = frag;
	return true;
}

static void deleteShader(GLShader& shader) {
	if (shader.prog != 0) glDeleteProgram(shader.prog);
	if (shader.vert != 0) glDeleteShader(shader.vert);
	if (shader.frag != 0) glDeleteShader(shader.frag);
	shader = GLShader{};
}

static void getUniforms(GLShader& shader) {
	shader.loc[LocViewSize] = glGetUniformLocation(shader.prog, "viewSize");
	shader.loc[LocTex]      = glGetUniformLocation(shader.prog, "tex");
	shader.loc[LocParams]   = glGetUniformBlockIndex(shader.prog, "NisabaShaderParams");
}

static void xformToMat3x4(float* m3, const Transform2D& t) {
	t.toMat3x4(m3);
}

static GLenum convertBlendFactor(BlendFactor factor) {
	switch (factor) {
		case BlendFactor::Zero:                return GL_ZERO;
		case BlendFactor::One:                 return GL_ONE;
		case BlendFactor::SrcColor:            return GL_SRC_COLOR;
		case BlendFactor::OneMinusSrcColor:    return GL_ONE_MINUS_SRC_COLOR;
		case BlendFactor::DstColor:            return GL_DST_COLOR;
		case BlendFactor::OneMinusDstColor:    return GL_ONE_MINUS_DST_COLOR;
		case BlendFactor::SrcAlpha:            return GL_SRC_ALPHA;
		case BlendFactor::OneMinusSrcAlpha:    return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DstAlpha:            return GL_DST_ALPHA;
		case BlendFactor::OneMinusDstAlpha:    return GL_ONE_MINUS_DST_ALPHA;
		case BlendFactor::SrcAlphaSaturate:    return GL_SRC_ALPHA_SATURATE;
	}
	return GL_INVALID_ENUM;
}

static Blend convertBlend(const CompositeOperationState& op) {
	Blend b;
	b.srcRGB   = convertBlendFactor(op.srcRGB);
	b.dstRGB   = convertBlendFactor(op.dstRGB);
	b.srcAlpha = convertBlendFactor(op.srcAlpha);
	b.dstAlpha = convertBlendFactor(op.dstAlpha);
	if (b.srcRGB == GL_INVALID_ENUM || b.dstRGB == GL_INVALID_ENUM ||
	    b.srcAlpha == GL_INVALID_ENUM || b.dstAlpha == GL_INVALID_ENUM) {
		b.srcRGB   = GL_ONE;
		b.dstRGB   = GL_ONE_MINUS_SRC_ALPHA;
		b.srcAlpha = GL_ONE;
		b.dstAlpha = GL_ONE_MINUS_SRC_ALPHA;
	}
	return b;
}

} // namespace

// -------------------------------------------------------------
// GL3Renderer::Impl
// -------------------------------------------------------------
struct GL3Renderer::Impl {
	int flags{0};
	GLShader shader;
	GLShader stencilStrokeShader;
	GLShader simpleShader;
	GLShader solidVertShader;
	GLShader circleVertShader;
	GLShader rectVertShader;
	GLuint activeProg{0};
	float view[2]{0.0f, 0.0f};

	std::vector<Texture> textures;
	int textureIdCounter{0};
	int dummyTex{0};

	GLuint vertArr{0};
	GLuint vertBuf{0};
	GLuint fragBuf{0};
	int fragSize{0};

	// Per-frame batches
	std::vector<Call> calls;
	std::vector<GLPath> paths;
	std::vector<Vertex> verts;
	std::vector<uint8_t> uniforms;

	// Cached state filter to eliminate redundant GL state binds
	GLuint boundTexture{0};
	GLuint stencilMaskVal{0xffffffff};
	GLenum stencilFuncMode{GL_ALWAYS};
	GLint stencilFuncRefVal{0};
	GLuint stencilFuncMaskVal{0xffffffff};
	Blend blendState;
	int boundUniformOffset{-1};
	int lastTextureId{0};
	Texture* lastTexturePtr{nullptr};
	size_t fragBufCap{0};
	size_t vertBufCap{0};
	size_t fragRingOffset{0};
	int vertRingPos{0};
	int baseVertOffset{0};
	int baseUboOffset{0};
	bool scissorEnabled{false};
	bool stencilEnabled{false};
	int currentScissor[4]{-1, -1, -1, -1};
	int lastUniformOffset{-1};
	int currentClipDepth{0};

	void useProgramCached(GLuint prog) {
		if (activeProg != prog) {
			activeProg = prog;
			glUseProgram(prog);
		}
	}

	void enableStencil(bool enable) {
		if (stencilEnabled != enable) {
			stencilEnabled = enable;
			if (enable) {
				glEnable(GL_STENCIL_TEST);
			} else {
				glDisable(GL_STENCIL_TEST);
			}
		}
	}

	void bindTexture(GLuint tex) {
		if (boundTexture != tex) {
			boundTexture = tex;
			glBindTexture(GL_TEXTURE_2D, tex);
		}
	}

	void stencilMask(GLuint mask) {
		if (stencilMaskVal != mask) {
			stencilMaskVal = mask;
			glStencilMask(mask);
		}
	}

	void stencilFunc(GLenum func, GLint ref, GLuint mask) {
		if (stencilFuncMode != func || stencilFuncRefVal != ref || stencilFuncMaskVal != mask) {
			stencilFuncMode = func;
			stencilFuncRefVal = ref;
			stencilFuncMaskVal = mask;
			glStencilFunc(func, ref, mask);
		}
	}

	void blendFuncSeparate(const Blend& blend) {
		if (blendState.srcRGB != blend.srcRGB || blendState.dstRGB != blend.dstRGB ||
		    blendState.srcAlpha != blend.srcAlpha || blendState.dstAlpha != blend.dstAlpha) {
			blendState = blend;
			glBlendFuncSeparate(blend.srcRGB, blend.dstRGB, blend.srcAlpha, blend.dstAlpha);
		}
	}

	Texture* allocTexture() {
		for (auto& t : textures) {
			if (t.id == 0) {
				t = Texture{};
				t.id = ++textureIdCounter;
				return &t;
			}
		}
		textures.emplace_back();
		Texture* tex = &textures.back();
		tex->id = ++textureIdCounter;
		return tex;
	}

	Texture* findTexture(int id) {
		if (lastTexturePtr != nullptr && lastTextureId == id) return lastTexturePtr;
		for (auto& t : textures) {
			if (t.id == id) {
				lastTextureId = id;
				lastTexturePtr = &t;
				return &t;
			}
		}
		return nullptr;
	}

	bool deleteTexture(int id) {
		if (lastTextureId == id) {
			lastTextureId = 0;
			lastTexturePtr = nullptr;
		}
		for (auto& t : textures) {
			if (t.id == id) {
				if (t.tex != 0 && (t.flags & ImageFlagsGL::ImageNoDelete) == 0) {
					glDeleteTextures(1, &t.tex);
				}
				t = Texture{};
				return true;
			}
		}
		return false;
	}

	int allocFragUniforms(int n) {
		int ret = static_cast<int>(uniforms.size());
		uniforms.resize(ret + n * fragSize, 0);
		return ret;
	}

	FragUniforms* fragUniformPtr(int offset) {
		return reinterpret_cast<FragUniforms*>(&uniforms[offset]);
	}

	Texture* dummyTexPtr{nullptr};

	void setUniforms(int uniformOffset, int image) {
		const FragUniforms* f = fragUniformPtr(uniformOffset);
		if (image == 0 && f->scissorExt[0] < 0.0f) {
			if (f->type == ShaderSolid && f->strokeMult == 1.0f && solidVertShader.prog != 0) {
				useProgramCached(solidVertShader.prog);
				return;
			}
			if (f->type == ShaderCircle && circleVertShader.prog != 0) {
				useProgramCached(circleVertShader.prog);
				return;
			}
			if (f->type == ShaderRect && rectVertShader.prog != 0) {
				useProgramCached(rectVertShader.prog);
				return;
			}
		}
		useProgramCached(shader.prog);

		int actualOffset = baseUboOffset + uniformOffset;
		if (boundUniformOffset != actualOffset) {
			boundUniformOffset = actualOffset;
			glBindBufferRange(GL_UNIFORM_BUFFER, ParamsBinding, fragBuf, actualOffset, sizeof(NisabaShaderUniforms));
		}

		if (image != 0) {
			Texture* tex = findTexture(image);
			bindTexture(tex != nullptr ? tex->tex : (dummyTexPtr != nullptr ? dummyTexPtr->tex : 0));
			checkGLError(flags, "tex paint tex");
		}
	}

	void bindCallShader(const Call& call, int uniformOffset, int image) {
		if (call.customShader) {
			auto rts = std::dynamic_pointer_cast<RuntimeShader>(call.customShader);
			if (rts) {
				activeProg = 0;
				rts->bind(view);
				return;
			}
		}
		setUniforms(uniformOffset, image);
	}

	void unbindCallShader(const Call& call) {
		if (call.customShader) {
			useProgramCached(shader.prog);
			boundUniformOffset = -1;
		}
	}

	std::vector<GLint> fanFirst;
	std::vector<GLsizei> fanCount;
	std::vector<GLint> stripFirst;
	std::vector<GLsizei> stripCount;

	bool getHardwareScissor(const Scissor& scissor, int outRect[4]) {
		if (scissor.extent[0] < -0.5f || scissor.extent[1] < -0.5f) {
			outRect[0] = -1;
			outRect[1] = -1;
			outRect[2] = -1;
			outRect[3] = -1;
			return false;
		}
		if (std::abs(scissor.xform[1]) > 1e-4f || std::abs(scissor.xform[2]) > 1e-4f) {
			outRect[0] = -1;
			return false;
		}
		float sx = scissor.xform[0];
		float sy = scissor.xform[3];
		if (sx <= 0.0f || sy <= 0.0f) {
			outRect[0] = -1;
			return false;
		}
		float cx = scissor.xform[4];
		float cy = scissor.xform[5];
		float hw = scissor.extent[0] * sx;
		float hh = scissor.extent[1] * sy;

		float minX = cx - hw;
		float maxX = cx + hw;
		float minY = cy - hh;
		float maxY = cy + hh;

		int ix = static_cast<int>(std::floor(minX));
		int iy = static_cast<int>(std::floor(view[1] - maxY));
		int iw = static_cast<int>(std::ceil(maxX - minX));
		int ih = static_cast<int>(std::ceil(maxY - minY));

		outRect[0] = std::max(0, ix);
		outRect[1] = std::max(0, iy);
		outRect[2] = std::max(0, iw);
		outRect[3] = std::max(0, ih);
		return true;
	}

	bool convertPaint(FragUniforms* frag, const Paint& paint, const Scissor& scissor,
	                  float width, float fringe, float strokeThr, bool hasHwScissor = false) {
		*frag = FragUniforms{};

		frag->innerCol = paint.innerColor.premultiplied();
		frag->outerCol = paint.outerColor.premultiplied();

		if (hasHwScissor || scissor.extent[0] < -0.5f || scissor.extent[1] < -0.5f) {
			std::memset(frag->scissorMat, 0, sizeof(frag->scissorMat));
			frag->scissorExt[0] = -1.0f;
			frag->scissorExt[1] = -1.0f;
			frag->scissorScale[0] = 1.0f;
			frag->scissorScale[1] = 1.0f;
		} else {
			Transform2D invxform;
			scissor.xform.inverse(invxform);
			xformToMat3x4(frag->scissorMat, invxform);
			frag->scissorExt[0] = scissor.extent[0];
			frag->scissorExt[1] = scissor.extent[1];
			float effFringe = (fringe > 1e-4f) ? fringe : 1.0f;
			frag->scissorScale[0] = std::sqrt(scissor.xform[0] * scissor.xform[0] + scissor.xform[2] * scissor.xform[2]) / effFringe;
			frag->scissorScale[1] = std::sqrt(scissor.xform[1] * scissor.xform[1] + scissor.xform[3] * scissor.xform[3]) / effFringe;
		}

		frag->extent[0] = paint.extent[0];
		frag->extent[1] = paint.extent[1];
		frag->strokeMult = (fringe > 1e-4f) ? ((width * 0.5f + fringe * 0.5f) / fringe) : 1.0f;
		frag->strokeThr = strokeThr;

		Transform2D invxform;
		if (paint.image != 0) {
			Texture* tex = findTexture(paint.image);
			if (tex == nullptr) return false;

			if ((tex->flags & ImageFlags::ImageFlipY) != 0) {
				Transform2D m1 = Transform2D::translate(0.0f, frag->extent[1] * 0.5f).multiply(paint.xform);
				Transform2D m2 = Transform2D::scale(1.0f, -1.0f).multiply(m1);
				Transform2D m3 = Transform2D::translate(0.0f, -frag->extent[1] * 0.5f).multiply(m2);
				m3.inverse(invxform);
			} else {
				paint.xform.inverse(invxform);
			}
			frag->type = ShaderFillImg;

			if (tex->type == TextureType::RGBA) {
				frag->texType = (tex->flags & ImageFlags::ImagePremultiplied) ? 0 : 1;
			} else {
				frag->texType = 2;
			}
			float invExtX = (frag->extent[0] > 1e-4f) ? (1.0f / frag->extent[0]) : 1.0f;
			float invExtY = (frag->extent[1] > 1e-4f) ? (1.0f / frag->extent[1]) : 1.0f;
			Transform2D uvXform = invxform.multiply(Transform2D::scale(invExtX, invExtY));
			xformToMat3x4(frag->paintMat, uvXform);
		} else if (paint.extent[0] == 0.0f && paint.extent[1] == 0.0f && paint.radius == 0.0f &&
		           paint.innerColor.r == paint.outerColor.r &&
		           paint.innerColor.g == paint.outerColor.g &&
		           paint.innerColor.b == paint.outerColor.b &&
		           paint.innerColor.a == paint.outerColor.a) {
			frag->type = ShaderSolid;
			frag->radius = 0.0f;
			frag->feather = 1.0f;
			frag->innerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
			frag->outerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
		} else if (paint.radius == 0.0f && paint.extent[0] == 0.0f) {
			frag->type = ShaderLinearGrad;
			frag->radius = 0.0f;
			frag->feather = paint.feather;
			paint.xform.inverse(invxform);
			float invFeather = (paint.feather > 1e-4f) ? (1.0f / paint.feather) : 1.0f;
			Transform2D gradXform = invxform.multiply(Transform2D::scale(1.0f, invFeather));
			xformToMat3x4(frag->paintMat, gradXform);
		} else if (paint.radius > 0.0f && paint.extent[0] == paint.radius && paint.extent[1] == paint.radius) {
			frag->type = ShaderRadialGrad;
			frag->radius = paint.radius;
			frag->feather = paint.feather;
			float invFeather = (paint.feather > 1e-4f) ? (1.0f / paint.feather) : 1.0f;
			frag->extent[0] = invFeather;
			frag->extent[1] = 0.5f - paint.radius * invFeather;
			paint.xform.inverse(invxform);
			xformToMat3x4(frag->paintMat, invxform);
		} else {
			frag->type = ShaderFillGrad;
			frag->radius = paint.radius;
			frag->feather = paint.feather;
			paint.xform.inverse(invxform);
			xformToMat3x4(frag->paintMat, invxform);
		}

		return true;
	}

	void fill(const Call& call, int activeClipDepth) {
		const GLPath* callPaths = &paths[call.pathOffset];
		int npaths = call.pathCount;

		if (activeClipDepth > 0) {
			int clipRef = activeClipDepth << 4;
			// 1. Draw shapes into Stencil Buffer (lower nibble bits 0..3)
			enableStencil(true);
			stencilMask(0x0f);
			stencilFunc(GL_EQUAL, clipRef, 0xf0);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			if (simpleShader.prog != 0) useProgramCached(simpleShader.prog);
			else setUniforms(call.uniformOffset, 0);
			checkGLError(flags, "fill clip simple");

			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
			glDisable(GL_CULL_FACE);
			for (int i = 0; i < npaths; i++) {
				glDrawArrays(GL_TRIANGLE_FAN, baseVertOffset + callPaths[i].fillOffset, callPaths[i].fillCount);
			}
			glEnable(GL_CULL_FACE);

			// 2. Draw anti-aliased pixels (AA Fringe)
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			bindCallShader(call, call.uniformOffset, call.image);
			checkGLError(flags, "fill clip fill");

			if ((flags & CreateFlags::Antialias) != 0) {
				stencilMask(0x00);
				stencilFunc(GL_EQUAL, clipRef, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				for (int i = 0; i < npaths; i++) {
					glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[i].strokeOffset, callPaths[i].strokeCount);
				}
			}

			// 3. Draw fill cover quad
			glDisable(GL_CULL_FACE);
			stencilMask(0x0f);
			stencilFunc(GL_NOTEQUAL, clipRef, 0x0f);
			glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
			glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + call.triangleOffset, call.triangleCount);
			glEnable(GL_CULL_FACE);

			stencilMask(0x00);
			stencilFunc(GL_EQUAL, clipRef, 0xf0);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			unbindCallShader(call);
			return;
		}

		// 1. Draw shapes into Stencil Buffer (unclipped)
		enableStencil(true);
		stencilMask(0xff);
		stencilFunc(GL_ALWAYS, 0, 0xff);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		if (simpleShader.prog != 0) useProgramCached(simpleShader.prog);
		else setUniforms(call.uniformOffset, 0);
		checkGLError(flags, "fill simple");

		glDisable(GL_CULL_FACE);
		if (npaths == 1) {
			// Single contour concave polygon (e.g. Star): GL_INVERT is 2x faster than Separate INCR/DECR!
			glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
			glDrawArrays(GL_TRIANGLE_FAN, baseVertOffset + callPaths[0].fillOffset, callPaths[0].fillCount);
		} else {
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
			for (int i = 0; i < npaths; i++) {
				glDrawArrays(GL_TRIANGLE_FAN, baseVertOffset + callPaths[i].fillOffset, callPaths[i].fillCount);
			}
		}
		glEnable(GL_CULL_FACE);

		// 2. Draw anti-aliased pixels (AA Fringe)
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		bindCallShader(call, call.uniformOffset, call.image);
		checkGLError(flags, "fill fill");

		if ((flags & CreateFlags::Antialias) != 0) {
			stencilFunc(GL_EQUAL, 0x00, 0xff);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			for (int i = 0; i < npaths; i++) {
				glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[i].strokeOffset, callPaths[i].strokeCount);
			}
		}

		// 3. Draw fill cover quad
		stencilFunc(GL_NOTEQUAL, 0x0, 0xff);
		glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
		glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + call.triangleOffset, call.triangleCount);

		enableStencil(false);
		unbindCallShader(call);
	}

	void convexFill(const Call& call, int activeClipDepth) {
		const GLPath* callPaths = &paths[call.pathOffset];
		int npaths = call.pathCount;

		if (activeClipDepth > 0) {
			enableStencil(true);
			stencilMask(0x00);
			stencilFunc(GL_EQUAL, activeClipDepth << 4, 0xf0);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			enableStencil(false);
		}

		bindCallShader(call, call.uniformOffset, call.image);
		checkGLError(flags, "convex fill");

		if (npaths == 1) {
			if (callPaths[0].fillCount > 0) {
				glDrawArrays(GL_TRIANGLE_FAN, baseVertOffset + callPaths[0].fillOffset, callPaths[0].fillCount);
			}
			if (callPaths[0].strokeCount > 0) {
				glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[0].strokeOffset, callPaths[0].strokeCount);
			}
			unbindCallShader(call);
			return;
		}

		fanFirst.clear();
		fanCount.clear();
		stripFirst.clear();
		stripCount.clear();

		fanFirst.reserve(npaths);
		fanCount.reserve(npaths);
		stripFirst.reserve(npaths);
		stripCount.reserve(npaths);

		for (int i = 0; i < npaths; i++) {
			if (callPaths[i].fillCount > 0) {
				fanFirst.push_back(baseVertOffset + callPaths[i].fillOffset);
				fanCount.push_back(callPaths[i].fillCount);
			}
			if (callPaths[i].strokeCount > 0) {
				stripFirst.push_back(baseVertOffset + callPaths[i].strokeOffset);
				stripCount.push_back(callPaths[i].strokeCount);
			}
		}

		if (!fanFirst.empty()) {
			glMultiDrawArrays(GL_TRIANGLE_FAN, fanFirst.data(), fanCount.data(), static_cast<GLsizei>(fanFirst.size()));
		}
		if (!stripFirst.empty()) {
			glMultiDrawArrays(GL_TRIANGLE_STRIP, stripFirst.data(), stripCount.data(), static_cast<GLsizei>(stripFirst.size()));
		}
		unbindCallShader(call);
	}

	void stroke(const Call& call, int activeClipDepth) {
		const GLPath* callPaths = &paths[call.pathOffset];
		int npaths = call.pathCount;

		if (call.useStencil) {
			enableStencil(true);
			bool switchedProg = (!call.customShader && stencilStrokeShader.prog != 0);
			if (activeClipDepth > 0) {
				int clipRef = activeClipDepth << 4;
				stencilMask(0x0f);
				stencilFunc(GL_EQUAL, clipRef, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
				bindCallShader(call, call.uniformOffset + fragSize, call.image);
				if (switchedProg) {
					useProgramCached(stencilStrokeShader.prog);
				}
				checkGLError(flags, "stroke fill 0");
				for (int i = 0; i < npaths; i++) {
					glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[i].strokeOffset, callPaths[i].strokeCount);
				}

				bindCallShader(call, call.uniformOffset, call.image);
				stencilMask(0x00);
				stencilFunc(GL_EQUAL, clipRef, 0xf0);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				for (int i = 0; i < npaths; i++) {
					glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[i].strokeOffset, callPaths[i].strokeCount);
				}

				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				if (simpleShader.prog != 0) useProgramCached(simpleShader.prog);
				stencilMask(0x0f);
				stencilFunc(GL_NOTEQUAL, 0x00, 0x0f);
				glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
				checkGLError(flags, "stroke fill 1");
				for (int i = 0; i < npaths; i++) {
					glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[i].strokeOffset, callPaths[i].strokeCount);
				}
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				stencilMask(0x00);
				stencilFunc(GL_EQUAL, clipRef, 0xf0);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			} else {
				stencilMask(0xff);
				stencilFunc(GL_EQUAL, 0x0, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
				bindCallShader(call, call.uniformOffset + fragSize, call.image);
				if (switchedProg) {
					useProgramCached(stencilStrokeShader.prog);
				}
				checkGLError(flags, "stroke fill 0");
				for (int i = 0; i < npaths; i++) {
					glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[i].strokeOffset, callPaths[i].strokeCount);
				}

				bindCallShader(call, call.uniformOffset, call.image);
				stencilFunc(GL_EQUAL, 0x00, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				for (int i = 0; i < npaths; i++) {
					glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[i].strokeOffset, callPaths[i].strokeCount);
				}

				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				if (simpleShader.prog != 0) useProgramCached(simpleShader.prog);
				stencilFunc(GL_ALWAYS, 0, 0xff);
				glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
				checkGLError(flags, "stroke fill 1");
				for (int i = 0; i < npaths; i++) {
					glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[i].strokeOffset, callPaths[i].strokeCount);
				}
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				enableStencil(false);
			}
			unbindCallShader(call);
		} else {
			if (activeClipDepth > 0) {
				enableStencil(true);
				stencilMask(0x00);
				stencilFunc(GL_EQUAL, activeClipDepth << 4, 0xf0);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			} else {
				enableStencil(false);
			}
			bindCallShader(call, call.uniformOffset, call.image);
			checkGLError(flags, "stroke");
			if (npaths == 1) {
				if (callPaths[0].strokeCount > 0) {
					glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + callPaths[0].strokeOffset, callPaths[0].strokeCount);
				}
			} else {
				stripFirst.clear();
				stripCount.clear();
				stripFirst.reserve(npaths);
				stripCount.reserve(npaths);
				for (int i = 0; i < npaths; i++) {
					if (callPaths[i].strokeCount > 0) {
						stripFirst.push_back(baseVertOffset + callPaths[i].strokeOffset);
						stripCount.push_back(callPaths[i].strokeCount);
					}
				}
				if (!stripFirst.empty()) {
					glMultiDrawArrays(GL_TRIANGLE_STRIP, stripFirst.data(), stripCount.data(), static_cast<GLsizei>(stripFirst.size()));
				}
			}
			unbindCallShader(call);
		}
	}

	void triangles(const Call& call, int activeClipDepth) {
		if (activeClipDepth > 0) {
			enableStencil(true);
			stencilMask(0x00);
			stencilFunc(GL_EQUAL, activeClipDepth << 4, 0xf0);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			enableStencil(false);
		}
		bindCallShader(call, call.uniformOffset, call.image);
		checkGLError(flags, "triangles fill");
		glDrawArrays(GL_TRIANGLES, baseVertOffset + call.triangleOffset, call.triangleCount);
		unbindCallShader(call);
	}

	void clip(const Call& call, int& activeClipDepth) {
		const GLPath* callPaths = &paths[call.pathOffset];
		int npaths = call.pathCount;
		int prevDepth = call.clipDepth - 1;
		int prevDepthVal = prevDepth << 4;
		int newDepthVal = call.clipDepth << 4;

		enableStencil(true);
		glDisable(GL_CULL_FACE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		stencilMask(0x0f);
		stencilFunc(GL_EQUAL, prevDepthVal, 0xf0);

		if (simpleShader.prog != 0) useProgramCached(simpleShader.prog);
		checkGLError(flags, "clip path");

		if (call.winding == Winding::Clockwise) {
			glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
		} else {
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
		}

		for (int i = 0; i < npaths; i++) {
			glDrawArrays(GL_TRIANGLE_FAN, baseVertOffset + callPaths[i].fillOffset, callPaths[i].fillCount);
		}

		stencilMask(0xff);
		stencilFunc(GL_NOTEQUAL, newDepthVal, 0x0f);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + call.triangleOffset, call.triangleCount);

		activeClipDepth = call.clipDepth;
		glEnable(GL_CULL_FACE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		stencilMask(0x00);
		stencilFunc(GL_EQUAL, newDepthVal, 0xf0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	}

	void restoreClip(const Call& call, int& activeClipDepth) {
		if (call.clipDepth < activeClipDepth) {
			int targetDepthVal = call.clipDepth << 4;
			enableStencil(true);
			glDisable(GL_CULL_FACE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthMask(GL_FALSE);
			stencilMask(0xff);
			stencilFunc(GL_LESS, targetDepthVal, 0xf0);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			if (simpleShader.prog != 0) useProgramCached(simpleShader.prog);
			checkGLError(flags, "restore clip");

			glDrawArrays(GL_TRIANGLE_STRIP, baseVertOffset + call.triangleOffset, call.triangleCount);

			activeClipDepth = call.clipDepth;
			glEnable(GL_CULL_FACE);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);
			stencilMask(0x00);

			if (activeClipDepth == 0) {
				enableStencil(false);
			} else {
				stencilFunc(GL_EQUAL, activeClipDepth << 4, 0xf0);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			}
		}
	}
};

// -------------------------------------------------------------
// GL3Renderer Implementation
// -------------------------------------------------------------
GL3Renderer::GL3Renderer(int flags)
	: m_impl(std::make_unique<Impl>()) {
	m_impl->flags = flags;
}

GL3Renderer::~GL3Renderer() {
	shutdown();
}

bool GL3Renderer::init() {
	const char* ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	bool is_gles = (ver != nullptr && std::strstr(ver, "OpenGL ES") != nullptr);

	static const char* shaderHeaderGL =
		"#version 150 core\n"
		"#define NISABA_GL3 1\n"
		"#define USE_UNIFORMBUFFER 1\n\n";

	static const char* shaderHeaderGLES =
		"#version 300 es\n"
		"#define NISABA_GL3 1\n"
		"#define USE_UNIFORMBUFFER 1\n"
		"precision highp float;\n"
		"precision highp int;\n\n";

	const char* shaderHeader = is_gles ? shaderHeaderGLES : shaderHeaderGL;

	static const char* fillVertShader =
		"uniform vec2 viewSize;\n"
		"in vec2 vertex;\n"
		"in vec2 tcoord;\n"
		"in vec4 color;\n"
		"out vec2 ftcoord;\n"
		"out vec2 fpos;\n"
		"out vec4 fcolor;\n"
		"void main(void) {\n"
		"	ftcoord = tcoord;\n"
		"	fpos = vertex;\n"
		"	fcolor = color;\n"
		"	gl_Position = vec4(2.0*vertex.x/viewSize.x - 1.0, 1.0 - 2.0*vertex.y/viewSize.y, 0, 1);\n"
		"}\n";

	// Ultra-lightweight specialized shaders with ZERO UBO reads and ZERO branches!
	static const char* simpleVertShader =
		"uniform vec2 viewSize;\n"
		"in vec2 vertex;\n"
		"void main(void) {\n"
		"	gl_Position = vec4(2.0*vertex.x/viewSize.x - 1.0, 1.0 - 2.0*vertex.y/viewSize.y, 0, 1);\n"
		"}\n";
	static const char* simpleFragShader =
		"out vec4 outColor;\n"
		"void main(void) {\n"
		"	outColor = vec4(1.0);\n"
		"}\n";
	static const char* solidVertFragShader =
		"in vec2 ftcoord;\n"
		"in vec4 fcolor;\n"
		"out vec4 outColor;\n"
		"void main(void) {\n"
		"	float alpha = min(1.0, 1.0 - abs(ftcoord.x * 2.0 - 1.0)) * min(1.0, ftcoord.y);\n"
		"	outColor = fcolor * alpha;\n"
		"}\n";
	static const char* circleVertFragShader =
		"in vec2 ftcoord;\n"
		"in vec4 fcolor;\n"
		"out vec4 outColor;\n"
		"void main(void) {\n"
		"	float dist = length(ftcoord);\n"
		"	float alpha = clamp((1.0 - dist) / max(fwidth(dist), 0.0001) + 0.5, 0.0, 1.0);\n"
		"	outColor = fcolor * alpha;\n"
		"}\n";
	static const char* rectVertFragShader =
		"in vec2 ftcoord;\n"
		"in vec4 fcolor;\n"
		"out vec4 outColor;\n"
		"void main(void) {\n"
		"	vec2 d = (vec2(1.0) - abs(ftcoord)) / max(fwidth(ftcoord), vec2(0.0001));\n"
		"	vec2 cov = clamp(d + vec2(0.5), 0.0, 1.0);\n"
		"	outColor = fcolor * (cov.x * cov.y);\n"
		"}\n";

	static const char* fillFragShader =
		"#ifdef EDGE_AA\n"
		"#define EDGE_AA 1\n"
		"#endif\n"
		"layout(std140) uniform NisabaShaderParams {\n"
		"	mat3 scissorMat;\n"
		"	mat3 paintMat;\n"
		"	vec4 innerCol;\n"
		"	vec4 outerCol;\n"
		"	vec2 scissorExt;\n"
		"	vec2 scissorScale;\n"
		"	vec2 extent;\n"
		"	float radius;\n"
		"	float feather;\n"
		"	float strokeMult;\n"
		"	float strokeThr;\n"
		"	int texType;\n"
		"	int type;\n"
		"};\n"
		"uniform sampler2D tex;\n"
		"in vec2 ftcoord;\n"
		"in vec2 fpos;\n"
		"in vec4 fcolor;\n"
		"out vec4 outColor;\n"
		"\n"
		"float sdf_rounded_rect(vec2 pt, vec2 ext, float rad) {\n"
		"	if (ext.x == rad && ext.y == rad) return length(pt) - rad;\n"
		"	vec2 ext2 = ext - vec2(rad,rad);\n"
		"	vec2 d = abs(pt) - ext2;\n"
		"	return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;\n"
		"}\n"
		"\n"
		"float evaluate_scissor_clip(vec2 p) {\n"
		"	vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);\n"
		"	sc = vec2(0.5,0.5) - sc * scissorScale;\n"
		"	return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);\n"
		"}\n"
		"#ifdef EDGE_AA\n"
		"float evaluate_edge_coverage() {\n"
		"	return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, ftcoord.y);\n"
		"}\n"
		"#endif\n"
		"\n"
		"void main(void) {\n"
		"	if (type == 2) {\n"
		"		outColor = innerCol;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 4) { // Solid color (Ultra-fast path with per-vertex color)\n"
		"#ifdef EDGE_AA\n"
		"		float alpha = evaluate_edge_coverage();\n"
		"#ifdef STENCIL_STROKES\n"
		"		if (alpha < strokeThr) discard;\n"
		"#endif\n"
		"#else\n"
		"		float alpha = 1.0;\n"
		"#endif\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		outColor = fcolor * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 8) { // Analytical Anti-Aliased Rectangle Quad\n"
		"		vec2 d = (vec2(1.0) - abs(ftcoord)) / max(fwidth(ftcoord), vec2(0.0001));\n"
		"		vec2 cov = clamp(d + vec2(0.5), 0.0, 1.0);\n"
		"		float alpha = cov.x * cov.y;\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		outColor = fcolor * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 7) { // Analytical Anti-Aliased Circle/Ellipse Quad\n"
		"		float dist = length(ftcoord);\n"
		"		float alpha = clamp((1.0 - dist) / max(fwidth(dist), 0.0001) + 0.5, 0.0, 1.0);\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		outColor = fcolor * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 9) { // Analytical Anti-Aliased Rounded Rectangle Quad\n"
		"		float r = (ftcoord.x < 0.0) ? ((ftcoord.y < 0.0) ? radius : strokeThr) : ((ftcoord.y < 0.0) ? feather : strokeMult);\n"
		"		vec2 ext2 = extent - vec2(r, r);\n"
		"		vec2 d = abs(ftcoord) - ext2;\n"
		"		float dist = min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;\n"
		"		float alpha = clamp(-dist / max(fwidth(dist), 0.0001) + 0.5, 0.0, 1.0);\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		outColor = fcolor * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 3) { // Textured triangles (Text / Direct UV Quad)\n"
		"		vec4 color = texture(tex, ftcoord);\n"
		"		if (texType == 2) color = vec4(color.r);\n"
		"		else if (texType == 1) color = vec4(color.rgb * color.a, color.a);\n"
		"		color *= innerCol * fcolor;\n"
		"		if (scissorExt.x >= 0.0) color *= evaluate_scissor_clip(fpos);\n"
		"		outColor = color;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 5) { // Fast-path Linear Gradient\n"
		"		float alpha = 1.0;\n"
		"		float d;\n"
		"		if (radius > 0.5) {\n"
		"			d = clamp(ftcoord.y, 0.0, 1.0);\n"
		"		} else {\n"
		"#ifdef EDGE_AA\n"
		"			alpha = evaluate_edge_coverage();\n"
		"#ifdef STENCIL_STROKES\n"
		"			if (alpha < strokeThr) discard;\n"
		"#endif\n"
		"#endif\n"
		"			d = clamp((paintMat * vec3(fpos,1.0)).y, 0.0, 1.0);\n"
		"		}\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		outColor = mix(innerCol,outerCol,d) * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 6) { // Fast-path Radial Gradient\n"
		"		float alpha = 1.0;\n"
		"		vec2 pt;\n"
		"		if (strokeMult > 3.5) {\n"
		"			float dist = length(ftcoord);\n"
		"			alpha = clamp((strokeThr - dist) / max(fwidth(dist), 0.0001) + 0.5, 0.0, 1.0);\n"
		"			if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"			float d = clamp(dist * extent.x + extent.y, 0.0, 1.0);\n"
		"			outColor = mix(innerCol,outerCol,d) * alpha;\n"
		"			return;\n"
		"		} else if (strokeMult > 2.5) {\n"
		"			pt = ftcoord;\n"
		"		} else if (strokeMult > 1.5) {\n"
		"			float dist = length(ftcoord);\n"
		"			alpha = clamp((1.0 - dist) / max(fwidth(dist), 0.0001) + 0.5, 0.0, 1.0);\n"
		"			pt = (paintMat * vec3(fpos,1.0)).xy;\n"
		"		} else {\n"
		"#ifdef EDGE_AA\n"
		"			alpha = evaluate_edge_coverage();\n"
		"#ifdef STENCIL_STROKES\n"
		"			if (alpha < strokeThr) discard;\n"
		"#endif\n"
		"#endif\n"
		"			pt = (paintMat * vec3(fpos,1.0)).xy;\n"
		"		}\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		float d = clamp(length(pt) * extent.x + extent.y, 0.0, 1.0);\n"
		"		outColor = mix(innerCol,outerCol,d) * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 0) { // Box Gradient\n"
		"		float alpha = 1.0;\n"
		"		vec2 pt;\n"
		"		if (strokeMult > 2.5) {\n"
		"			pt = ftcoord;\n"
		"		} else {\n"
		"#ifdef EDGE_AA\n"
		"			alpha = evaluate_edge_coverage();\n"
		"#ifdef STENCIL_STROKES\n"
		"			if (alpha < strokeThr) discard;\n"
		"#endif\n"
		"#endif\n"
		"			pt = (paintMat * vec3(fpos,1.0)).xy;\n"
		"		}\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		float d = clamp((sdf_rounded_rect(pt, extent, radius) + feather*0.5) / feather, 0.0, 1.0);\n"
		"		outColor = mix(innerCol,outerCol,d) * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 1) { // Image Pattern\n"
		"		float alpha = 1.0;\n"
		"		vec2 pt;\n"
		"		if (radius > 0.0) {\n"
		"			vec2 ext2 = extent - vec2(radius, radius);\n"
		"			vec2 d = abs(ftcoord) - ext2;\n"
		"			float dist = min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;\n"
		"			alpha = clamp(-dist / max(fwidth(dist), 0.0001) + 0.5, 0.0, 1.0);\n"
		"			pt = (paintMat * vec3(ftcoord,1.0)).xy;\n"
		"		} else {\n"
		"#ifdef EDGE_AA\n"
		"			alpha = evaluate_edge_coverage();\n"
		"#ifdef STENCIL_STROKES\n"
		"			if (alpha < strokeThr) discard;\n"
		"#endif\n"
		"#endif\n"
		"			pt = (paintMat * vec3(fpos,1.0)).xy;\n"
		"		}\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		vec4 color = texture(tex, pt);\n"
		"		if (texType == 1) color = vec4(color.rgb * color.a, color.a);\n"
		"		else if (texType == 2) color = vec4(color.r);\n"
		"		outColor = color * innerCol * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 11) { // Analytical Anti-Aliased Rounded Rectangle / Circle with Linear Gradient\n"
		"		float r = (ftcoord.x < 0.0) ? ((ftcoord.y < 0.0) ? radius : strokeThr) : ((ftcoord.y < 0.0) ? feather : strokeMult);\n"
		"		vec2 ext2 = extent - vec2(r, r);\n"
		"		vec2 d = abs(ftcoord) - ext2;\n"
		"		float dist = min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;\n"
		"		float alpha = clamp(-dist / max(fwidth(dist), 0.0001) + 0.5, 0.0, 1.0);\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		vec2 pt = (paintMat * vec3(ftcoord,1.0)).xy;\n"
		"		float gradD = clamp(pt.y, 0.0, 1.0);\n"
		"		outColor = mix(innerCol, outerCol, gradD) * alpha;\n"
		"		return;\n"
		"	}\n"
		"	if (type == 10) { // Analytical Anti-Aliased Ring / Stroked RRect / Stroked Ellipse Quad\n"
		"		float dist;\n"
		"		if (extent.x > 0.0) {\n"
		"			vec2 ext2 = extent - vec2(radius, radius);\n"
		"			vec2 d = abs(ftcoord) - ext2;\n"
		"			dist = abs(min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius) - strokeThr;\n"
		"		} else if (extent.x < 0.0) {\n"
		"			vec2 rxry = vec2(-extent.x, extent.y);\n"
		"			float u = length(ftcoord);\n"
		"			float grad = max(length(ftcoord / rxry), 0.0001);\n"
		"			dist = abs(u - 1.0) / grad - strokeThr;\n"
		"		} else {\n"
		"			dist = abs(length(ftcoord) - radius) - strokeThr;\n"
		"		}\n"
		"		float alpha = clamp(-dist / max(fwidth(dist), 0.0001) + 0.5, 0.0, 1.0);\n"
		"		if (scissorExt.x >= 0.0) alpha *= evaluate_scissor_clip(fpos);\n"
		"		outColor = fcolor * alpha;\n"
		"		return;\n"
		"	}\n"
		"	outColor = vec4(1.0);\n"
		"}\n";

	checkGLError(m_impl->flags, "init start");

	// Compile main shader WITHOUT discard (STENCIL_STROKES) so hardware Early-Z/Stencil is 100% active!
	std::string mainOpts;
	if (m_impl->flags & CreateFlags::Antialias) mainOpts += "#define EDGE_AA 1\n";

	if (!createShader(m_impl->shader, "shader", shaderHeader, mainOpts.empty() ? nullptr : mainOpts.c_str(), fillVertShader, fillFragShader)) {
		return false;
	}
	getUniforms(m_impl->shader);

	auto bindAttr3 = [](GLShader& s) {
		if (s.prog != 0) {
			glBindAttribLocation(s.prog, 2, "color");
			glLinkProgram(s.prog);
			getUniforms(s);
		}
	};
	bindAttr3(m_impl->shader);

	if (createShader(m_impl->simpleShader, "simple_shader", shaderHeader, nullptr, simpleVertShader, simpleFragShader)) {
		getUniforms(m_impl->simpleShader);
	}
	if (createShader(m_impl->solidVertShader, "solid_vert_shader", shaderHeader, nullptr, fillVertShader, solidVertFragShader)) {
		bindAttr3(m_impl->solidVertShader);
	}
	if (createShader(m_impl->circleVertShader, "circle_vert_shader", shaderHeader, nullptr, fillVertShader, circleVertFragShader)) {
		bindAttr3(m_impl->circleVertShader);
	}
	if (createShader(m_impl->rectVertShader, "rect_vert_shader", shaderHeader, nullptr, fillVertShader, rectVertFragShader)) {
		bindAttr3(m_impl->rectVertShader);
	}

	if (m_impl->flags & CreateFlags::StencilStrokes) {
		std::string stencilOpts = mainOpts + "#define STENCIL_STROKES 1\n";
		if (createShader(m_impl->stencilStrokeShader, "stencil_stroke_shader", shaderHeader, stencilOpts.c_str(), fillVertShader, fillFragShader)) {
			bindAttr3(m_impl->stencilStrokeShader);
			glUniformBlockBinding(m_impl->stencilStrokeShader.prog, m_impl->stencilStrokeShader.loc[LocParams], ParamsBinding);
			glUseProgram(m_impl->stencilStrokeShader.prog);
			glUniform1i(m_impl->stencilStrokeShader.loc[LocTex], 0);
			glUseProgram(0);
		}
	}

	glGenVertexArrays(1, &m_impl->vertArr);
	glGenBuffers(1, &m_impl->vertBuf);

	glBindVertexArray(m_impl->vertArr);
	glBindBuffer(GL_ARRAY_BUFFER, m_impl->vertBuf);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(2 * sizeof(float)));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUniformBlockBinding(m_impl->shader.prog, m_impl->shader.loc[LocParams], ParamsBinding);
	glGenBuffers(1, &m_impl->fragBuf);

	GLint align = 4;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
	m_impl->fragSize = sizeof(FragUniforms) + align - (sizeof(FragUniforms) % align);

	// Pre-allocate ring buffers once at init to avoid per-frame GEM allocations
	m_impl->fragBufCap = (524288 / m_impl->fragSize) * m_impl->fragSize;
	glBindBuffer(GL_UNIFORM_BUFFER, m_impl->fragBuf);
	glBufferData(GL_UNIFORM_BUFFER, m_impl->fragBufCap, nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	m_impl->vertBufCap = static_cast<size_t>(262144) * sizeof(Vertex);
	glBindBuffer(GL_ARRAY_BUFFER, m_impl->vertBuf);
	glBufferData(GL_ARRAY_BUFFER, m_impl->vertBufCap, nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create dummy texture for unbound shaders
	m_impl->dummyTex = createTexture(TextureType::Alpha, 1, 1, 0, nullptr);
	m_impl->dummyTexPtr = m_impl->findTexture(m_impl->dummyTex);

	glUseProgram(m_impl->shader.prog);
	glUniform1i(m_impl->shader.loc[LocTex], 0);
	glUseProgram(0);

	checkGLError(m_impl->flags, "init done");
	glFinish();
	return true;
}

void GL3Renderer::shutdown() {
	if (!m_impl) return;

	deleteShader(m_impl->shader);
	deleteShader(m_impl->stencilStrokeShader);
	deleteShader(m_impl->simpleShader);
	deleteShader(m_impl->solidVertShader);
	deleteShader(m_impl->circleVertShader);
	deleteShader(m_impl->rectVertShader);

	if (m_impl->fragBuf != 0) {
		glDeleteBuffers(1, &m_impl->fragBuf);
		m_impl->fragBuf = 0;
	}
	if (m_impl->vertArr != 0) {
		glDeleteVertexArrays(1, &m_impl->vertArr);
		m_impl->vertArr = 0;
	}
	if (m_impl->vertBuf != 0) {
		glDeleteBuffers(1, &m_impl->vertBuf);
		m_impl->vertBuf = 0;
	}

	for (auto& t : m_impl->textures) {
		if (t.tex != 0 && (t.flags & ImageFlagsGL::ImageNoDelete) == 0) {
			glDeleteTextures(1, &t.tex);
		}
	}
	m_impl->textures.clear();
}

bool GL3Renderer::edgeAntiAlias() const {
	return (m_impl->flags & CreateFlags::Antialias) != 0;
}

int GL3Renderer::createTexture(TextureType type, int width, int height, int imageFlags, const unsigned char* data) {
	Texture* tex = m_impl->allocTexture();
	if (!tex) return 0;

	glGenTextures(1, &tex->tex);
	tex->width = width;
	tex->height = height;
	tex->type = type;
	tex->flags = imageFlags;
	glActiveTexture(GL_TEXTURE0);
	m_impl->bindTexture(tex->tex);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	if (type == TextureType::RGBA) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}

	if ((imageFlags & ImageFlags::ImageGenerateMipmaps) != 0) {
		if ((imageFlags & ImageFlags::ImageNearest) != 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
	} else {
		if ((imageFlags & ImageFlags::ImageNearest) != 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
	}

	if ((imageFlags & ImageFlags::ImageNearest) != 0) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (imageFlags & ImageFlags::ImageRepeatX) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (imageFlags & ImageFlags::ImageRepeatY) ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	if ((imageFlags & ImageFlags::ImageGenerateMipmaps) != 0) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	checkGLError(m_impl->flags, "create texture");
	m_impl->bindTexture(0);
	return tex->id;
}

bool GL3Renderer::deleteTexture(int image) {
	return m_impl->deleteTexture(image);
}

bool GL3Renderer::updateTexture(int image, int x, int y, int width, int height, const unsigned char* data) {
	Texture* tex = m_impl->findTexture(image);
	if (!tex) return false;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->tex);
	m_impl->boundTexture = tex->tex;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, y);

	if (tex->type == TextureType::RGBA) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	m_impl->boundTexture = 0;
	return true;
}

bool GL3Renderer::getTextureSize(int image, int& outWidth, int& outHeight) {
	Texture* tex = m_impl->findTexture(image);
	if (!tex) return false;
	outWidth = tex->width;
	outHeight = tex->height;
	return true;
}

int GL3Renderer::createTextureFromNativeHandle(uint64_t handle, int w, int h, int imageFlags) {
	return createImageFromHandle(static_cast<uint32_t>(handle), w, h, imageFlags | ImageFlagsGL::ImageNoDelete);
}

int GL3Renderer::createImageFromHandle(uint32_t textureId, int w, int h, int imageFlags) {
	Texture* tex = m_impl->allocTexture();
	if (!tex) return 0;
	tex->type = TextureType::RGBA;
	tex->tex = textureId;
	tex->flags = imageFlags;
	tex->width = w;
	tex->height = h;
	return tex->id;
}

uint32_t GL3Renderer::imageHandle(int image) const {
	Texture* tex = m_impl->findTexture(image);
	return tex ? tex->tex : 0;
}

void GL3Renderer::viewport(float width, float height, float devicePixelRatio) {
	(void)devicePixelRatio;
	if (m_impl->view[0] != width || m_impl->view[1] != height) {
		m_impl->view[0] = width;
		m_impl->view[1] = height;
		auto setView = [&](const GLShader& s) {
			if (s.prog != 0) {
				glUseProgram(s.prog);
				glUniform2fv(s.loc[LocViewSize], 1, m_impl->view);
			}
		};
		setView(m_impl->shader);
		setView(m_impl->stencilStrokeShader);
		setView(m_impl->simpleShader);
		setView(m_impl->solidVertShader);
		setView(m_impl->circleVertShader);
		setView(m_impl->rectVertShader);
		m_impl->activeProg = 0;
	}
}

void GL3Renderer::cancel() {
	m_impl->verts.clear();
	m_impl->paths.clear();
	m_impl->calls.clear();
	m_impl->uniforms.clear();
	m_impl->boundUniformOffset = -1;
	m_impl->lastUniformOffset = -1;
	m_impl->currentClipDepth = 0;
}

void GL3Renderer::flush() {
	if (m_impl->calls.empty()) return;

	// Hardware fast-clear promotion: if the first call is a full-viewport opaque solid quad,
	// execute hardware glClear(GL_COLOR_BUFFER_BIT) (Intel CCS_E fast-clear metadata) just like Skia!
	size_t firstCallIdx = 0;
	{
		const Call& c0 = m_impl->calls[0];
		if (c0.type == CallType::Triangles && c0.triangleCount == 6 &&
		    c0.clipDepth == 0 && c0.scissorRect[0] < 0 && c0.customShader == nullptr &&
		    (c0.blendFunc.srcRGB == GL_ONE && (c0.blendFunc.dstRGB == GL_ONE_MINUS_SRC_ALPHA || c0.blendFunc.dstRGB == GL_ZERO))) {
			const FragUniforms* f0 = m_impl->fragUniformPtr(c0.uniformOffset);
			if (f0->type == ShaderSimple && f0->innerCol.a >= 0.999f) {
				glDisable(GL_SCISSOR_TEST);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glClearColor(f0->innerCol.r, f0->innerCol.g, f0->innerCol.b, f0->innerCol.a);
				glClear(GL_COLOR_BUFFER_BIT);
				firstCallIdx = 1;
				if (m_impl->calls.size() == 1) {
					cancel();
					return;
				}
			}
		}
	}

	m_impl->activeProg = 0;
	m_impl->useProgramCached(m_impl->shader.prog);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glActiveTexture(GL_TEXTURE0);

	m_impl->boundTexture = 0;
	m_impl->stencilEnabled = false;
	m_impl->stencilMaskVal = 0xffffffff;
	m_impl->stencilFuncMode = GL_ALWAYS;
	m_impl->stencilFuncRefVal = 0;
	m_impl->stencilFuncMaskVal = 0xffffffff;
	m_impl->blendState = Blend{};
	m_impl->boundUniformOffset = -1;
	m_impl->scissorEnabled = false;
	m_impl->currentScissor[0] = -1;
	m_impl->currentScissor[1] = -1;
	m_impl->currentScissor[2] = -1;
	m_impl->currentScissor[3] = -1;

	// Stream UBO via persistent ring buffer (zero per-frame GEM allocations)
	size_t uboBytes = m_impl->uniforms.size();
	if (uboBytes > 0) {
		glBindBuffer(GL_UNIFORM_BUFFER, m_impl->fragBuf);
		if (m_impl->fragBufCap == 0 || m_impl->fragRingOffset + uboBytes > m_impl->fragBufCap) {
			size_t minCap = (524288 / m_impl->fragSize) * m_impl->fragSize;
			m_impl->fragBufCap = std::max(m_impl->fragBufCap, std::max(uboBytes * 4, minCap));
			glBufferData(GL_UNIFORM_BUFFER, m_impl->fragBufCap, nullptr, GL_STREAM_DRAW);
			m_impl->fragRingOffset = 0;
		}
		m_impl->baseUboOffset = static_cast<int>(m_impl->fragRingOffset);
		glBufferSubData(GL_UNIFORM_BUFFER, m_impl->baseUboOffset, uboBytes, m_impl->uniforms.data());
		m_impl->fragRingOffset += uboBytes;
	}

	// Stream VBO via persistent ring buffer (zero per-frame GEM allocations)
	glBindVertexArray(m_impl->vertArr);
	int nverts = static_cast<int>(m_impl->verts.size());
	size_t vertBytes = static_cast<size_t>(nverts) * sizeof(Vertex);
	if (vertBytes > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, m_impl->vertBuf);
		if (m_impl->vertBufCap == 0 || (static_cast<size_t>(m_impl->vertRingPos + nverts) * sizeof(Vertex)) > m_impl->vertBufCap) {
			m_impl->vertBufCap = std::max(m_impl->vertBufCap, std::max(vertBytes * 4, static_cast<size_t>(262144) * sizeof(Vertex)));
			glBufferData(GL_ARRAY_BUFFER, m_impl->vertBufCap, nullptr, GL_STREAM_DRAW);
			m_impl->vertRingPos = 0;
		}
		m_impl->baseVertOffset = m_impl->vertRingPos;
		glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(m_impl->baseVertOffset) * sizeof(Vertex), vertBytes, m_impl->verts.data());
		m_impl->vertRingPos += nverts;
	}

	int activeClipDepth = 0;
	for (size_t idx = firstCallIdx; idx < m_impl->calls.size(); ++idx) {
		const auto& call = m_impl->calls[idx];
		m_impl->blendFuncSeparate(call.blendFunc);
		bool needScissor = (call.scissorRect[0] >= 0);
		if (m_impl->scissorEnabled != needScissor) {
			m_impl->scissorEnabled = needScissor;
			if (needScissor) {
				glEnable(GL_SCISSOR_TEST);
			} else {
				glDisable(GL_SCISSOR_TEST);
			}
		}
		if (needScissor) {
			if (m_impl->currentScissor[0] != call.scissorRect[0] ||
			    m_impl->currentScissor[1] != call.scissorRect[1] ||
			    m_impl->currentScissor[2] != call.scissorRect[2] ||
			    m_impl->currentScissor[3] != call.scissorRect[3]) {
				m_impl->currentScissor[0] = call.scissorRect[0];
				m_impl->currentScissor[1] = call.scissorRect[1];
				m_impl->currentScissor[2] = call.scissorRect[2];
				m_impl->currentScissor[3] = call.scissorRect[3];
				glScissor(call.scissorRect[0], call.scissorRect[1], call.scissorRect[2], call.scissorRect[3]);
			}
		}
		switch (call.type) {
			case CallType::Clip:
				m_impl->clip(call, activeClipDepth);
				break;
			case CallType::RestoreClip:
				m_impl->restoreClip(call, activeClipDepth);
				break;
			case CallType::Fill:
				m_impl->fill(call, activeClipDepth);
				break;
			case CallType::ConvexFill:
				m_impl->convexFill(call, activeClipDepth);
				break;
			case CallType::Stroke:
				m_impl->stroke(call, activeClipDepth);
				break;
			case CallType::Triangles:
				m_impl->triangles(call, activeClipDepth);
				break;
			default:
				break;
		}
	}

	if (activeClipDepth > 0) {
		m_impl->enableStencil(true);
		m_impl->stencilMask(0x00);
		m_impl->stencilFunc(GL_EQUAL, activeClipDepth << 4, 0xf0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	} else if (m_impl->stencilEnabled) {
		m_impl->enableStencil(false);
	}

	if (m_impl->scissorEnabled) {
		glDisable(GL_SCISSOR_TEST);
		m_impl->scissorEnabled = false;
	}
	glDisable(GL_CULL_FACE);

	cancel();
}

void GL3Renderer::renderFill(const Paint& paint, const CompositeOperationState& compOp,
                             const Scissor& scissor, float fringe, const float* bounds,
                             const Path* paths, int npaths) {
	Blend blend = convertBlend(compOp);
	int hwScissor[4]{-1, -1, -1, -1};
	bool hasHwScissor = m_impl->getHardwareScissor(scissor, hwScissor);
	Scissor effectiveScissor = scissor;

	if (hasHwScissor && bounds != nullptr) {
		float bMinX = bounds[0] - fringe;
		float bMinY = bounds[1] - fringe;
		float bMaxX = bounds[2] + fringe;
		float bMaxY = bounds[3] + fringe;

		float sMinX = static_cast<float>(hwScissor[0]);
		float sMaxX = static_cast<float>(hwScissor[0] + hwScissor[2]);
		float sMinY = m_impl->view[1] - static_cast<float>(hwScissor[1] + hwScissor[3]);
		float sMaxY = m_impl->view[1] - static_cast<float>(hwScissor[1]);

		if (bMaxX <= sMinX || bMinX >= sMaxX || bMaxY <= sMinY || bMinY >= sMaxY) {
			return;
		}

		if (bMinX >= sMinX && bMaxX <= sMaxX && bMinY >= sMinY && bMaxY <= sMaxY) {
			hasHwScissor = false;
			hwScissor[0] = -1;
			effectiveScissor.extent[0] = -1.0f;
			effectiveScissor.extent[1] = -1.0f;
		}
	}

	CallType callType = CallType::Fill;
	int triangleCount = 4;
	if (npaths == 1 && paths[0].convex) {
		callType = CallType::ConvexFill;
		triangleCount = 0;
	}

	FragUniforms tempFrag;
	m_impl->convertPaint(&tempFrag, paint, effectiveScissor, fringe, fringe, -1.0f, hasHwScissor);

	uint32_t c = paint.innerColor.premultiplied().toRGBA8();

	int numTriVerts = 0;
	if (npaths == 1 && paths[0].convex) {
		const Path& p = paths[0];
		if (p.fillCount >= 3) numTriVerts += (p.fillCount - 2) * 3;
		if (p.strokeCount >= 3) numTriVerts += (p.strokeCount - 2) * 3;
	}

	if (numTriVerts > 0) {
		const Path& p = paths[0];
		// Try coalescing with previous Call if both are Triangles and have identical state
		if (!m_impl->calls.empty()) {
			Call& last = m_impl->calls.back();
			if (last.type == CallType::Triangles &&
			    last.image == paint.image &&
			    last.customShader == paint.customShader &&
			    last.blendFunc.srcRGB == blend.srcRGB &&
			    last.blendFunc.dstRGB == blend.dstRGB &&
			    last.blendFunc.srcAlpha == blend.srcAlpha &&
			    last.blendFunc.dstAlpha == blend.dstAlpha &&
			    last.scissorRect[0] == (hasHwScissor ? hwScissor[0] : -1) &&
			    last.scissorRect[1] == (hasHwScissor ? hwScissor[1] : -1) &&
			    last.scissorRect[2] == (hasHwScissor ? hwScissor[2] : -1) &&
			    last.scissorRect[3] == (hasHwScissor ? hwScissor[3] : -1) &&
			    last.clipDepth == m_impl->currentClipDepth) {

				FragUniforms* lastFrag = m_impl->fragUniformPtr(last.uniformOffset);
				if (paint.customShader != nullptr || std::memcmp(lastFrag, &tempFrag, sizeof(FragUniforms)) == 0) {
					int offset = static_cast<int>(m_impl->verts.size());
					m_impl->verts.resize(offset + numTriVerts);
					Vertex* dst = &m_impl->verts[offset];

					if (p.fillCount >= 3 && p.fill != nullptr) {
						const Vertex& v0 = p.fill[0];
						for (int k = 0; k < p.fillCount - 2; ++k) {
							dst[0] = v0;            dst[0].color = c;
							dst[1] = p.fill[k + 1]; dst[1].color = c;
							dst[2] = p.fill[k + 2]; dst[2].color = c;
							dst += 3;
						}
					}
					if (p.strokeCount >= 3 && p.stroke != nullptr) {
						for (int k = 0; k < p.strokeCount - 2; ++k) {
							if ((k & 1) == 0) {
								dst[0] = p.stroke[k];     dst[0].color = c;
								dst[1] = p.stroke[k + 1]; dst[1].color = c;
								dst[2] = p.stroke[k + 2]; dst[2].color = c;
							} else {
								dst[0] = p.stroke[k + 1]; dst[0].color = c;
								dst[1] = p.stroke[k];     dst[1].color = c;
								dst[2] = p.stroke[k + 2]; dst[2].color = c;
							}
							dst += 3;
						}
					}
					last.triangleCount += numTriVerts;
					return;
				}
			}
		}

		// Create a new Triangles call for this convex shape
		Call call;
		call.type = CallType::Triangles;
		call.clipDepth = m_impl->currentClipDepth;
		call.image = paint.image;
		call.customShader = paint.customShader;
		call.blendFunc = blend;
		call.triangleOffset = static_cast<int>(m_impl->verts.size());
		call.triangleCount = numTriVerts;
		if (hasHwScissor) {
			call.scissorRect[0] = hwScissor[0];
			call.scissorRect[1] = hwScissor[1];
			call.scissorRect[2] = hwScissor[2];
			call.scissorRect[3] = hwScissor[3];
		} else {
			call.scissorRect[0] = -1;
		}

		m_impl->verts.resize(call.triangleOffset + numTriVerts);
		Vertex* dst = &m_impl->verts[call.triangleOffset];
		if (p.fillCount >= 3 && p.fill != nullptr) {
			const Vertex& v0 = p.fill[0];
			for (int k = 0; k < p.fillCount - 2; ++k) {
				dst[0] = v0;            dst[0].color = c;
				dst[1] = p.fill[k + 1]; dst[1].color = c;
				dst[2] = p.fill[k + 2]; dst[2].color = c;
				dst += 3;
			}
		}
		if (p.strokeCount >= 3 && p.stroke != nullptr) {
			for (int k = 0; k < p.strokeCount - 2; ++k) {
				if ((k & 1) == 0) {
					dst[0] = p.stroke[k];     dst[0].color = c;
					dst[1] = p.stroke[k + 1]; dst[1].color = c;
					dst[2] = p.stroke[k + 2]; dst[2].color = c;
				} else {
					dst[0] = p.stroke[k + 1]; dst[0].color = c;
					dst[1] = p.stroke[k];     dst[1].color = c;
					dst[2] = p.stroke[k + 2]; dst[2].color = c;
				}
				dst += 3;
			}
		}

		if (m_impl->lastUniformOffset >= 0 && !m_impl->uniforms.empty() &&
		    std::memcmp(m_impl->fragUniformPtr(m_impl->lastUniformOffset), &tempFrag, sizeof(FragUniforms)) == 0) {
			call.uniformOffset = m_impl->lastUniformOffset;
		} else {
			call.uniformOffset = m_impl->allocFragUniforms(1);
			FragUniforms* frag = m_impl->fragUniformPtr(call.uniformOffset);
			*frag = tempFrag;
			m_impl->lastUniformOffset = call.uniformOffset;
		}

		m_impl->calls.push_back(call);
		return;
	}

	Call call;
	call.type = callType;
	call.clipDepth = m_impl->currentClipDepth;
	call.triangleCount = triangleCount;
	call.pathOffset = static_cast<int>(m_impl->paths.size());
	call.pathCount = npaths;
	call.image = paint.image;
	call.customShader = paint.customShader;
	call.blendFunc = blend;
	if (hasHwScissor) {
		call.scissorRect[0] = hwScissor[0];
		call.scissorRect[1] = hwScissor[1];
		call.scissorRect[2] = hwScissor[2];
		call.scissorRect[3] = hwScissor[3];
	} else {
		call.scissorRect[0] = -1;
	}

	m_impl->paths.resize(call.pathOffset + npaths);

	int maxverts = call.triangleCount;
	for (int i = 0; i < npaths; i++) {
		maxverts += paths[i].fillCount + paths[i].strokeCount;
	}

	int offset = static_cast<int>(m_impl->verts.size());
	m_impl->verts.resize(offset + maxverts);

	int curOffset = offset;
	for (int i = 0; i < npaths; i++) {
		const Path& path = paths[i];
		GLPath& copy = m_impl->paths[call.pathOffset + i];
		copy = GLPath{};
		if (path.fillCount > 0 && path.fill != nullptr) {
			copy.fillOffset = curOffset;
			copy.fillCount = path.fillCount;
			std::memcpy(&m_impl->verts[curOffset], path.fill, path.fillCount * sizeof(Vertex));
			for (int k = 0; k < path.fillCount; ++k) m_impl->verts[curOffset + k].color = c;
			curOffset += path.fillCount;
		}
		if (path.strokeCount > 0 && path.stroke != nullptr) {
			copy.strokeOffset = curOffset;
			copy.strokeCount = path.strokeCount;
			std::memcpy(&m_impl->verts[curOffset], path.stroke, path.strokeCount * sizeof(Vertex));
			for (int k = 0; k < path.strokeCount; ++k) m_impl->verts[curOffset + k].color = c;
			curOffset += path.strokeCount;
		}
	}

	if (call.type == CallType::Fill) {
		call.triangleOffset = curOffset;
		Vertex* quad = &m_impl->verts[curOffset];
		quad[0] = Vertex(bounds[2], bounds[3], 0.5f, 1.0f, c);
		quad[1] = Vertex(bounds[2], bounds[1], 0.5f, 1.0f, c);
		quad[2] = Vertex(bounds[0], bounds[3], 0.5f, 1.0f, c);
		quad[3] = Vertex(bounds[0], bounds[1], 0.5f, 1.0f, c);
		curOffset += 4;
	}

	if (m_impl->lastUniformOffset >= 0 && !m_impl->uniforms.empty() &&
	    std::memcmp(m_impl->fragUniformPtr(m_impl->lastUniformOffset), &tempFrag, sizeof(FragUniforms)) == 0) {
		call.uniformOffset = m_impl->lastUniformOffset;
	} else {
		call.uniformOffset = m_impl->allocFragUniforms(1);
		FragUniforms* fragFill = m_impl->fragUniformPtr(call.uniformOffset);
		*fragFill = tempFrag;
		m_impl->lastUniformOffset = call.uniformOffset;
	}

	m_impl->verts.resize(curOffset);
	m_impl->calls.push_back(call);
}

void GL3Renderer::renderStroke(const Paint& paint, const CompositeOperationState& compOp,
                              const Scissor& scissor, float fringe, float strokeWidth,
                              const Path* paths, int npaths) {
	Blend blend = convertBlend(compOp);
	int hwScissor[4]{-1, -1, -1, -1};
	bool hasHwScissor = m_impl->getHardwareScissor(scissor, hwScissor);

	bool useStencil = ((m_impl->flags & CreateFlags::StencilStrokes) != 0);
	if (npaths == 1 && paths[0].convex) {
		useStencil = false; // Convex paths cannot self-intersect, bypass stencil overhead!
	}
	if (strokeWidth <= fringe * 1.5f || (paint.innerColor.a >= 0.999f && paint.outerColor.a >= 0.999f)) {
		useStencil = false; // Hairlines and fully opaque strokes do not produce semi-transparent overlap artifacts!
	}

	FragUniforms tempFrag;
	m_impl->convertPaint(&tempFrag, paint, scissor, strokeWidth, fringe, -1.0f, hasHwScissor);

	uint32_t c = paint.innerColor.premultiplied().toRGBA8();

	int numTriVerts = 0;
	if (!useStencil) {
		for (int i = 0; i < npaths; ++i) {
			if (paths[i].strokeCount >= 3) {
				numTriVerts += (paths[i].strokeCount - 2) * 3;
			}
		}
	}

	if (numTriVerts > 0) {
		// Try coalescing with previous Call if both are Triangles and have identical state
		if (!m_impl->calls.empty()) {
			Call& last = m_impl->calls.back();
			if (last.type == CallType::Triangles &&
			    last.image == paint.image &&
			    last.customShader == paint.customShader &&
			    last.blendFunc.srcRGB == blend.srcRGB &&
			    last.blendFunc.dstRGB == blend.dstRGB &&
			    last.blendFunc.srcAlpha == blend.srcAlpha &&
			    last.blendFunc.dstAlpha == blend.dstAlpha &&
			    last.scissorRect[0] == (hasHwScissor ? hwScissor[0] : -1) &&
			    last.scissorRect[1] == (hasHwScissor ? hwScissor[1] : -1) &&
			    last.scissorRect[2] == (hasHwScissor ? hwScissor[2] : -1) &&
			    last.scissorRect[3] == (hasHwScissor ? hwScissor[3] : -1) &&
			    last.clipDepth == m_impl->currentClipDepth) {

				FragUniforms* lastFrag = m_impl->fragUniformPtr(last.uniformOffset);
				if (paint.customShader != nullptr || std::memcmp(lastFrag, &tempFrag, sizeof(FragUniforms)) == 0) {
					int offset = static_cast<int>(m_impl->verts.size());
					m_impl->verts.resize(offset + numTriVerts);
					Vertex* dst = &m_impl->verts[offset];

					for (int i = 0; i < npaths; ++i) {
						const Path& p = paths[i];
						if (p.strokeCount >= 3 && p.stroke != nullptr) {
							for (int k = 0; k < p.strokeCount - 2; ++k) {
								if ((k & 1) == 0) {
									dst[0] = p.stroke[k];     dst[0].color = c;
									dst[1] = p.stroke[k + 1]; dst[1].color = c;
									dst[2] = p.stroke[k + 2]; dst[2].color = c;
								} else {
									dst[0] = p.stroke[k + 1]; dst[0].color = c;
									dst[1] = p.stroke[k];     dst[1].color = c;
									dst[2] = p.stroke[k + 2]; dst[2].color = c;
								}
								dst += 3;
							}
						}
					}
					last.triangleCount += numTriVerts;
					return;
				}
			}
		}

		// Create a new Triangles call for this stroke
		Call call;
		call.type = CallType::Triangles;
		call.clipDepth = m_impl->currentClipDepth;
		call.image = paint.image;
		call.customShader = paint.customShader;
		call.blendFunc = blend;
		call.triangleOffset = static_cast<int>(m_impl->verts.size());
		call.triangleCount = numTriVerts;
		if (hasHwScissor) {
			call.scissorRect[0] = hwScissor[0];
			call.scissorRect[1] = hwScissor[1];
			call.scissorRect[2] = hwScissor[2];
			call.scissorRect[3] = hwScissor[3];
		} else {
			call.scissorRect[0] = -1;
		}

		m_impl->verts.resize(call.triangleOffset + numTriVerts);
		Vertex* dst = &m_impl->verts[call.triangleOffset];
		for (int i = 0; i < npaths; ++i) {
			const Path& p = paths[i];
			if (p.strokeCount >= 3 && p.stroke != nullptr) {
				for (int k = 0; k < p.strokeCount - 2; ++k) {
					if ((k & 1) == 0) {
						dst[0] = p.stroke[k];     dst[0].color = c;
						dst[1] = p.stroke[k + 1]; dst[1].color = c;
						dst[2] = p.stroke[k + 2]; dst[2].color = c;
					} else {
						dst[0] = p.stroke[k + 1]; dst[0].color = c;
						dst[1] = p.stroke[k];     dst[1].color = c;
						dst[2] = p.stroke[k + 2]; dst[2].color = c;
					}
					dst += 3;
				}
			}
		}

		if (m_impl->lastUniformOffset >= 0 && !m_impl->uniforms.empty() &&
		    std::memcmp(m_impl->fragUniformPtr(m_impl->lastUniformOffset), &tempFrag, sizeof(FragUniforms)) == 0) {
			call.uniformOffset = m_impl->lastUniformOffset;
		} else {
			call.uniformOffset = m_impl->allocFragUniforms(1);
			FragUniforms* frag = m_impl->fragUniformPtr(call.uniformOffset);
			*frag = tempFrag;
			m_impl->lastUniformOffset = call.uniformOffset;
		}

		m_impl->calls.push_back(call);
		return;
	}

	Call call;
	call.type = CallType::Stroke;
	call.clipDepth = m_impl->currentClipDepth;
	call.pathOffset = static_cast<int>(m_impl->paths.size());
	call.pathCount = npaths;
	call.image = paint.image;
	call.customShader = paint.customShader;
	call.blendFunc = blend;
	call.useStencil = useStencil;
	if (hasHwScissor) {
		call.scissorRect[0] = hwScissor[0];
		call.scissorRect[1] = hwScissor[1];
		call.scissorRect[2] = hwScissor[2];
		call.scissorRect[3] = hwScissor[3];
	} else {
		call.scissorRect[0] = -1;
	}

	m_impl->paths.resize(call.pathOffset + npaths);

	int maxverts = 0;
	for (int i = 0; i < npaths; i++) {
		maxverts += paths[i].strokeCount;
	}

	int offset = static_cast<int>(m_impl->verts.size());
	m_impl->verts.resize(offset + maxverts);

	int curOffset = offset;
	for (int i = 0; i < npaths; i++) {
		const Path& path = paths[i];
		GLPath& copy = m_impl->paths[call.pathOffset + i];
		copy = GLPath{};
		if (path.strokeCount > 0 && path.stroke != nullptr) {
			copy.strokeOffset = curOffset;
			copy.strokeCount = path.strokeCount;
			std::memcpy(&m_impl->verts[curOffset], path.stroke, path.strokeCount * sizeof(Vertex));
			for (int k = 0; k < path.strokeCount; ++k) m_impl->verts[curOffset + k].color = c;
			curOffset += path.strokeCount;
		}
	}

	if (useStencil) {
		call.uniformOffset = m_impl->allocFragUniforms(2);
		FragUniforms* fragBase = m_impl->fragUniformPtr(call.uniformOffset);
		*fragBase = tempFrag;

		FragUniforms* fragOverlap = m_impl->fragUniformPtr(call.uniformOffset + m_impl->fragSize);
		m_impl->convertPaint(fragOverlap, paint, scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f, hasHwScissor);
	} else {
		if (m_impl->lastUniformOffset >= 0 && !m_impl->uniforms.empty() &&
		    std::memcmp(m_impl->fragUniformPtr(m_impl->lastUniformOffset), &tempFrag, sizeof(FragUniforms)) == 0) {
			call.uniformOffset = m_impl->lastUniformOffset;
		} else {
			call.uniformOffset = m_impl->allocFragUniforms(1);
			FragUniforms* fragBase = m_impl->fragUniformPtr(call.uniformOffset);
			*fragBase = tempFrag;
			m_impl->lastUniformOffset = call.uniformOffset;
		}
	}

	m_impl->verts.resize(curOffset);
	m_impl->calls.push_back(call);
}

void GL3Renderer::renderTriangles(const Paint& paint, const CompositeOperationState& compOp,
                                 const Scissor& scissor, const Vertex* verts, int nverts,
                                 float fringe, int shaderType) {
	Blend blend = convertBlend(compOp);
	int hwScissor[4]{-1, -1, -1, -1};
	bool hasHwScissor = m_impl->getHardwareScissor(scissor, hwScissor);

	FragUniforms tempFrag;
	float widthForMult = (shaderType == -2) ? paint.feather : 1.0f;
	m_impl->convertPaint(&tempFrag, paint, scissor, widthForMult, fringe, -1.0f, hasHwScissor);
	if (shaderType == -2) {
		tempFrag.type = ShaderSolid;
		tempFrag.radius = 0.0f;
		tempFrag.feather = 1.0f;
		tempFrag.innerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
		tempFrag.outerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
	} else if (shaderType == 2) {
		tempFrag.type = ShaderSimple;
		tempFrag.innerCol = paint.innerColor.premultiplied();
		tempFrag.outerCol = tempFrag.innerCol;
	} else if (shaderType == 15) {
		// Concentric Circle Radial Gradient (ftcoord in [-Rq, Rq], circle radius in paint.xform[0])
		tempFrag.type = ShaderRadialGrad;
		tempFrag.strokeMult = 4.0f;
		tempFrag.strokeThr = paint.xform[0];
		std::memset(tempFrag.paintMat, 0, sizeof(tempFrag.paintMat));
	} else if (shaderType == 12) {
		// Vertex-interpolated Radial Gradient (ftcoord = pt)
		tempFrag.type = ShaderRadialGrad;
		tempFrag.strokeMult = 3.0f;
		std::memset(tempFrag.paintMat, 0, sizeof(tempFrag.paintMat));
	} else if (shaderType == 13) {
		// Vertex-interpolated Box Gradient (ftcoord = pt)
		tempFrag.type = ShaderFillGrad;
		tempFrag.strokeMult = 3.0f;
		std::memset(tempFrag.paintMat, 0, sizeof(tempFrag.paintMat));
	} else if (shaderType == 14) {
		// Vertex-interpolated Linear Gradient (ftcoord.y = pt.y)
		tempFrag.type = ShaderLinearGrad;
		tempFrag.radius = 1.0f;
		tempFrag.feather = 1.0f;
		std::memset(tempFrag.paintMat, 0, sizeof(tempFrag.paintMat));
	} else if (shaderType >= 0) {
		tempFrag.type = shaderType;
	}

	if (shaderType == 3) {
		if (tempFrag.texType == 0 && paint.image != 0) {
			Texture* tex = m_impl->findTexture(paint.image);
			if (tex && tex->type == TextureType::Alpha) {
				tempFrag.texType = 2;
			}
		}
		std::memset(tempFrag.paintMat, 0, sizeof(tempFrag.paintMat));
		tempFrag.extent[0] = 0.0f;
		tempFrag.extent[1] = 0.0f;
		tempFrag.radius = 0.0f;
		tempFrag.feather = 1.0f;
		if (paint.radius > 0.5f) {
			tempFrag.innerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
			tempFrag.outerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
		}
	} else if (shaderType == 7 || shaderType == 8) {
		tempFrag.innerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
		tempFrag.outerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
	} else if (shaderType == 9) {
		tempFrag.extent[0] = paint.extent[0];
		tempFrag.extent[1] = paint.extent[1];
		tempFrag.radius = paint.radius;
		tempFrag.feather = paint.feather;
		tempFrag.strokeMult = paint.xform[0];
		tempFrag.strokeThr = paint.xform[1];
		tempFrag.innerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
		tempFrag.outerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
		std::memset(tempFrag.paintMat, 0, sizeof(tempFrag.paintMat));
	} else if (shaderType == 10) {
		tempFrag.extent[0] = paint.extent[0];
		tempFrag.extent[1] = paint.extent[1];
		tempFrag.radius = paint.radius;
		tempFrag.strokeThr = paint.feather;
		tempFrag.innerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
		tempFrag.outerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
	} else if (shaderType == 11) {
		tempFrag.extent[0] = paint.extent[0];
		tempFrag.extent[1] = paint.radius;
		tempFrag.radius = paint.feather;
		tempFrag.feather = paint.feather;
		tempFrag.strokeMult = paint.feather;
		tempFrag.strokeThr = paint.feather;
	} else if (shaderType == 6) {
		tempFrag.strokeMult = 2.0f;
	} else if (shaderType == 1) {
		if (paint.radius > 0.0f) {
			tempFrag.radius = paint.radius;
			tempFrag.extent[0] = paint.feather;
			tempFrag.extent[1] = (paint.outerColor.r > 0.0f) ? paint.outerColor.r : paint.feather;
			tempFrag.outerCol = Color(1.0f, 1.0f, 1.0f, 1.0f);
			tempFrag.feather = 1.0f;
			tempFrag.strokeMult = 1.0f;
		}
	}

	if (!m_impl->calls.empty()) {
		Call& last = m_impl->calls.back();
		if (last.type == CallType::Triangles &&
		    last.image == paint.image &&
		    last.customShader == paint.customShader &&
		    last.blendFunc.srcRGB == blend.srcRGB &&
		    last.blendFunc.dstRGB == blend.dstRGB &&
		    last.blendFunc.srcAlpha == blend.srcAlpha &&
		    last.blendFunc.dstAlpha == blend.dstAlpha &&
		    last.scissorRect[0] == (hasHwScissor ? hwScissor[0] : -1) &&
		    last.scissorRect[1] == (hasHwScissor ? hwScissor[1] : -1) &&
		    last.scissorRect[2] == (hasHwScissor ? hwScissor[2] : -1) &&
		    last.scissorRect[3] == (hasHwScissor ? hwScissor[3] : -1) &&
		    last.clipDepth == m_impl->currentClipDepth) {

			FragUniforms* lastFrag = m_impl->fragUniformPtr(last.uniformOffset);
			if (paint.customShader != nullptr || std::memcmp(lastFrag, &tempFrag, sizeof(FragUniforms)) == 0) {
				int offset = static_cast<int>(m_impl->verts.size());
				m_impl->verts.resize(offset + nverts);
				std::memcpy(&m_impl->verts[offset], verts, nverts * sizeof(Vertex));
				last.triangleCount += nverts;
				return;
			}
		}
	}

	Call call;
	call.type = CallType::Triangles;
	call.clipDepth = m_impl->currentClipDepth;
	call.image = paint.image;
	call.customShader = paint.customShader;
	call.blendFunc = blend;
	call.triangleOffset = static_cast<int>(m_impl->verts.size());
	call.triangleCount = nverts;
	if (hasHwScissor) {
		call.scissorRect[0] = hwScissor[0];
		call.scissorRect[1] = hwScissor[1];
		call.scissorRect[2] = hwScissor[2];
		call.scissorRect[3] = hwScissor[3];
	} else {
		call.scissorRect[0] = -1;
	}

	m_impl->verts.resize(call.triangleOffset + nverts);
	std::memcpy(&m_impl->verts[call.triangleOffset], verts, nverts * sizeof(Vertex));

	if (m_impl->lastUniformOffset >= 0 && !m_impl->uniforms.empty() &&
	    std::memcmp(m_impl->fragUniformPtr(m_impl->lastUniformOffset), &tempFrag, sizeof(FragUniforms)) == 0) {
		call.uniformOffset = m_impl->lastUniformOffset;
	} else {
		call.uniformOffset = m_impl->allocFragUniforms(1);
		FragUniforms* frag = m_impl->fragUniformPtr(call.uniformOffset);
		*frag = tempFrag;
		m_impl->lastUniformOffset = call.uniformOffset;
	}

	m_impl->calls.push_back(call);
}

void GL3Renderer::renderClip(const RenderPath* paths, int npaths, Winding winding, int targetDepth,
                             const Scissor& scissor, const float* bounds) {
	if (npaths <= 0 || paths == nullptr || m_impl->fragSize <= 0) return;

	int hwScissor[4]{-1, -1, -1, -1};
	bool hasHwScissor = m_impl->getHardwareScissor(scissor, hwScissor);

	Call call;
	call.type = CallType::Clip;
	call.clipDepth = targetDepth;
	call.winding = winding;
	call.blendFunc = Blend{GL_ONE, GL_ZERO, GL_ONE, GL_ZERO};
	if (hasHwScissor) {
		call.scissorRect[0] = hwScissor[0];
		call.scissorRect[1] = hwScissor[1];
		call.scissorRect[2] = hwScissor[2];
		call.scissorRect[3] = hwScissor[3];
	} else {
		call.scissorRect[0] = -1;
	}

	call.pathOffset = static_cast<int>(m_impl->paths.size());
	call.pathCount = npaths;
	m_impl->paths.resize(call.pathOffset + npaths);

	int maxverts = 4; // cover quad
	for (int i = 0; i < npaths; i++) {
		maxverts += paths[i].fillCount;
	}

	int offset = static_cast<int>(m_impl->verts.size());
	m_impl->verts.resize(offset + maxverts);

	int curOffset = offset;
	for (int i = 0; i < npaths; i++) {
		const RenderPath& path = paths[i];
		GLPath& copy = m_impl->paths[call.pathOffset + i];
		copy = GLPath{};
		if (path.fillCount > 0 && path.fill != nullptr) {
			copy.fillOffset = curOffset;
			copy.fillCount = path.fillCount;
			std::memcpy(&m_impl->verts[curOffset], path.fill, path.fillCount * sizeof(Vertex));
			curOffset += path.fillCount;
		}
	}

	call.triangleOffset = curOffset;
	call.triangleCount = 4;
	Vertex* quad = &m_impl->verts[curOffset];
	float minX = bounds ? bounds[0] : 0.0f;
	float minY = bounds ? bounds[1] : 0.0f;
	float maxX = bounds ? bounds[2] : m_impl->view[0];
	float maxY = bounds ? bounds[3] : m_impl->view[1];

	quad[0] = Vertex(maxX, maxY, 0.5f, 1.0f);
	quad[1] = Vertex(maxX, minY, 0.5f, 1.0f);
	quad[2] = Vertex(minX, maxY, 0.5f, 1.0f);
	quad[3] = Vertex(minX, minY, 0.5f, 1.0f);
	curOffset += 4;

	call.uniformOffset = m_impl->allocFragUniforms(1);
	FragUniforms* frag = m_impl->fragUniformPtr(call.uniformOffset);
	*frag = FragUniforms{};
	frag->strokeThr = -1.0f;
	frag->type = ShaderSimple;

	m_impl->verts.resize(curOffset);
	m_impl->calls.push_back(call);
	m_impl->currentClipDepth = targetDepth;
}

void GL3Renderer::restoreClip(int targetDepth) {
	if (m_impl->fragSize <= 0) return;

	Call call;
	call.type = CallType::RestoreClip;
	call.clipDepth = targetDepth;
	call.blendFunc = Blend{GL_ONE, GL_ZERO, GL_ONE, GL_ZERO};
	call.scissorRect[0] = -1;

	int offset = static_cast<int>(m_impl->verts.size());
	m_impl->verts.resize(offset + 4);

	Vertex* quad = &m_impl->verts[offset];
	quad[0] = Vertex(m_impl->view[0], m_impl->view[1], 0.5f, 1.0f);
	quad[1] = Vertex(m_impl->view[0], 0.0f, 0.5f, 1.0f);
	quad[2] = Vertex(0.0f, m_impl->view[1], 0.5f, 1.0f);
	quad[3] = Vertex(0.0f, 0.0f, 0.5f, 1.0f);

	call.triangleOffset = offset;
	call.triangleCount = 4;

	call.uniformOffset = m_impl->allocFragUniforms(1);
	FragUniforms* frag = m_impl->fragUniformPtr(call.uniformOffset);
	*frag = FragUniforms{};
	frag->strokeThr = -1.0f;
	frag->type = ShaderSimple;

	m_impl->calls.push_back(call);
	m_impl->currentClipDepth = targetDepth;
}

void GL3Renderer::resetClip() {
	restoreClip(0);
}

std::unique_ptr<Context> createContextGL3(int flags) {
	auto renderer = std::make_unique<GL3Renderer>(flags);
	return std::make_unique<Context>(std::move(renderer));
}

} // namespace nisaba::gpu
