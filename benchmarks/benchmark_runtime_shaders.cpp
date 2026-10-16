//
// Nisaba vs Google Skia (Ganesh GL) — Sovereign Runtime Shader Micro-Benchmark
// 100% Headless Offscreen • 1:1 Parity • 1080x720 Framebuffer • 0x MSAA
//

#include <GL/glew.h>
#include <GL/gl.h>
#include <dlfcn.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cmath>
#include <sstream>
#include <fstream>
#include <memory>
#include <string>
#include <cstring>
#include <algorithm>

// Nisaba Engine Headers
#include "nisaba/nisaba.hpp"
#include "nisaba/gpu/gpu_device.hpp"
#include "nisaba/gpu/gpu_surface.hpp"
#include "nisaba/gpu/gpu_canvas.hpp"
#include "nisaba/gpu/runtime_effect.hpp"
#include "nisaba/gpu/context.hpp"

// Skia Headers
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#define SK_GL 1
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLAssembleInterface.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

struct ShaderStat {
    std::string name;
    std::string category;
    double nisaba_cpu_us{0.0};
    double nisaba_gpu_us{0.0};
    double nisaba_total_us{0.0};
    double skia_cpu_us{0.0};
    double skia_gpu_us{0.0};
    double skia_total_us{0.0};
    bool is_gpu_draw{true};
};

// Shaders definitions
const std::string kSksl_SimpleGradient = R"(
    uniform float time;
    uniform vec2 resolution;
    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        return vec4(uv.x, uv.y, 0.5 + 0.5 * sin(time), 1.0);
    }
)";

const std::string kSksl_CyberPlasma = R"(
    uniform float time;
    uniform vec2 resolution;
    uniform vec4 colorTint;
    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float v1 = sin(uv.x * 10.0 + time);
        float v2 = sin(uv.y * 10.0 + time * 0.5);
        float v3 = sin((uv.x + uv.y) * 10.0 + time);
        float v = 0.5 + 0.5 * sin(v1 + v2 + v3);
        return vec4(v * colorTint.r, v * colorTint.g, v * colorTint.b, 1.0);
    }
)";

const std::string kSksl_FractalNoise = R"(
    uniform float time;
    uniform vec2 resolution;
    float hash(vec2 p) {
        return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
    }
    float noise(vec2 p) {
        vec2 i = floor(p);
        vec2 f = fract(p);
        vec2 u = f * f * (3.0 - 2.0 * f);
        return mix(mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
                   mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
    }
    float fbm(vec2 p) {
        float v = 0.0;
        float a = 0.5;
        for (int i = 0; i < 4; ++i) {
            v += a * noise(p);
            p = p * 2.0 + vec2(100.0);
            a *= 0.5;
        }
        return v;
    }
    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float n = fbm(uv * 6.0 + vec2(time * 0.2, time * 0.1));
        return vec4(n * 0.2, n * 0.6, n, 1.0);
    }
)";

const std::string kSksl_Voronoi = R"(
    uniform float time;
    uniform vec2 resolution;
    vec2 hash2(vec2 p) {
        return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
    }
    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution * 6.0;
        vec2 n = floor(uv);
        vec2 f = fract(uv);
        float m_dist = 1.0;
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                vec2 neighbor = vec2(float(x), float(y));
                vec2 point = hash2(n + neighbor);
                point = 0.5 + 0.5 * sin(time + 6.2831 * point);
                vec2 diff = neighbor + point - f;
                float dist = length(diff);
                m_dist = min(m_dist, dist);
            }
        }
        vec3 col = vec3(m_dist) * vec3(0.2, 0.7, 0.9);
        return vec4(col, 1.0);
    }
)";

const std::string kSksl_RaymarchSDF = R"(
    uniform float time;
    uniform vec2 resolution;
    uniform vec3 lightDir;
    float sphereSDF(vec3 p, float r) {
        return length(p) - r;
    }
    float sceneSDF(vec3 p) {
        return sphereSDF(p, 1.0);
    }
    vec3 calcNormal(vec3 p) {
        const float h = 0.001;
        return normalize(vec3(
            sceneSDF(p + vec3(h, 0.0, 0.0)) - sceneSDF(p - vec3(h, 0.0, 0.0)),
            sceneSDF(p + vec3(0.0, h, 0.0)) - sceneSDF(p - vec3(0.0, h, 0.0)),
            sceneSDF(p + vec3(0.0, 0.0, h)) - sceneSDF(p - vec3(0.0, 0.0, h))
        ));
    }
    vec4 main(vec2 fragCoord) {
        vec2 uv = (fragCoord - 0.5 * resolution) / resolution.y;
        vec3 ro = vec3(0.0, 0.0, 2.5);
        vec3 rd = normalize(vec3(uv, -1.0));
        float t = 0.0;
        for (int i = 0; i < 32; ++i) {
            vec3 p = ro + t * rd;
            float d = sceneSDF(p);
            if (d < 0.001) {
                vec3 n = calcNormal(p);
                float diff = max(dot(n, normalize(lightDir)), 0.0);
                vec3 refl = reflect(-normalize(lightDir), n);
                float spec = pow(max(dot(refl, -rd), 0.0), 16.0);
                vec3 col = vec3(0.1, 0.3, 0.8) * diff + vec3(spec);
                return vec4(col, 1.0);
            }
            t += d;
            if (t > 10.0) break;
        }
        return vec4(0.05, 0.05, 0.08, 1.0);
    }
)";

const std::string kSksl_FrostedVignette = R"(
    uniform float time;
    uniform vec2 resolution;
    uniform float intensity;
    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        vec2 distFromCenter = uv - 0.5;
        float dist = length(distFromCenter);
        vec2 offset = distFromCenter * dist * intensity;
        float r = sin((uv.x + offset.x) * 12.0 + time);
        float g = sin(uv.x * 12.0 + time);
        float b = sin((uv.x - offset.x) * 12.0 + time);
        float vignette = 1.0 - smoothstep(0.3, 0.7, dist);
        return vec4(0.5 + 0.5 * vec3(r, g, b), 1.0) * vignette;
    }
)";

const std::string kSksl_MultiUniformHUD = R"(
    uniform float time;
    uniform vec2 resolution;
    uniform vec4 baseColor;
    uniform vec4 accentColor;
    uniform float speed;
    uniform float scale;
    uniform int segments;
    uniform float threshold;
    uniform vec2 offsetPos;
    uniform float glowRadius;
    vec4 main(vec2 fragCoord) {
        vec2 p = (fragCoord - offsetPos) / resolution.y * scale;
        float r = length(p);
        float a = atan(p.y, p.x);
        float sector = floor((a + 3.14159) / (6.28318 / float(segments)));
        float sweep = sin(sector + time * speed);
        vec4 col = (sweep > threshold) ? accentColor : baseColor;
        float ring = abs(r - glowRadius);
        col += vec4(0.1 / (ring + 0.02));
        return col;
    }
)";

} // namespace

int main(int argc, char** argv) {
    int iterations = 50;
    std::string json_output_path = "";

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--iters") == 0 && i + 1 < argc) {
            iterations = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--json") == 0 && i + 1 < argc) {
            json_output_path = argv[++i];
        }
    }

    std::cout << "=========================================================================================================\n";
    std::cout << "         NISABA vs GOOGLE SKIA — 27-SUITE RUNTIME SHADER BENCHMARK & BOTTLENECK PROFILER                \n";
    std::cout << "               (100% Headless Offscreen • 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                   \n";
    std::cout << "=========================================================================================================\n\n";

    const int W = 1080;
    const int H = 720;

    // 1. Initialize Headless Hardware Context via Nisaba GpuDevice
    auto device = nisaba::gpu::GpuDevice::create_headless(W, H);
    if (!device || !device->is_valid()) {
        std::cerr << "Fatal: Failed to create headless GPU device context!\n";
        return 1;
    }
    device->make_current();

    std::cout << "GPU Driver   : " << device->renderer_name() << "\n";
    std::cout << "OpenGL Core  : " << device->version_name() << "\n";
    std::cout << "Iterations   : " << iterations << " runs per draw suite\n";
    std::cout << "Target Size  : " << W << "x" << H << " Offscreen FBO (0x MSAA)\n";
    std::cout << "Mode         : 100% Headless Offscreen (Zero Windowing Overhead)\n\n";

    // 2. Initialize Nisaba GPU Surface and Context
    auto nisabaSurface = nisaba::gpu::GpuSurface::create(device, W, H, 0);
    if (!nisabaSurface) {
        std::cerr << "Fatal: Failed to create Nisaba GpuSurface!\n";
        return 1;
    }
    nisaba::gpu::GpuCanvas nisabaCanvas(nisabaSurface);
    auto nisabaCtx = nisabaSurface->create_context();
    if (!nisabaCtx) {
        std::cerr << "Fatal: Failed to create Nisaba GPU Context!\n";
        return 1;
    }

    // 3. Initialize Skia GrDirectContext and SkSurface on the SAME headless EGL context
    auto glInterface = GrGLMakeAssembledInterface(nullptr, [](void*, const char* name) -> GrGLFuncPtr {
        if (void* proc = dlsym(RTLD_DEFAULT, name)) {
            return reinterpret_cast<GrGLFuncPtr>(proc);
        }
        static void* s_libGL = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
        if (s_libGL) {
            if (void* proc = dlsym(s_libGL, name)) return reinterpret_cast<GrGLFuncPtr>(proc);
        }
        return nullptr;
    });

    if (!glInterface) {
        std::cerr << "Fatal: Failed to assemble Skia GrGLInterface\n";
        return 1;
    }

    auto dContext = GrDirectContext::MakeGL(glInterface);
    if (!dContext) {
        std::cerr << "Fatal: Failed to create Skia GrDirectContext\n";
        return 1;
    }

    GrGLFramebufferInfo fbInfo;
    fbInfo.fFBOID = nisabaSurface->fbo_id();
    fbInfo.fFormat = GL_RGBA8;
    GrBackendRenderTarget backendRT(W, H, 0, 8, fbInfo);

    auto skiaSurface = SkSurface::MakeFromBackendRenderTarget(
        dContext.get(), backendRT, kTopLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType, nullptr, nullptr
    );
    if (!skiaSurface) {
        std::cerr << "Fatal: Failed to create Skia GPU Surface via MakeFromBackendRenderTarget\n";
        return 1;
    }
    auto skiaCanvas = skiaSurface->getCanvas();

    std::vector<ShaderStat> stats;

    // =========================================================================
    // PART 1: Cold Pipeline — Shader Compilation & Program Linking Benchmarks
    // =========================================================================
    std::cout << ">>> Running Part 1: Shader Compilation & Program Linking (Cold Pipeline)...\n";

    auto benchmark_compilation = [&](const std::string& name, const std::string& sksl) {
        const int comp_iters = 10;
        double n_total = 0.0;
        double s_total = 0.0;

        for (int i = 0; i < comp_iters; ++i) {
            // Measure Nisaba RuntimeEffect compilation
            auto t0 = std::chrono::high_resolution_clock::now();
            auto n_res = nisaba::gpu::RuntimeEffect::MakeForShader(sksl);
            auto t1 = std::chrono::high_resolution_clock::now();
            if (!n_res) {
                std::cerr << "Nisaba compile failed for " << name << ": " << n_res.errorText << "\n";
            }
            n_total += std::chrono::duration<double, std::micro>(t1 - t0).count();

            // Measure Skia SkRuntimeEffect compilation
            auto t2 = std::chrono::high_resolution_clock::now();
            auto s_res = SkRuntimeEffect::MakeForShader(SkString(sksl));
            auto t3 = std::chrono::high_resolution_clock::now();
            if (!s_res.effect) {
                std::cerr << "Skia compile failed for " << name << ": " << s_res.errorText.c_str() << "\n";
            }
            s_total += std::chrono::duration<double, std::micro>(t3 - t2).count();
        }

        ShaderStat st;
        st.name = name;
        st.category = "Compilation";
        st.nisaba_cpu_us = n_total / comp_iters;
        st.nisaba_gpu_us = 0.0;
        st.nisaba_total_us = st.nisaba_cpu_us;
        st.skia_cpu_us = s_total / comp_iters;
        st.skia_gpu_us = 0.0;
        st.skia_total_us = st.skia_cpu_us;
        st.is_gpu_draw = false;
        stats.push_back(st);
    };

    benchmark_compilation("1. Compile: Simple Gradient (2 Uniforms)", kSksl_SimpleGradient);
    benchmark_compilation("2. Compile: Cyber Plasma Wave (3 Uniforms)", kSksl_CyberPlasma);
    benchmark_compilation("3. Compile: Fractal Noise FBM (2 Uniforms, Loop)", kSksl_FractalNoise);
    benchmark_compilation("4. Compile: Voronoi Cells (2 Uniforms, Nested 2D)", kSksl_Voronoi);
    benchmark_compilation("5. Compile: Raymarched 3D SDF (3 Uniforms, 32-Step)", kSksl_RaymarchSDF);
    benchmark_compilation("6. Compile: Frosted Vignette (3 Uniforms)", kSksl_FrostedVignette);
    benchmark_compilation("7. Compile: Multi-Uniform HUD (10 Uniforms)", kSksl_MultiUniformHUD);

    // =========================================================================
    // PART 2: CPU Builder & Uniform Data Marshalling Overhead
    // =========================================================================
    std::cout << ">>> Running Part 2: Uniform Data Marshalling & Builder Overhead...\n";

    // Pre-compile HUD effect for uniform tests
    auto n_hud_effect = nisaba::gpu::RuntimeEffect::MakeForShader(kSksl_MultiUniformHUD).effect;
    auto s_hud_effect = SkRuntimeEffect::MakeForShader(SkString(kSksl_MultiUniformHUD)).effect;

    auto benchmark_uniform_updates = [&](const std::string& name, auto op_nisaba, auto op_skia) {
        const int u_iters = 1000;

        auto t0 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < u_iters; ++i) {
            op_nisaba(i);
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        auto t2 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < u_iters; ++i) {
            op_skia(i);
        }
        auto t3 = std::chrono::high_resolution_clock::now();

        ShaderStat st;
        st.name = name;
        st.category = "Uniforms";
        st.nisaba_cpu_us = std::chrono::duration<double, std::micro>(t1 - t0).count() / u_iters;
        st.nisaba_gpu_us = 0.0;
        st.nisaba_total_us = st.nisaba_cpu_us;
        st.skia_cpu_us = std::chrono::duration<double, std::micro>(t3 - t2).count() / u_iters;
        st.skia_gpu_us = 0.0;
        st.skia_total_us = st.skia_cpu_us;
        st.is_gpu_draw = false;
        stats.push_back(st);
    };

    benchmark_uniform_updates("8. Uniform Set: Single Float (1,000 iters)",
        [&](int i) {
            nisaba::gpu::RuntimeShaderBuilder b(n_hud_effect);
            b.uniform("time") = float(i) * 0.016f;
        },
        [&](int i) {
            SkRuntimeShaderBuilder b(s_hud_effect);
            b.uniform("time") = float(i) * 0.016f;
        }
    );

    benchmark_uniform_updates("9. Uniform Set: Vec2 + Vec4 Colors (1,000 iters)",
        [&](int i) {
            nisaba::gpu::RuntimeShaderBuilder b(n_hud_effect);
            float res[2] = {static_cast<float>(W + i), static_cast<float>(H)};
            float col[4] = {0.2f, 0.4f, 0.8f, 1.0f};
            b.uniform("resolution").set(res, 2);
            b.uniform("baseColor").set(col, 4);
        },
        [&](int i) {
            SkRuntimeShaderBuilder b(s_hud_effect);
            b.uniform("resolution") = SkV2{static_cast<float>(W + i), static_cast<float>(H)};
            b.uniform("baseColor") = SkV4{0.2f, 0.4f, 0.8f, 1.0f};
        }
    );

    benchmark_uniform_updates("10. Uniform Set: 10 Mixed Uniforms Batch (1,000 iters)",
        [&](int i) {
            nisaba::gpu::RuntimeShaderBuilder b(n_hud_effect);
            b.uniform("time") = float(i) * 0.01f;
            float r[2] = {static_cast<float>(W), static_cast<float>(H)};
            float c1[4] = {0.1f, 0.2f, 0.3f, 1.0f};
            float c2[4] = {0.9f, 0.8f, 0.1f, 1.0f};
            float off[2] = {100.0f, 100.0f};
            b.uniform("resolution").set(r, 2);
            b.uniform("baseColor").set(c1, 4);
            b.uniform("accentColor").set(c2, 4);
            b.uniform("speed") = 1.5f;
            b.uniform("scale") = 1.0f;
            b.uniform("segments") = 8;
            b.uniform("threshold") = 0.5f;
            b.uniform("offsetPos").set(off, 2);
            b.uniform("glowRadius") = 0.35f;
        },
        [&](int i) {
            SkRuntimeShaderBuilder b(s_hud_effect);
            b.uniform("time") = float(i) * 0.01f;
            b.uniform("resolution") = SkV2{static_cast<float>(W), static_cast<float>(H)};
            b.uniform("baseColor") = SkV4{0.1f, 0.2f, 0.3f, 1.0f};
            b.uniform("accentColor") = SkV4{0.9f, 0.8f, 0.1f, 1.0f};
            b.uniform("speed") = 1.5f;
            b.uniform("scale") = 1.0f;
            b.uniform("segments") = 8;
            b.uniform("threshold") = 0.5f;
            b.uniform("offsetPos") = SkV2{100.0f, 100.0f};
            b.uniform("glowRadius") = 0.35f;
        }
    );

    benchmark_uniform_updates("11. Shader Instantiation: makeShader() (1,000 iters)",
        [&](int) {
            nisaba::gpu::RuntimeShaderBuilder b(n_hud_effect);
            auto s = b.makeShader();
            (void)s;
        },
        [&](int) {
            SkRuntimeShaderBuilder b(s_hud_effect);
            auto s = b.makeShader();
            (void)s;
        }
    );

    // =========================================================================
    // Pre-Compile Shaders for Hardware GPU Draw Suites (Parts 3, 4, 5)
    // =========================================================================
    // Pre-create shader objects to isolate steady-state GPU frame rendering
    auto n_eff_gradient = nisaba::gpu::RuntimeEffect::MakeForShader(kSksl_SimpleGradient).effect;
    auto s_eff_gradient = SkRuntimeEffect::MakeForShader(SkString(kSksl_SimpleGradient)).effect;
    nisaba::gpu::RuntimeShaderBuilder n_bld_gradient(n_eff_gradient);
    SkRuntimeShaderBuilder s_bld_gradient(s_eff_gradient);
    n_bld_gradient.uniform("time") = 1.0f;
    float res_arr[2] = {static_cast<float>(W), static_cast<float>(H)};
    n_bld_gradient.uniform("resolution").set(res_arr, 2);
    s_bld_gradient.uniform("time") = 1.0f;
    s_bld_gradient.uniform("resolution") = SkV2{static_cast<float>(W), static_cast<float>(H)};
    auto n_sh_gradient = n_bld_gradient.makeShader();
    auto s_sh_gradient = s_bld_gradient.makeShader();

    auto n_eff_plasma = nisaba::gpu::RuntimeEffect::MakeForShader(kSksl_CyberPlasma).effect;
    auto s_eff_plasma = SkRuntimeEffect::MakeForShader(SkString(kSksl_CyberPlasma)).effect;
    nisaba::gpu::RuntimeShaderBuilder n_bld_plasma(n_eff_plasma);
    SkRuntimeShaderBuilder s_bld_plasma(s_eff_plasma);
    n_bld_plasma.uniform("time") = 1.5f;
    n_bld_plasma.uniform("resolution").set(res_arr, 2);
    float tint_arr[4] = {0.1f, 0.8f, 0.95f, 1.0f};
    n_bld_plasma.uniform("colorTint").set(tint_arr, 4);
    s_bld_plasma.uniform("time") = 1.5f;
    s_bld_plasma.uniform("resolution") = SkV2{static_cast<float>(W), static_cast<float>(H)};
    s_bld_plasma.uniform("colorTint") = SkV4{0.1f, 0.8f, 0.95f, 1.0f};
    auto n_sh_plasma = n_bld_plasma.makeShader();
    auto s_sh_plasma = s_bld_plasma.makeShader();

    auto n_eff_fbm = nisaba::gpu::RuntimeEffect::MakeForShader(kSksl_FractalNoise).effect;
    auto s_eff_fbm = SkRuntimeEffect::MakeForShader(SkString(kSksl_FractalNoise)).effect;
    nisaba::gpu::RuntimeShaderBuilder n_bld_fbm(n_eff_fbm);
    SkRuntimeShaderBuilder s_bld_fbm(s_eff_fbm);
    n_bld_fbm.uniform("time") = 2.0f;
    n_bld_fbm.uniform("resolution").set(res_arr, 2);
    s_bld_fbm.uniform("time") = 2.0f;
    s_bld_fbm.uniform("resolution") = SkV2{static_cast<float>(W), static_cast<float>(H)};
    auto n_sh_fbm = n_bld_fbm.makeShader();
    auto s_sh_fbm = s_bld_fbm.makeShader();

    auto n_eff_voronoi = nisaba::gpu::RuntimeEffect::MakeForShader(kSksl_Voronoi).effect;
    auto s_eff_voronoi = SkRuntimeEffect::MakeForShader(SkString(kSksl_Voronoi)).effect;
    nisaba::gpu::RuntimeShaderBuilder n_bld_voronoi(n_eff_voronoi);
    SkRuntimeShaderBuilder s_bld_voronoi(s_eff_voronoi);
    n_bld_voronoi.uniform("time") = 1.2f;
    n_bld_voronoi.uniform("resolution").set(res_arr, 2);
    s_bld_voronoi.uniform("time") = 1.2f;
    s_bld_voronoi.uniform("resolution") = SkV2{static_cast<float>(W), static_cast<float>(H)};
    auto n_sh_voronoi = n_bld_voronoi.makeShader();
    auto s_sh_voronoi = s_bld_voronoi.makeShader();

    auto n_eff_sdf = nisaba::gpu::RuntimeEffect::MakeForShader(kSksl_RaymarchSDF).effect;
    auto s_eff_sdf = SkRuntimeEffect::MakeForShader(SkString(kSksl_RaymarchSDF)).effect;
    nisaba::gpu::RuntimeShaderBuilder n_bld_sdf(n_eff_sdf);
    SkRuntimeShaderBuilder s_bld_sdf(s_eff_sdf);
    n_bld_sdf.uniform("time") = 0.5f;
    n_bld_sdf.uniform("resolution").set(res_arr, 2);
    float ldir[3] = {0.577f, 0.577f, -0.577f};
    n_bld_sdf.uniform("lightDir").set(ldir, 3);
    s_bld_sdf.uniform("time") = 0.5f;
    s_bld_sdf.uniform("resolution") = SkV2{static_cast<float>(W), static_cast<float>(H)};
    s_bld_sdf.uniform("lightDir") = SkV3{0.577f, 0.577f, -0.577f};
    auto n_sh_sdf = n_bld_sdf.makeShader();
    auto s_sh_sdf = s_bld_sdf.makeShader();

    auto n_eff_vignette = nisaba::gpu::RuntimeEffect::MakeForShader(kSksl_FrostedVignette).effect;
    auto s_eff_vignette = SkRuntimeEffect::MakeForShader(SkString(kSksl_FrostedVignette)).effect;
    nisaba::gpu::RuntimeShaderBuilder n_bld_vignette(n_eff_vignette);
    SkRuntimeShaderBuilder s_bld_vignette(s_eff_vignette);
    n_bld_vignette.uniform("time") = 3.0f;
    n_bld_vignette.uniform("resolution").set(res_arr, 2);
    n_bld_vignette.uniform("intensity") = 0.35f;
    s_bld_vignette.uniform("time") = 3.0f;
    s_bld_vignette.uniform("resolution") = SkV2{static_cast<float>(W), static_cast<float>(H)};
    s_bld_vignette.uniform("intensity") = 0.35f;
    auto n_sh_vignette = n_bld_vignette.makeShader();
    auto s_sh_vignette = s_bld_vignette.makeShader();

    auto n_eff_hud = n_hud_effect;
    auto s_eff_hud = s_hud_effect;
    nisaba::gpu::RuntimeShaderBuilder n_bld_hud(n_eff_hud);
    SkRuntimeShaderBuilder s_bld_hud(s_eff_hud);
    n_bld_hud.uniform("time") = 2.5f;
    n_bld_hud.uniform("resolution").set(res_arr, 2);
    float col_base[4] = {0.05f, 0.1f, 0.2f, 1.0f};
    float col_acc[4] = {0.2f, 0.9f, 0.7f, 1.0f};
    float off_pos[2] = {W * 0.5f, H * 0.5f};
    n_bld_hud.uniform("baseColor").set(col_base, 4);
    n_bld_hud.uniform("accentColor").set(col_acc, 4);
    n_bld_hud.uniform("speed") = 1.0f;
    n_bld_hud.uniform("scale") = 1.0f;
    n_bld_hud.uniform("segments") = 12;
    n_bld_hud.uniform("threshold") = 0.4f;
    n_bld_hud.uniform("offsetPos").set(off_pos, 2);
    n_bld_hud.uniform("glowRadius") = 0.3f;

    s_bld_hud.uniform("time") = 2.5f;
    s_bld_hud.uniform("resolution") = SkV2{static_cast<float>(W), static_cast<float>(H)};
    s_bld_hud.uniform("baseColor") = SkV4{0.05f, 0.1f, 0.2f, 1.0f};
    s_bld_hud.uniform("accentColor") = SkV4{0.2f, 0.9f, 0.7f, 1.0f};
    s_bld_hud.uniform("speed") = 1.0f;
    s_bld_hud.uniform("scale") = 1.0f;
    s_bld_hud.uniform("segments") = 12;
    s_bld_hud.uniform("threshold") = 0.4f;
    s_bld_hud.uniform("offsetPos") = SkV2{W * 0.5f, H * 0.5f};
    s_bld_hud.uniform("glowRadius") = 0.3f;
    auto n_sh_hud = n_bld_hud.makeShader();
    auto s_sh_hud = s_bld_hud.makeShader();

    // Generic GPU Drawing Benchmark Runner
    auto benchmark_draw = [&](const std::string& name, const std::string& category, auto op_nisaba, auto op_skia) {
        std::cout << "Running suite: " << name << "..." << std::flush;

        // -------------------------------------------------------------
        // Warmup Phase (3 iterations each)
        // -------------------------------------------------------------
        for (int i = 0; i < 3; ++i) {
            nisabaSurface->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            nisabaCtx->beginFrame(static_cast<float>(W), static_cast<float>(H), 1.0f);
            op_nisaba(*nisabaCtx);
            nisabaCtx->endFrame();
            glFinish();

            dContext->resetContext();
            nisabaSurface->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            op_skia(skiaCanvas);
            dContext->flush();
            dContext->submit(false);
            glFinish();
        }

        // -------------------------------------------------------------
        // Measure Nisaba GPU Engine (True CPU vs GPU Separation)
        // -------------------------------------------------------------
        double n_cpu_total = 0.0;
        double n_gpu_total = 0.0;
        double n_all_total = 0.0;

        nisabaSurface->bind();

        for (int i = 0; i < iterations; ++i) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            auto t0 = std::chrono::high_resolution_clock::now();
            nisabaCtx->beginFrame(static_cast<float>(W), static_cast<float>(H), 1.0f);
            op_nisaba(*nisabaCtx);
            nisabaCtx->endFrame();
            auto t1 = std::chrono::high_resolution_clock::now();

            glFinish();
            auto t2 = std::chrono::high_resolution_clock::now();

            n_cpu_total += std::chrono::duration<double, std::micro>(t1 - t0).count();
            n_gpu_total += std::chrono::duration<double, std::micro>(t2 - t1).count();
            n_all_total += std::chrono::duration<double, std::micro>(t2 - t0).count();
        }

        // -------------------------------------------------------------
        // Measure Google Skia Ganesh Engine (True CPU vs GPU Separation)
        // -------------------------------------------------------------
        double s_cpu_total = 0.0;
        double s_gpu_total = 0.0;
        double s_all_total = 0.0;

        nisabaSurface->bind();
        dContext->resetContext();

        for (int i = 0; i < iterations; ++i) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            auto t0 = std::chrono::high_resolution_clock::now();
            op_skia(skiaCanvas);
            dContext->flush();
            dContext->submit(false);
            auto t1 = std::chrono::high_resolution_clock::now();

            glFinish();
            auto t2 = std::chrono::high_resolution_clock::now();

            s_cpu_total += std::chrono::duration<double, std::micro>(t1 - t0).count();
            s_gpu_total += std::chrono::duration<double, std::micro>(t2 - t1).count();
            s_all_total += std::chrono::duration<double, std::micro>(t2 - t0).count();
        }

        ShaderStat st;
        st.name = name;
        st.category = category;
        st.nisaba_cpu_us = n_cpu_total / iterations;
        st.nisaba_gpu_us = n_gpu_total / iterations;
        st.nisaba_total_us = n_all_total / iterations;
        st.skia_cpu_us = s_cpu_total / iterations;
        st.skia_gpu_us = s_gpu_total / iterations;
        st.skia_total_us = s_all_total / iterations;
        st.is_gpu_draw = true;
        stats.push_back(st);

        std::cout << " Done. (Nisaba: " << std::fixed << std::setprecision(1) << st.nisaba_total_us
                  << " µs, Skia: " << st.skia_total_us << " µs)\n";
    };

    // =========================================================================
    // PART 3: Hot GPU Rasterization — Shader Complexity Breakdown (600x400 Card)
    // =========================================================================
    std::cout << "\n>>> Running Part 3: Shader Complexity Breakdown (600x400 Standard Card)...\n";

    benchmark_draw("12. Render: Simple Gradient Quad (600x400)", "Shader Complexity",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(240.0f, 160.0f, 600.0f, 400.0f);
            ctx.fillShader(n_sh_gradient);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_gradient);
            canvas->drawRect(SkRect::MakeXYWH(240.0f, 160.0f, 600.0f, 400.0f), p);
        }
    );

    benchmark_draw("13. Render: Cyber Plasma Quad (600x400)", "Shader Complexity",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(240.0f, 160.0f, 600.0f, 400.0f);
            ctx.fillShader(n_sh_plasma);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_plasma);
            canvas->drawRect(SkRect::MakeXYWH(240.0f, 160.0f, 600.0f, 400.0f), p);
        }
    );

    benchmark_draw("14. Render: Fractal Noise FBM Quad (600x400)", "Shader Complexity",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(240.0f, 160.0f, 600.0f, 400.0f);
            ctx.fillShader(n_sh_fbm);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_fbm);
            canvas->drawRect(SkRect::MakeXYWH(240.0f, 160.0f, 600.0f, 400.0f), p);
        }
    );

    benchmark_draw("15. Render: Voronoi Cells Quad (600x400)", "Shader Complexity",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(240.0f, 160.0f, 600.0f, 400.0f);
            ctx.fillShader(n_sh_voronoi);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_voronoi);
            canvas->drawRect(SkRect::MakeXYWH(240.0f, 160.0f, 600.0f, 400.0f), p);
        }
    );

    benchmark_draw("16. Render: Raymarched 3D SDF Quad (600x400)", "Shader Complexity",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(240.0f, 160.0f, 600.0f, 400.0f);
            ctx.fillShader(n_sh_sdf);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_sdf);
            canvas->drawRect(SkRect::MakeXYWH(240.0f, 160.0f, 600.0f, 400.0f), p);
        }
    );

    benchmark_draw("17. Render: Frosted Vignette Quad (600x400)", "Shader Complexity",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(240.0f, 160.0f, 600.0f, 400.0f);
            ctx.fillShader(n_sh_vignette);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_vignette);
            canvas->drawRect(SkRect::MakeXYWH(240.0f, 160.0f, 600.0f, 400.0f), p);
        }
    );

    benchmark_draw("18. Render: Multi-Uniform HUD Quad (600x400)", "Shader Complexity",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(240.0f, 160.0f, 600.0f, 400.0f);
            ctx.fillShader(n_sh_hud);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_hud);
            canvas->drawRect(SkRect::MakeXYWH(240.0f, 160.0f, 600.0f, 400.0f), p);
        }
    );

    // =========================================================================
    // PART 4: Geometry & Primitive Workloads with Runtime Shaders
    // =========================================================================
    std::cout << "\n>>> Running Part 4: Geometry & Primitive Workloads with Shaders...\n";

    benchmark_draw("19. Geometry: Fullscreen HD Quad (1080x720)", "Geometry Workload",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(0.0f, 0.0f, static_cast<float>(W), static_cast<float>(H));
            ctx.fillShader(n_sh_plasma);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_plasma);
            canvas->drawRect(SkRect::MakeXYWH(0.0f, 0.0f, static_cast<float>(W), static_cast<float>(H)), p);
        }
    );

    benchmark_draw("20. Geometry: Small Tile Quad (128x128)", "Geometry Workload",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.rect(476.0f, 296.0f, 128.0f, 128.0f);
            ctx.fillShader(n_sh_plasma);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_plasma);
            canvas->drawRect(SkRect::MakeXYWH(476.0f, 296.0f, 128.0f, 128.0f), p);
        }
    );

    benchmark_draw("21. Geometry: Rounded Rectangle Card (400x300, r=24)", "Geometry Workload",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.roundedRect(340.0f, 210.0f, 400.0f, 300.0f, 24.0f);
            ctx.fillShader(n_sh_plasma);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_plasma);
            canvas->drawRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(340.0f, 210.0f, 400.0f, 300.0f), 24.0f, 24.0f), p);
        }
    );

    benchmark_draw("22. Geometry: Stroked Rounded Rect (400x300, 12px)", "Geometry Workload",
        [&](nisaba::gpu::Context& ctx) {
            ctx.beginPath();
            ctx.roundedRect(340.0f, 210.0f, 400.0f, 300.0f, 24.0f);
            ctx.strokeWidth(12.0f);
            ctx.strokeShader(n_sh_plasma);
            ctx.stroke();
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(12.0f);
            p.setShader(s_sh_plasma);
            canvas->drawRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(340.0f, 210.0f, 400.0f, 300.0f), 24.0f, 24.0f), p);
        }
    );

    // 10-Point Star Path
    benchmark_draw("23. Geometry: 10-Point Star Vector Path", "Geometry Workload",
        [&](nisaba::gpu::Context& ctx) {
            float cx = W * 0.5f;
            float cy = H * 0.5f;
            ctx.beginPath();
            for (int i = 0; i < 20; ++i) {
                float a = i * (M_PI / 10.0f) - M_PI * 0.5f;
                float r = (i % 2 == 0) ? 220.0f : 100.0f;
                float px = cx + std::cos(a) * r;
                float py = cy + std::sin(a) * r;
                if (i == 0) ctx.moveTo(px, py);
                else ctx.lineTo(px, py);
            }
            ctx.closePath();
            ctx.fillShader(n_sh_plasma);
            ctx.fill();
        },
        [&](SkCanvas* canvas) {
            float cx = W * 0.5f;
            float cy = H * 0.5f;
            SkPath star;
            for (int i = 0; i < 20; ++i) {
                float a = i * (M_PI / 10.0f) - M_PI * 0.5f;
                float r = (i % 2 == 0) ? 220.0f : 100.0f;
                float px = cx + std::cos(a) * r;
                float py = cy + std::sin(a) * r;
                if (i == 0) star.moveTo(px, py);
                else star.lineTo(px, py);
            }
            star.close();
            SkPaint p;
            p.setShader(s_sh_plasma);
            canvas->drawPath(star, p);
        }
    );

    benchmark_draw("24. Geometry: Multi-Rect Grid (24 Cards, 100x80)", "Geometry Workload",
        [&](nisaba::gpu::Context& ctx) {
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 6; ++c) {
                    float x = 60.0f + c * 165.0f;
                    float y = 40.0f + r * 160.0f;
                    ctx.beginPath();
                    ctx.rect(x, y, 100.0f, 80.0f);
                    ctx.fillShader(n_sh_gradient);
                    ctx.fill();
                }
            }
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_gradient);
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 6; ++c) {
                    float x = 60.0f + c * 165.0f;
                    float y = 40.0f + r * 160.0f;
                    canvas->drawRect(SkRect::MakeXYWH(x, y, 100.0f, 80.0f), p);
                }
            }
        }
    );

    benchmark_draw("25. Geometry: Multi-RRect Grid (24 Cards, r=12)", "Geometry Workload",
        [&](nisaba::gpu::Context& ctx) {
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 6; ++c) {
                    float x = 60.0f + c * 165.0f;
                    float y = 40.0f + r * 160.0f;
                    ctx.beginPath();
                    ctx.roundedRect(x, y, 100.0f, 80.0f, 12.0f);
                    ctx.fillShader(n_sh_gradient);
                    ctx.fill();
                }
            }
        },
        [&](SkCanvas* canvas) {
            SkPaint p;
            p.setShader(s_sh_gradient);
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 6; ++c) {
                    float x = 60.0f + c * 165.0f;
                    float y = 40.0f + r * 160.0f;
                    canvas->drawRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(x, y, 100.0f, 80.0f), 12.0f, 12.0f), p);
                }
            }
        }
    );

    // =========================================================================
    // PART 5: Shader State Transitions & Multi-Program Pipelining
    // =========================================================================
    std::cout << "\n>>> Running Part 5: Shader State Transitions & Multi-Program Pipelining...\n";

    benchmark_draw("26. Pipeline: Ping-Pong Shaders (20 Swaps/Frame)", "Pipeline State",
        [&](nisaba::gpu::Context& ctx) {
            for (int k = 0; k < 20; ++k) {
                float qx = 40.0f + (k % 5) * 200.0f;
                float qy = 40.0f + (k / 5) * 160.0f;
                ctx.beginPath();
                ctx.rect(qx, qy, 180.0f, 140.0f);
                ctx.fillShader((k % 2 == 0) ? n_sh_plasma : n_sh_voronoi);
                ctx.fill();
            }
        },
        [&](SkCanvas* canvas) {
            for (int k = 0; k < 20; ++k) {
                float qx = 40.0f + (k % 5) * 200.0f;
                float qy = 40.0f + (k / 5) * 160.0f;
                SkPaint p;
                p.setShader((k % 2 == 0) ? s_sh_plasma : s_sh_voronoi);
                canvas->drawRect(SkRect::MakeXYWH(qx, qy, 180.0f, 140.0f), p);
            }
        }
    );

    benchmark_draw("27. Pipeline: Interleaved Shader & Solid Fills", "Pipeline State",
        [&](nisaba::gpu::Context& ctx) {
            for (int k = 0; k < 20; ++k) {
                float qx = 40.0f + (k % 5) * 200.0f;
                float qy = 40.0f + (k / 5) * 160.0f;
                ctx.beginPath();
                ctx.rect(qx, qy, 180.0f, 140.0f);
                if (k % 2 == 0) {
                    ctx.fillShader(n_sh_plasma);
                } else {
                    ctx.fillColor(nisaba::Color::from_rgba8(220, 50, 60, 255));
                }
                ctx.fill();
            }
        },
        [&](SkCanvas* canvas) {
            for (int k = 0; k < 20; ++k) {
                float qx = 40.0f + (k % 5) * 200.0f;
                float qy = 40.0f + (k / 5) * 160.0f;
                SkPaint p;
                if (k % 2 == 0) {
                    p.setShader(s_sh_plasma);
                } else {
                    p.setColor(SkColorSetARGB(255, 220, 50, 60));
                }
                canvas->drawRect(SkRect::MakeXYWH(qx, qy, 180.0f, 140.0f), p);
            }
        }
    );

    // =========================================================================
    // Format and Print Comprehensive Quantitative Scorecard
    // =========================================================================
    std::cout << "\n\n===============================================================================================================================================\n";
    std::cout << "                                NISABA vs GOOGLE SKIA — RUNTIME SHADER BENCHMARK REPORT                                         \n";
    std::cout << "                                    (Strict 1:1 Parity • 100% Headless Offscreen Pipeline)                                     \n";
    std::cout << "===============================================================================================================================================\n";
    std::cout << std::left << std::setw(4) << "#"
              << std::setw(50) << "Suite Name"
              << std::right << std::setw(12) << "Nisaba CPU"
              << std::setw(12) << "Nisaba GPU"
              << std::setw(13) << "Nisaba Total"
              << std::setw(12) << "Skia CPU"
              << std::setw(12) << "Skia GPU"
              << std::setw(13) << "Skia Total"
              << std::setw(15) << "CPU Speedup"
              << std::setw(15) << "Total Speedup"
              << "\n";
    std::cout << "-----------------------------------------------------------------------------------------------------------------------------------------------\n";

    double total_nisaba_cpu = 0.0;
    double total_nisaba_gpu = 0.0;
    double total_nisaba_all = 0.0;
    double total_skia_cpu = 0.0;
    double total_skia_gpu = 0.0;
    double total_skia_all = 0.0;

    int nisaba_wins = 0;
    int skia_wins = 0;
    int ties = 0;

    for (size_t i = 0; i < stats.size(); ++i) {
        const auto& s = stats[i];
        total_nisaba_cpu += s.nisaba_cpu_us;
        total_nisaba_gpu += s.nisaba_gpu_us;
        total_nisaba_all += s.nisaba_total_us;

        total_skia_cpu += s.skia_cpu_us;
        total_skia_gpu += s.skia_gpu_us;
        total_skia_all += s.skia_total_us;

        std::string cpu_speedup_str;
        if (s.nisaba_cpu_us < s.skia_cpu_us) {
            double ratio = s.skia_cpu_us / std::max(0.001, s.nisaba_cpu_us);
            if (ratio < 1.05) cpu_speedup_str = "Tie (~1.0x)";
            else {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << ratio << "x Nisaba";
                cpu_speedup_str = ss.str();
            }
        } else {
            double ratio = s.nisaba_cpu_us / std::max(0.001, s.skia_cpu_us);
            if (ratio < 1.05) cpu_speedup_str = "Tie (~1.0x)";
            else {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << ratio << "x Skia";
                cpu_speedup_str = ss.str();
            }
        }

        std::string total_speedup_str;
        if (s.nisaba_total_us < s.skia_total_us) {
            double ratio = s.skia_total_us / std::max(0.001, s.nisaba_total_us);
            if (ratio < 1.05) {
                total_speedup_str = "Tie (~1.0x)";
                ties++;
            } else {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << ratio << "x Nisaba";
                total_speedup_str = ss.str();
                nisaba_wins++;
            }
        } else {
            double ratio = s.nisaba_total_us / std::max(0.001, s.skia_total_us);
            if (ratio < 1.05) {
                total_speedup_str = "Tie (~1.0x)";
                ties++;
            } else {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << ratio << "x Skia";
                total_speedup_str = ss.str();
                skia_wins++;
            }
        }

        std::cout << std::left << std::setw(4) << (i + 1)
                  << std::setw(50) << s.name
                  << std::right << std::fixed << std::setprecision(1)
                  << std::setw(9) << s.nisaba_cpu_us << " µs"
                  << std::setw(9) << s.nisaba_gpu_us << " µs"
                  << std::setw(10) << s.nisaba_total_us << " µs"
                  << std::setw(9) << s.skia_cpu_us << " µs"
                  << std::setw(9) << s.skia_gpu_us << " µs"
                  << std::setw(10) << s.skia_total_us << " µs"
                  << std::setw(15) << cpu_speedup_str
                  << std::setw(15) << total_speedup_str
                  << "\n";
    }

    std::cout << "-----------------------------------------------------------------------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(4) << "--"
              << std::setw(50) << "TOTAL BENCHMARK OVERHEAD"
              << std::right << std::fixed << std::setprecision(1)
              << std::setw(9) << total_nisaba_cpu << " µs"
              << std::setw(9) << total_nisaba_gpu << " µs"
              << std::setw(10) << total_nisaba_all << " µs"
              << std::setw(9) << total_skia_cpu << " µs"
              << std::setw(9) << total_skia_gpu << " µs"
              << std::setw(10) << total_skia_all << " µs"
              << std::setw(15) << (total_nisaba_cpu < total_skia_cpu ? (std::to_string(total_skia_cpu / total_nisaba_cpu).substr(0, 4) + "x Nisaba") : (std::to_string(total_nisaba_cpu / total_skia_cpu).substr(0, 4) + "x Skia"))
              << std::setw(15) << (total_nisaba_all < total_skia_all ? (std::to_string(total_skia_all / total_nisaba_all).substr(0, 4) + "x Nisaba") : (std::to_string(total_nisaba_all / total_skia_all).substr(0, 4) + "x Skia"))
              << "\n";
    std::cout << "===============================================================================================================================================\n";

    std::cout << "\nEXECUTIVE SUMMARY:\n";
    std::cout << "  • Total Suites Evaluated: " << stats.size() << "\n";
    std::cout << "  • Nisaba Wins:           " << nisaba_wins << " (" << std::fixed << std::setprecision(1) << (nisaba_wins * 100.0 / stats.size()) << "%)\n";
    std::cout << "  • Skia Wins:             " << skia_wins << " (" << std::fixed << std::setprecision(1) << (skia_wins * 100.0 / stats.size()) << "%)\n";
    std::cout << "  • Statistical Ties:      " << ties << " (" << std::fixed << std::setprecision(1) << (ties * 100.0 / stats.size()) << "%)\n";
    std::cout << "  • Overall CPU Overhead:  " << std::fixed << std::setprecision(2)
              << (total_skia_cpu / total_nisaba_cpu) << "x faster (" << (total_nisaba_cpu < total_skia_cpu ? "Nisaba" : "Skia") << ")\n";
    std::cout << "  • Overall End-to-End:    " << std::fixed << std::setprecision(2)
              << (total_skia_all / total_nisaba_all) << "x faster (" << (total_nisaba_all < total_skia_all ? "Nisaba" : "Skia") << ")\n";

    if (!json_output_path.empty()) {
        std::ofstream jf(json_output_path);
        if (jf.is_open()) {
            jf << "[\n";
            for (size_t i = 0; i < stats.size(); ++i) {
                const auto& s = stats[i];
                jf << "  {\n"
                   << "    \"suite\": \"" << s.name << "\",\n"
                   << "    \"category\": \"" << s.category << "\",\n"
                   << "    \"nisaba_cpu_us\": " << s.nisaba_cpu_us << ",\n"
                   << "    \"nisaba_gpu_us\": " << s.nisaba_gpu_us << ",\n"
                   << "    \"nisaba_total_us\": " << s.nisaba_total_us << ",\n"
                   << "    \"skia_cpu_us\": " << s.skia_cpu_us << ",\n"
                   << "    \"skia_gpu_us\": " << s.skia_gpu_us << ",\n"
                   << "    \"skia_total_us\": " << s.skia_total_us << "\n"
                   << "  }" << (i + 1 < stats.size() ? "," : "") << "\n";
            }
            jf << "]\n";
        }
    }

    return 0;
}
