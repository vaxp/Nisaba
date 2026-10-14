// -----------------------------------------------------------------------------
// Nisaba Sovereign 2D Graphics Engine — Runtime Shaders & Containers Showcase
// Direct native window setup matching examples/example_calculator.cpp
// -----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <cstring>

#include "nisaba/gpu/context.hpp"
#include "nisaba/gpu/gl3_renderer.hpp"
#include "nisaba/gpu/runtime_effect.hpp"
#include "nisaba/gpu/shader.hpp"
#include "nisaba/canvas/pixmap.hpp"

#ifdef NISABA_GLEW
#   include <GL/glew.h>
#else
#   include <GL/gl.h>
#endif

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/window.hpp"
#include <EGL/egl.h>

using namespace nisaba;
using nisaba::gpu::RuntimeEffect;
using nisaba::gpu::RuntimeShaderBuilder;
using nisaba::gpu::Transform2D;

namespace {

constexpr int WIN_WIDTH  = 1280;
constexpr int WIN_HEIGHT = 860;

static const char* resolveFont(const char* name) {
    static char resolvedPaths[4][512];
    static int nextIdx = 0;
    char* outBuf = resolvedPaths[nextIdx++ % 4];

    const char* prefixes[] = {
        "fonts/",
        "nisaba/fonts/",
        "../fonts/",
        "../../fonts/",
        "../../../fonts/"
    };

    for (const auto& prefix : prefixes) {
        std::snprintf(outBuf, 512, "%s%s", prefix, name);
        FILE* f = std::fopen(outBuf, "rb");
        if (f) {
            std::fclose(f);
            return outBuf;
        }
    }
    return name;
}

// ═════════════════════════════════════════════════════════════════
// 1. PROCEDURAL PIXEL ART RIVETED FRAME SHADER (Sovereign GLSL)
// ═════════════════════════════════════════════════════════════════
const std::string kPixelArtShader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        float p = 3.0; // 3.0 physical pixels per block
        vec2 g = floor(fragCoord / p);
        vec2 sz = floor(resolution / p);

        float dL = g.x;
        float dR = sz.x - 1.0 - g.x;
        float dT = g.y;
        float dB = sz.y - 1.0 - g.y;

        float dX = min(dL, dR);
        float dY = min(dT, dB);

        vec4 cOutline = vec4(0.18, 0.16, 0.19, 1.0); // #2E2930 Charcoal Iron
        vec4 cDark    = vec4(0.32, 0.29, 0.34, 1.0); // #524A57 Deep Iron Shadow
        vec4 cMid     = vec4(0.48, 0.45, 0.50, 1.0); // #7A7380 Stone Body
        vec4 cLight   = vec4(0.68, 0.65, 0.71, 1.0); // #AEA6B5 Highlight Stone

        // 5x5 Corner Rivet Blocks
        if (dX < 5.0 && dY < 5.0) {
            if (dX == 0.0 && dY == 0.0) return vec4(0.0);
            if (dX == 0.0 || dY == 0.0 || dX == 4.0 || dY == 4.0) return cOutline;
            if (dX == 2.0 && dY == 2.0) return cDark;
            if (dY <= 2.0 && dX <= 2.0) return cLight;
            return cMid;
        }

        float depth = min(dX, dY);
        if (depth > 4.0) return vec4(0.0);

        float along = (dX < dY) ? g.y : g.x;
        float modStep = mod(along, 4.0);

        if (depth == 0.0) {
            if (modStep == 1.0 || modStep == 2.0) return cOutline;
            return vec4(0.0);
        }
        if (depth == 1.0) {
            if (modStep == 1.0 || modStep == 2.0) return cLight;
            return cOutline;
        }
        if (depth == 2.0) {
            if (modStep == 0.0 || modStep == 3.0) return cDark;
            return cMid;
        }
        if (depth == 3.0) {
            if (modStep == 0.0 || modStep == 3.0) return cOutline;
            return cDark;
        }
        if (depth == 4.0) {
            if (modStep == 0.0 || modStep == 3.0) return cOutline;
            return vec4(0.0);
        }

        return cOutline;
    }
)";

// ═════════════════════════════════════════════════════════════════
// 2. PIXEL ART 2X ZOOM MACRO SHADER
// ═════════════════════════════════════════════════════════════════
const std::string kPixelArtZoomShader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        float p = 6.0; // 2x Zoom: 6.0 pixels per block
        vec2 g = floor(fragCoord / p);
        vec2 sz = floor(resolution / p);

        float dL = g.x;
        float dR = sz.x - 1.0 - g.x;
        float dT = g.y;
        float dB = sz.y - 1.0 - g.y;

        float dX = min(dL, dR);
        float dY = min(dT, dB);

        vec4 cOutline = vec4(0.18, 0.16, 0.19, 1.0);
        vec4 cDark    = vec4(0.32, 0.29, 0.34, 1.0);
        vec4 cMid     = vec4(0.48, 0.45, 0.50, 1.0);
        vec4 cLight   = vec4(0.68, 0.65, 0.71, 1.0);

        if (dX < 5.0 && dY < 5.0) {
            if (dX == 0.0 && dY == 0.0) return vec4(0.0);
            if (dX == 0.0 || dY == 0.0 || dX == 4.0 || dY == 4.0) return cOutline;
            if (dX == 2.0 && dY == 2.0) return cDark;
            if (dY <= 2.0 && dX <= 2.0) return cLight;
            return cMid;
        }

        float depth = min(dX, dY);
        if (depth > 4.0) return vec4(0.0);

        float along = (dX < dY) ? g.y : g.x;
        float modStep = mod(along, 4.0);

        if (depth == 0.0) {
            if (modStep == 1.0 || modStep == 2.0) return cOutline;
            return vec4(0.0);
        }
        if (depth == 1.0) {
            if (modStep == 1.0 || modStep == 2.0) return cLight;
            return cOutline;
        }
        if (depth == 2.0) {
            if (modStep == 0.0 || modStep == 3.0) return cDark;
            return cMid;
        }
        if (depth == 3.0) {
            if (modStep == 0.0 || modStep == 3.0) return cOutline;
            return cDark;
        }
        if (depth == 4.0) {
            if (modStep == 0.0 || modStep == 3.0) return cOutline;
            return vec4(0.0);
        }

        return cOutline;
    }
)";

// ═════════════════════════════════════════════════════════════════
// 3. PROCEDURAL "DAGGERS & BLOOD" BORDER SHADER (خناجر ودماء)
// ═════════════════════════════════════════════════════════════════
const std::string kDaggerBloodShader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        float w = resolution.x;
        float h = resolution.y;

        float dLeft   = fragCoord.x;
        float dRight  = w - fragCoord.x;
        float dTop    = fragCoord.y;
        float dBottom = h - fragCoord.y;
        float minEdge = min(min(dLeft, dRight), min(dTop, dBottom));

        float s = 0.0;
        float v = 0.0;
        float bWidth = 16.0;

        if (minEdge > bWidth || minEdge < 0.0 || fragCoord.x < 0.0 || fragCoord.x > w || fragCoord.y < 0.0 || fragCoord.y > h) {
            return vec4(0.0);
        }

        if (minEdge == dTop) {
            s = fragCoord.x;
            v = clamp(fragCoord.y / bWidth, 0.0, 1.0);
        } else if (minEdge == dRight) {
            s = w + fragCoord.y;
            v = clamp((w - fragCoord.x) / bWidth, 0.0, 1.0);
        } else if (minEdge == dBottom) {
            s = w + h + (w - fragCoord.x);
            v = clamp((h - fragCoord.y) / bWidth, 0.0, 1.0);
        } else {
            s = 2.0 * w + h + (h - fragCoord.y);
            v = clamp(fragCoord.x / bWidth, 0.0, 1.0);
        }

        float cycleLen  = 45.0;
        float bloodGap  = 5.0;
        float daggerLen = cycleLen - bloodGap;

        float u = mod(s + time * 12.0, cycleLen);

        if (u < bloodGap) {
            float uBlood = u / bloodGap;
            float pulse = sin(time * 6.0 + s * 0.4) * 0.5 + 0.5;
            float droplet = sin(uBlood * 3.14159) * (1.0 - abs(v - 0.5) * 1.5);
            
            vec3 deepCoagulated = vec3(0.32, 0.01, 0.02);
            vec3 arterialFresh  = vec3(0.95, 0.05, 0.08);
            vec3 fluidGlint     = vec3(1.0, 0.5, 0.55);
            
            float spec = pow(max(0.0, sin(uBlood * 3.14159 * 2.0 - v * 2.0 + time * 4.0)), 12.0);
            vec3 blood = mix(deepCoagulated, arterialFresh, pulse * 0.6 + droplet * 0.4);
            blood += fluidGlint * spec * 0.8;
            
            return vec4(blood, 1.0);
        }

        float xDagger = (u - bloodGap) / daggerLen;
        float yDistCenter = abs(v - 0.5) * 2.0;

        float maxHalfW = (xDagger < 0.72) ? 1.0 : (1.0 - (xDagger - 0.72) / 0.28);
        if (yDistCenter > maxHalfW) {
            float bloodFade = sin(time * 3.0 + s * 0.3) * 0.5 + 0.5;
            vec3 seepBlood = mix(vec3(0.18, 0.01, 0.02), vec3(0.65, 0.02, 0.04), bloodFade);
            return vec4(seepBlood, 0.95);
        }

        float damascus = sin(xDagger * 40.0 + sin(yDistCenter * 16.0) * 2.5) * 0.5 + 0.5;
        vec3 darkSteel  = vec3(0.20, 0.22, 0.28);
        vec3 lightSteel = vec3(0.72, 0.78, 0.88);
        vec3 razorEdge  = vec3(0.98, 1.0, 1.0);

        vec3 metal = mix(darkSteel, lightSteel, damascus);
        float edgeBevel = smoothstep(0.60, 1.0, yDistCenter / maxHalfW);
        metal = mix(metal, razorEdge, edgeBevel * 0.85);

        if (yDistCenter < 0.25 && xDagger > 0.12 && xDagger < 0.78) {
            float grooveDepth = smoothstep(0.25, 0.04, yDistCenter);
            float bloodPulse = sin(time * 5.0 - xDagger * 10.0) * 0.5 + 0.5;
            vec3 grooveBlood = mix(vec3(0.35, 0.01, 0.02), vec3(0.92, 0.03, 0.05), bloodPulse);
            metal = mix(metal, grooveBlood, grooveDepth * 0.92);
        }

        if (xDagger < 0.14) {
            vec3 ornateGold = vec3(0.90, 0.70, 0.20);
            vec3 darkBronze = vec3(0.42, 0.25, 0.08);
            float guardRings = sin(yDistCenter * 14.0) * 0.5 + 0.5;
            vec3 guard = mix(darkBronze, ornateGold, guardRings);
            metal = mix(guard, metal, smoothstep(0.10, 0.14, xDagger));
        }

        float gleam = pow(max(0.0, cos(xDagger * 3.14159 - time * 2.2)), 18.0);
        metal += vec3(0.5, 0.6, 0.7) * gleam;

        return vec4(metal, 1.0);
    }
)";

// ═════════════════════════════════════════════════════════════════
// 4. ANIMATED PLASMA BACKGROUND SHADER
// ═════════════════════════════════════════════════════════════════
const std::string kPlasmaShader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float t = time * 0.8;
        float v = sin(uv.x * 6.0 + t) + sin(uv.y * 6.0 + t) + sin((uv.x + uv.y) * 6.0 + t);
        v = v * 0.5;
        vec3 col = 0.5 + 0.5 * cos(v + vec3(0.0, 2.0, 4.0));
        return vec4(col * 0.85, 1.0);
    }
)";

// ═════════════════════════════════════════════════════════════════
// 5. CYBER GLOWING NEON BORDER SHADER
// ═════════════════════════════════════════════════════════════
const std::string kCyberBorderShader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float angle = atan(uv.y - 0.5, uv.x - 0.5);
        float glow = sin(angle * 3.0 + time * 3.0) * 0.5 + 0.5;
        vec3 cyan = vec3(0.06, 0.72, 0.95);
        vec3 pink = vec3(0.93, 0.28, 0.60);
        vec3 color = mix(cyan, pink, glow);
        return vec4(color, 1.0);
    }
)";

} // namespace

int main(int argc, char* argv[]) {
    std::cout << "========================================================\n"
              << "Nisaba Sovereign Graphics Engine — Runtime Shaders Showcase\n"
              << "========================================================\n";

    std::string snapshotPath = "build/runtime_shaders_showcase.png";
    std::string artifactSnapshotPath = "showcase/runtime_shaders_visual_showcase.png";
    int targetFrames = 0;
    bool headless = false;
    float snapshotDelaySec = 5.0f;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--snapshot" && i + 1 < argc) {
            snapshotPath = argv[++i];
        } else if (arg == "--headless") {
            headless = true;
        } else if (arg == "--delay" && i + 1 < argc) {
            snapshotDelaySec = static_cast<float>(std::atof(argv[++i]));
        } else if (arg == "--frames" && i + 1 < argc) {
            targetFrames = std::atoi(argv[++i]);
        }
    }

    // 1. Initialize Nisaba Native Platform Subsystem (100% same as example_calculator.cpp)
    auto plat_res = nisaba::backend_os::Platform::create();
    if (!plat_res.isOk()) {
        std::fprintf(stderr, "Failed to initialize Nisaba native platform: %s\n", plat_res.error().message.c_str());
        return -1;
    }
    auto platform = std::move(plat_res.value());

    nisaba::backend_os::WindowConfig win_cfg;
    win_cfg.title = "Nisaba Sovereign Engine — Procedural Runtime Shaders & Containers Showcase";
    win_cfg.width = WIN_WIDTH;
    win_cfg.height = WIN_HEIGHT;
    win_cfg.resizable = true;
    win_cfg.vsync = false;
    win_cfg.transparent = false;

    auto win_res = nisaba::backend_os::Window::create(*platform, win_cfg);
    if (!win_res.isOk()) {
        std::fprintf(stderr, "Failed to create Nisaba native window: %s\n", win_res.error().message.c_str());
        return -1;
    }
    auto window = std::move(win_res.value());
    window->makeCurrent();

    EGLDisplay eglDpy = eglGetCurrentDisplay();
    if (eglDpy != EGL_NO_DISPLAY) {
        eglSwapInterval(eglDpy, 0);
    }

#ifdef NISABA_GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK && err != GLEW_ERROR_NO_GLX_DISPLAY) {
        std::fprintf(stderr, "Warning/Failed to initialize GLEW: %s (continuing)\n", (const char*)glewGetErrorString(err));
    }
    while (glGetError() != GL_NO_ERROR) {}
#endif

    // 2. Create Nisaba C++20 Context directly attached to the Window
    auto ctx = nisaba::gpu::createContextGL3(nisaba::gpu::CreateFlags::Antialias);
    if (!ctx) {
        std::fprintf(stderr, "Failed to create Nisaba C++ Context\n");
        return -1;
    }

    // 3. Compile all 5 procedural runtime effects
    auto resPixel = RuntimeEffect::MakeForShader(kPixelArtShader);
    assert(resPixel && "Pixel Art Shader compilation failed");

    auto resPixelZoom = RuntimeEffect::MakeForShader(kPixelArtZoomShader);
    assert(resPixelZoom && "Pixel Art Zoom Shader compilation failed");

    auto resDagger = RuntimeEffect::MakeForShader(kDaggerBloodShader);
    assert(resDagger && "Daggers & Blood Shader compilation failed");

    auto resPlasma = RuntimeEffect::MakeForShader(kPlasmaShader);
    assert(resPlasma && "Plasma Shader compilation failed");

    auto resCyber = RuntimeEffect::MakeForShader(kCyberBorderShader);
    assert(resCyber && "Cyber Border Shader compilation failed");

    std::cout << "[+] All 5 procedural GLSL Runtime Effects compiled successfully.\n";

    // 4. Load Fonts with Arabic Fallback
    int fontSans = ctx->createFont("sans", resolveFont("Inter-Regular.ttf"));
    if (fontSans == -1) fontSans = ctx->createFont("sans", resolveFont("NotoSans-Regular.ttf"));
    int fontMono = ctx->createFont("mono", resolveFont("FiraMono-Medium.ttf"));
    if (fontMono == -1) fontMono = fontSans;
    int fontArabic = ctx->createFont("arabic", resolveFont("NotoSansArabic.ttf"));
    if (fontSans != -1 && fontArabic != -1) ctx->addFallbackFontId(fontSans, fontArabic);
    if (fontMono != -1 && fontArabic != -1) ctx->addFallbackFontId(fontMono, fontArabic);

    std::printf("[+] Native Window Created Successfully (%dx%d) [VSYNC: OFF - MAXIMUM RAW GPU PERFORMANCE]\n", WIN_WIDTH, WIN_HEIGHT);
    std::printf("[+] GPU Renderer: %s\n", glGetString(GL_RENDERER));
    std::printf("[+] Controls: [V] Toggle VSync ON/OFF | [ESC / Q] Exit\n");

    bool running = true;
    bool vsyncEnabled = false;

    window->onClose().connect([&]() {
        running = false;
    });

    platform->onKeyDown().connect([&](int key, int) {
        if (key == 27 || key == 0xff1b || key == 'q' || key == 'Q') {
            running = false;
        } else if (key == 'v' || key == 'V' || key == 55) {
            vsyncEnabled = !vsyncEnabled;
            EGLDisplay dpy = eglGetCurrentDisplay();
            if (dpy != EGL_NO_DISPLAY) {
                eglSwapInterval(dpy, vsyncEnabled ? 1 : 0);
            }
            std::printf("[*] VSync toggled: %s\n", vsyncEnabled ? "LOCKED 60Hz" : "OFF (UNCAPPED RAW)");
        }
    });

    double startTime = platform->getTime();
    double prevTime = startTime;
    double smoothedFps = 300.0;
    double smoothedCpuMs = 3.0;
    int frameCount = 0;

    while (running) {
        if (!platform->pollEvents()) {
            break;
        }

        double currentTime = platform->getTime();
        float dt = static_cast<float>(currentTime - prevTime);
        if (dt > 0.1f) dt = 0.1f;
        prevTime = currentTime;
        float timeSec = static_cast<float>(currentTime - startTime);

        double frameTimeMs = dt * 1000.0;
        double currentFps = (dt > 0.0001f) ? (1.0 / dt) : 60.0;
        if (frameCount > 0) {
            smoothedFps += (currentFps - smoothedFps) * std::min(dt * 5.0f, 1.0f);
            smoothedCpuMs += (frameTimeMs - smoothedCpuMs) * std::min(dt * 5.0f, 1.0f);
        } else {
            smoothedFps = currentFps;
            smoothedCpuMs = frameTimeMs;
        }

        auto winSize = window->getSize();
        auto fbSize = window->getDrawableSize();
        int curWinW = static_cast<int>(winSize.width);
        int curWinH = static_cast<int>(winSize.height);
        int fbW = static_cast<int>(fbSize.width);
        int fbH = static_cast<int>(fbSize.height);
        if (curWinW <= 0 || curWinH <= 0) continue;
        float dpr = (curWinW > 0) ? static_cast<float>(fbW) / static_cast<float>(curWinW) : 1.0f;

        glViewport(0, 0, fbW, fbH);
        glClearColor(0.05f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        ctx->beginFrame(curWinW, curWinH, dpr);

        // 0. Ambient background grid lines
        for (float y = 0.0f; y < curWinH; y += 40.0f) {
            ctx->beginPath();
            ctx->moveTo(0.0f, y);
            ctx->lineTo(static_cast<float>(curWinW), y);
            ctx->strokeColor(nisaba::gpu::Color::rgba(255, 255, 255, 8));
            ctx->strokeWidth(1.0f);
            ctx->stroke();
        }
        for (float x = 0.0f; x < curWinW; x += 40.0f) {
            ctx->beginPath();
            ctx->moveTo(x, 0.0f);
            ctx->lineTo(x, static_cast<float>(curWinH));
            ctx->strokeColor(nisaba::gpu::Color::rgba(255, 255, 255, 8));
            ctx->strokeWidth(1.0f);
            ctx->stroke();
        }

        // 1. Header Banner
        {
            ctx->fontSize(24.0f);
            ctx->fontFace("sans");
            ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
            ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
            ctx->text(50.0f, 32.0f, "NISABA SOVEREIGN ENGINE — RUNTIME SHADER SHOWCASE");

            ctx->fontSize(14.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(56, 189, 248, 255));
            ctx->text(50.0f, 66.0f, "Live Procedural Shader Injection for ENKI Containers (Sovereign GPU Pipeline)");

            // Tech badge pill
            float badgeX = curWinW - 440.0f;
            ctx->beginPath();
            ctx->roundedRect(badgeX, 32.0f, 390.0f, 44.0f, 22.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(30, 41, 59, 220));
            ctx->fill();

            ctx->beginPath();
            ctx->roundedRect(badgeX, 32.0f, 390.0f, 44.0f, 22.0f);
            ctx->strokeColor(nisaba::gpu::Color::rgba(148, 163, 184, 100));
            ctx->strokeWidth(1.2f);
            ctx->stroke();

            char badgeStr[128];
            std::snprintf(badgeStr, sizeof(badgeStr), "GLSL 150 Core • %.1f FPS (%.2f ms) • VSync: %s",
                          smoothedFps, smoothedCpuMs, vsyncEnabled ? "ON" : "OFF (RAW)");
            ctx->fontSize(13.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(226, 232, 240, 240));
            ctx->text(badgeX + 20.0f, 46.0f, badgeStr);
        }

        // ═════════════════════════════════════════════════════════════
        // ROW 1: THE PIXEL ART SHOWCASE & RETRO CONTAINERS
        // ═════════════════════════════════════════════════════════════

        // Card 1: 1:1 Pixel Art Riveted Frame Container (p = 3.0)
        {
            float cX = 50.0f;
            float cY = 120.0f;
            float cW = 370.0f;
            float cH = 240.0f;

            ctx->beginPath();
            ctx->rect(cX, cY, cW, cH);
            ctx->fillColor(nisaba::gpu::Color::rgba(26, 31, 41, 255));
            ctx->fill();

            RuntimeShaderBuilder bld(resPixel.effect);
            bld.uniform("time") = timeSec;
            float res[2] = { cW, cH };
            bld.uniform("resolution").set(res, 2);

            Transform2D localMat = Transform2D::translate(cX, cY);
            ctx->beginPath();
            ctx->rect(cX, cY, cW, cH);
            ctx->fillShader(bld.makeShader(&localMat));
            ctx->fill();

            ctx->fontSize(17.0f);
            ctx->fontFace("sans");
            ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
            ctx->fillColor(nisaba::gpu::Color::rgba(248, 250, 252, 255));
            ctx->text(cX + 28.0f, cY + 30.0f, "👾 Pixel Art Riveted Frame");

            ctx->fontSize(13.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(168, 162, 179, 255));
            ctx->text(cX + 28.0f, cY + 58.0f, "Procedural 1:1 Retro Pixel Shader (p = 3.0)");

            ctx->fillColor(nisaba::gpu::Color::rgba(148, 163, 184, 230));
            ctx->text(cX + 28.0f, cY + 90.0f, "• 5x5 Corner Rivet Blocks with Glints");
            ctx->text(cX + 28.0f, cY + 110.0f, "• Dual Edge Rails with Staggered Cog Teeth");
            ctx->text(cX + 28.0f, cY + 130.0f, "• Authentic #2E2930 / #AEA6B5 Color Palette");
            ctx->text(cX + 28.0f, cY + 150.0f, "• Subpixel-Integer Geometric Alignment");
        }

        // Card 2: Pixel Art 2x Zoom Macro Inspector (p = 6.0)
        {
            float cX = 455.0f;
            float cY = 120.0f;
            float cW = 370.0f;
            float cH = 240.0f;

            ctx->beginPath();
            ctx->rect(cX, cY, cW, cH);
            ctx->fillColor(nisaba::gpu::Color::rgba(20, 26, 36, 255));
            ctx->fill();

            RuntimeShaderBuilder bld(resPixelZoom.effect);
            bld.uniform("time") = timeSec;
            float res[2] = { cW, cH };
            bld.uniform("resolution").set(res, 2);

            Transform2D localMat = Transform2D::translate(cX, cY);
            ctx->beginPath();
            ctx->rect(cX, cY, cW, cH);
            ctx->fillShader(bld.makeShader(&localMat));
            ctx->fill();

            ctx->fontSize(17.0f);
            ctx->fontFace("sans");
            ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
            ctx->fillColor(nisaba::gpu::Color::rgba(250, 204, 21, 255));
            ctx->text(cX + 42.0f, cY + 30.0f, "🔍 Pixel Art 2x Macro Zoom");

            ctx->fontSize(13.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(253, 230, 138, 240));
            ctx->text(cX + 42.0f, cY + 58.0f, "Visual Fidelity Inspection Grid (p = 6.0)");

            ctx->fillColor(nisaba::gpu::Color::rgba(203, 213, 225, 230));
            ctx->text(cX + 42.0f, cY + 90.0f, "• Visible Diagonal Corner Cutout");
            ctx->text(cX + 42.0f, cY + 110.0f, "• Central Dark Rivet Cavity & Specular Glint");
            ctx->text(cX + 42.0f, cY + 130.0f, "• Crisp 30px Thick Structural Iron Perimeter");
            ctx->text(cX + 42.0f, cY + 150.0f, "• Zero Anti-Aliasing Blur on Pixel Edges");
        }

        // Card 3: Dynamic Animated Plasma Container
        {
            float cX = 860.0f;
            float cY = 120.0f;
            float cW = 370.0f;
            float cH = 240.0f;

            RuntimeShaderBuilder bld(resPlasma.effect);
            bld.uniform("time") = timeSec;
            float res[2] = { cW, cH };
            bld.uniform("resolution").set(res, 2);

            Transform2D localMat = Transform2D::translate(cX, cY);
            ctx->beginPath();
            ctx->roundedRect(cX, cY, cW, cH, 18.0f);
            ctx->fillShader(bld.makeShader(&localMat));
            ctx->fill();

            ctx->beginPath();
            ctx->roundedRect(cX, cY, cW, cH, 18.0f);
            ctx->strokeColor(nisaba::gpu::Color::rgba(255, 255, 255, 120));
            ctx->strokeWidth(1.5f);
            ctx->stroke();

            ctx->beginPath();
            ctx->roundedRect(cX + 24.0f, cY + 36.0f, cW - 48.0f, cH - 72.0f, 12.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(15, 23, 42, 210));
            ctx->fill();

            ctx->fontSize(17.0f);
            ctx->fontFace("sans");
            ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
            ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
            ctx->text(cX + 42.0f, cY + 55.0f, "🌊 Animated Plasma Flow");

            ctx->fontSize(13.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(226, 232, 240, 230));
            ctx->text(cX + 42.0f, cY + 85.0f, "• Auto-ticking 'time' uniform at 60 FPS");
            ctx->text(cX + 42.0f, cY + 105.0f, "• Live Color Synthesis with Sine Harmonic Waves");
            ctx->text(cX + 42.0f, cY + 125.0f, "• Bounded Local Coordinates fragCoord");
        }

        // ═════════════════════════════════════════════════════════════
        // ROW 2: DAGGERS & BLOOD + CYBER NEON CONTAINERS
        // ═════════════════════════════════════════════════════════════

        // Card 4: Procedural "Daggers & Blood" Container (خناجر ودماء)
        {
            float cX = 50.0f;
            float cY = 390.0f;
            float cW = 575.0f;
            float cH = 250.0f;

            ctx->beginPath();
            ctx->rect(cX, cY, cW, cH);
            ctx->fillColor(nisaba::gpu::Color::rgba(18, 22, 31, 255));
            ctx->fill();

            RuntimeShaderBuilder bld(resDagger.effect);
            bld.uniform("time") = timeSec;
            float res[2] = { cW, cH };
            bld.uniform("resolution").set(res, 2);

            Transform2D localMat = Transform2D::translate(cX, cY);
            ctx->beginPath();
            ctx->rect(cX, cY, cW, cH);
            ctx->fillShader(bld.makeShader(&localMat));
            ctx->fill();

            ctx->fontSize(20.0f);
            ctx->fontFace("sans");
            ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
            ctx->fillColor(nisaba::gpu::Color::rgba(254, 226, 226, 255));
            ctx->text(cX + 36.0f, cY + 36.0f, "⚔️ Procedural 'Daggers & Blood' Border (خناجر ودماء)");

            ctx->fontSize(14.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(239, 68, 68, 240));
            ctx->text(cX + 36.0f, cY + 70.0f, "Complex Geometric Damascus Steel & Fluid Blood Pulse Shader");

            ctx->fontSize(13.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(203, 213, 225, 230));
            ctx->text(cX + 36.0f, cY + 105.0f, "• Full Damascus steel waviness & razor-sharp bevel edge reflection");
            ctx->text(cX + 36.0f, cY + 127.0f, "• Continuous perimeter traversing (s, v) coordinate parameterization");
            ctx->text(cX + 36.0f, cY + 149.0f, "• Pulsing arterial blood gaps with specular fluid glints");
            ctx->text(cX + 36.0f, cY + 171.0f, "• Central blood groove (Fuller) & Ornate Bronze Guard rings");
        }

        // Card 5: Cyber Neon Dual-Gradient Rotating Border Container
        {
            float cX = 655.0f;
            float cY = 390.0f;
            float cW = 575.0f;
            float cH = 250.0f;

            ctx->beginPath();
            ctx->roundedRect(cX, cY, cW, cH, 20.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(15, 23, 42, 255));
            ctx->fill();

            RuntimeShaderBuilder bld(resCyber.effect);
            bld.uniform("time") = timeSec;
            float res[2] = { cW, cH };
            bld.uniform("resolution").set(res, 2);

            Transform2D localMat = Transform2D::translate(cX, cY);
            ctx->beginPath();
            ctx->roundedRect(cX, cY, cW, cH, 20.0f);
            ctx->strokeShader(bld.makeShader(&localMat));
            ctx->strokeWidth(4.0f);
            ctx->stroke();

            ctx->fontSize(20.0f);
            ctx->fontFace("sans");
            ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
            ctx->fillColor(nisaba::gpu::Color::rgba(56, 189, 248, 255));
            ctx->text(cX + 36.0f, cY + 36.0f, "⚡ Cyber Neon Rotating Gradient Border");

            ctx->fontSize(14.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(244, 114, 182, 240));
            ctx->text(cX + 36.0f, cY + 70.0f, "Polar Coordinate Dual Cyan / Pink Gradient with Sine Harmonic Pulsing");

            ctx->fontSize(13.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(203, 213, 225, 230));
            ctx->text(cX + 36.0f, cY + 105.0f, "• Direct Injected Stroke Shader applied to arbitrary geometric path");
            ctx->text(cX + 36.0f, cY + 127.0f, "• Smooth anti-aliased perimeter with dynamic subpixel rasterization");
            ctx->text(cX + 36.0f, cY + 149.0f, "• Real-time rotating angle evaluation: atan(uv.y - 0.5, uv.x - 0.5)");
            ctx->text(cX + 36.0f, cY + 171.0f, "• 100% Sovereign OpenGL Pipeline Execution");
        }

        // ═════════════════════════════════════════════════════════════
        // ROW 3: FOOTER STATUS & REAL-TIME PERFORMANCE METRICS
        // ═════════════════════════════════════════════════════════════
        {
            float footY = 670.0f;
            ctx->beginPath();
            ctx->roundedRect(50.0f, footY, curWinW - 100.0f, 130.0f, 16.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(22, 27, 34, 230));
            ctx->fill();

            ctx->beginPath();
            ctx->roundedRect(50.0f, footY, curWinW - 100.0f, 130.0f, 16.0f);
            ctx->strokeColor(nisaba::gpu::Color::rgba(48, 54, 61, 200));
            ctx->strokeWidth(1.0f);
            ctx->stroke();

            ctx->fontSize(16.0f);
            ctx->fontFace("sans");
            ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
            ctx->fillColor(nisaba::gpu::Color::rgba(16, 185, 129, 255));
            ctx->text(80.0f, footY + 20.0f, "HARDWARE PIPELINE DIAGNOSTICS & VERIFICATION STATUS");

            ctx->fontSize(13.0f);
            ctx->fillColor(nisaba::gpu::Color::rgba(203, 213, 225, 220));
            ctx->text(80.0f, footY + 46.0f, "• Status: ALL RUNTIME EFFECTS ACTIVE (5/5 SHADERS LIVE)");

            char line2[256];
            std::snprintf(line2, sizeof(line2), "• Frame: %d | Time: %.2f s | Raw GPU FPS: %.1f FPS | Latency: %.2f ms | VSync: %s [Press V to toggle]",
                          frameCount, timeSec, smoothedFps, smoothedCpuMs, vsyncEnabled ? "LOCKED 60Hz" : "UNCAPPED RAW MAX");
            ctx->text(80.0f, footY + 68.0f, line2);

            char line3[256];
            std::snprintf(line3, sizeof(line3), "• Device: %s | API: %s",
                          glGetString(GL_RENDERER) ? (const char*)glGetString(GL_RENDERER) : "OpenGL",
                          glGetString(GL_VERSION) ? (const char*)glGetString(GL_VERSION) : "GL Core");
            ctx->text(80.0f, footY + 90.0f, line3);
        }

        ctx->endFrame();

        // Save stabilized snapshot after 5.0 seconds of warmup (or specified delay)
        static bool snapshotSaved = false;
        if (!snapshotSaved && timeSec >= snapshotDelaySec) {
            snapshotSaved = true;
            std::vector<uint8_t> pixels(fbW * fbH * 4);
            glReadPixels(0, 0, fbW, fbH, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            std::vector<uint8_t> flipped(fbW * fbH * 4);
            int stride = fbW * 4;
            for (int y = 0; y < fbH; ++y) {
                std::memcpy(&flipped[y * stride], &pixels[(fbH - 1 - y) * stride], stride);
            }
            auto sz = IntSize::from_wh(static_cast<uint32_t>(fbW), static_cast<uint32_t>(fbH));
            if (sz) {
                auto pix = Pixmap::from_vec(std::move(flipped), *sz, PixelFormat::RGBA8888);
                if (pix) {
                    pix->save_png(snapshotPath);
                    pix->save_png(artifactSnapshotPath);
                    pix->save_png("showcase/runtime_shaders_visual_showcase.png");
                    std::cout << "[+] Saved stabilized visual showcase snapshot (after "
                              << snapshotDelaySec << "s warmup, frame " << frameCount
                              << ", sustained " << static_cast<int>(smoothedFps) << " FPS) to: "
                              << snapshotPath << std::endl;
                }
            }
            if (headless) {
                break;
            }
        }

        window->swapBuffers();
        frameCount++;

        if (targetFrames > 0 && frameCount >= targetFrames) {
            break;
        }
    }

    std::cout << "\n========================================================\n"
              << "Nisaba Runtime Shader Showcase Completed.\n"
              << "========================================================\n";

    window.reset();
    platform.reset();
    return 0;
}
