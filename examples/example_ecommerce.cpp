//
// NovaStore - Modern Luxury E-Commerce & Nisaba Native Image Decoder Showcase
// Built purely on the modern Nisaba C++20 High-Performance Engine & Sovereign Image Decoders
//

#include "nisaba/gpu/context.hpp"
#include "nisaba/gpu/gl3_renderer.hpp"
#include "nisaba/image/image_io.hpp"

#ifdef NISABA_GLEW
#	include <GL/glew.h>
#else
#	include <GL/gl.h>
#endif

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/window.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

// -----------------------------------------------------------------------------
// Resource Resolvers (Fonts & Showcase Images)
// -----------------------------------------------------------------------------

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

static std::string resolveResource(const std::string& path) {
	const char* prefixes[] = {
		"",
		"showcase/",
		"assets/",
		"../",
		"../../",
		"../showcase/",
		"../../showcase/",
		"../assets/",
		"../../assets/"
	};

	for (const auto& prefix : prefixes) {
		std::string testPath = prefix + path;
		FILE* f = std::fopen(testPath.c_str(), "rb");
		if (f) {
			std::fclose(f);
			return testPath;
		}
	}
	return path;
}

// -----------------------------------------------------------------------------
// E-Commerce Data Models
// -----------------------------------------------------------------------------

struct Product {
	int id;
	std::string name;
	std::string category;
	std::string imageRelPath;
	std::string resolvedPath;
	std::string formatBadge;
	nisaba::gpu::Color badgeColor;
	double price;
	float rating;
	int reviewCount;
	std::string tag;
	std::string subtitle;
	std::string description;
	std::string specs;

	// Decoder & Texture Telemetry
	int textureId{-1};
	int imgW{0};
	int imgH{0};
	double decodeTimeMs{0.0};
	bool isLoaded{false};

	// UI & Animation state
	float hoverAnim{0.0f};
	float cardX{0}, cardY{0}, cardW{0}, cardH{0};
	float btnHover{0.0f};
};

struct CartItem {
	int productId;
	int quantity;
};

// -----------------------------------------------------------------------------
// Global Application State
// -----------------------------------------------------------------------------

class StoreApp {
public:
	std::vector<Product> products;
	std::vector<std::string> categories = {
		"All Products",
		"GPU Accelerators",
		"Neural Silicon",
		"Workstations",
		"Pro Displays",
		"Audio & Media"
	};
	std::string selectedCategory = "All Products";

	std::vector<CartItem> cart;
	bool cartOpen = false;
	float cartAnim = 0.0f; // 0.0 to 1.0

	int quickViewProductId = -1;
	float modalAnim = 0.0f;

	float scrollY = 0.0f;
	float targetScrollY = 0.0f;
	float maxScrollY = 0.0f;

	bool vsync = false;
	int shapesDrawn = 0;

	// Toast notification
	std::string toastText = "";
	float toastTimer = 0.0f;

	// Order confirmation modal
	bool orderConfirmed = false;

	void addToCart(int productId) {
		for (auto& item : cart) {
			if (item.productId == productId) {
				item.quantity++;
				showToast("Increased item quantity in cart");
				return;
			}
		}
		cart.push_back({productId, 1});
		showToast("Added item to shopping cart!");
	}

	void removeFromCart(int productId) {
		cart.erase(std::remove_if(cart.begin(), cart.end(), [productId](const CartItem& ci) {
			return ci.productId == productId;
		}), cart.end());
		showToast("Removed item from cart");
	}

	void updateQuantity(int productId, int delta) {
		for (auto it = cart.begin(); it != cart.end(); ++it) {
			if (it->productId == productId) {
				it->quantity += delta;
				if (it->quantity <= 0) {
					cart.erase(it);
					showToast("Removed item from cart");
				}
				return;
			}
		}
	}

	int getCartCount() const {
		int count = 0;
		for (const auto& item : cart) count += item.quantity;
		return count;
	}

	double getCartSubtotal() const {
		double subtotal = 0.0;
		for (const auto& item : cart) {
			for (const auto& p : products) {
				if (p.id == item.productId) {
					subtotal += p.price * item.quantity;
					break;
				}
			}
		}
		return subtotal;
	}

	void showToast(const std::string& msg) {
		toastText = msg;
		toastTimer = 2.5f; // 2.5 seconds
	}
};

static StoreApp g_app;

// Input tracking
static double g_mouseX = 0.0;
static double g_mouseY = 0.0;
static bool g_mouseDown = false;
static bool g_mouseClicked = false;
static float g_scrollDelta = 0.0f;
static bool g_running = true;

// -----------------------------------------------------------------------------
// Helper Drawing Functions (Glassmorphism, Badges, Buttons)
// -----------------------------------------------------------------------------

static void drawGlassPanel(nisaba::gpu::Context& ctx, float x, float y, float w, float h,
                           float r, nisaba::gpu::Color bgTop, nisaba::gpu::Color bgBottom,
                           nisaba::gpu::Color borderCol) {
	// Drop Shadow
	auto shadow = ctx.boxGradient(x, y + 4.0f, w, h, r * 1.5f, 20.0f,
	                              nisaba::gpu::Color::rgba(0, 0, 0, 140),
	                              nisaba::gpu::Color::rgba(0, 0, 0, 0));
	ctx.beginPath();
	ctx.roundedRect(x - 10.0f, y - 6.0f, w + 20.0f, h + 24.0f, r + 4.0f);
	ctx.fillPaint(shadow);
	ctx.fill();

	// Glass Body
	auto grad = ctx.linearGradient(x, y, x, y + h, bgTop, bgBottom);
	ctx.beginPath();
	ctx.roundedRect(x, y, w, h, r);
	ctx.fillPaint(grad);
	ctx.fill();

	// Border
	ctx.strokeColor(borderCol);
	ctx.strokeWidth(1.2f);
	ctx.stroke();

	g_app.shapesDrawn += 3;
}

static bool drawButton(nisaba::gpu::Context& ctx, float x, float y, float w, float h,
                       const char* text, nisaba::gpu::Color baseColor, float& hoverAnim,
                       const char* fontFace = "sans", float fontSize = 13.0f) {
	bool isHover = (g_mouseX >= x && g_mouseX <= x + w && g_mouseY >= y && g_mouseY <= y + h);
	hoverAnim += ((isHover ? 1.0f : 0.0f) - hoverAnim) * 0.25f;

	nisaba::gpu::Color colA = baseColor;
	nisaba::gpu::Color colB = nisaba::gpu::Color::rgba(
		static_cast<uint8_t>(baseColor.r * 255.0f * 0.75f),
		static_cast<uint8_t>(baseColor.g * 255.0f * 0.75f),
		static_cast<uint8_t>(baseColor.b * 255.0f * 0.75f),
		255
	);

	if (hoverAnim > 0.01f) {
		// Glow
		auto glow = ctx.boxGradient(x, y, w, h, 8.0f, 12.0f,
		                            nisaba::gpu::Color::rgba(
		                                static_cast<uint8_t>(baseColor.r * 255.0f),
		                                static_cast<uint8_t>(baseColor.g * 255.0f),
		                                static_cast<uint8_t>(baseColor.b * 255.0f),
		                                static_cast<uint8_t>(120 * hoverAnim)),
		                            nisaba::gpu::Color::rgba(0, 0, 0, 0));
		ctx.beginPath();
		ctx.roundedRect(x - 6.0f, y - 6.0f, w + 12.0f, h + 12.0f, 12.0f);
		ctx.fillPaint(glow);
		ctx.fill();
	}

	auto bg = ctx.linearGradient(x, y, x, y + h, colA, colB);
	ctx.beginPath();
	ctx.roundedRect(x, y, w, h, 8.0f);
	ctx.fillPaint(bg);
	ctx.fill();

	// Border highlight
	ctx.strokeColor(nisaba::gpu::Color::rgba(255, 255, 255, static_cast<uint8_t>(60 + 100 * hoverAnim)));
	ctx.strokeWidth(1.0f);
	ctx.stroke();

	// Text: High Contrast Color based on background brightness
	float luminance = 0.299f * baseColor.r + 0.587f * baseColor.g + 0.114f * baseColor.b;
	nisaba::gpu::Color textColor = (luminance > 0.45f)
		? nisaba::gpu::Color::rgba(8, 12, 22, 255)       // Dark obsidian text on bright buttons (Cyan, Amber, Emerald)
		: nisaba::gpu::Color::rgba(255, 255, 255, 255);  // Crisp white text on dark buttons

	ctx.fontSize(fontSize);
	ctx.fontFace(fontFace);
	ctx.textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
	ctx.fillColor(textColor);
	ctx.text(x + w * 0.5f, y + h * 0.5f, text);

	g_app.shapesDrawn += 3;
	return (isHover && g_mouseClicked);
}

// -----------------------------------------------------------------------------
// Main Application
// -----------------------------------------------------------------------------

int main(int argc, char** argv) {
	int targetFrames = 0;
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
			targetFrames = std::atoi(argv[++i]);
		}
	}

	// Initialize Nisaba Native Platform Subsystem
	auto plat_res = nisaba::backend_os::Platform::create();
	if (!plat_res.isOk()) {
		std::fprintf(stderr, "Failed to initialize Nisaba native platform: %s\n", plat_res.error().message.c_str());
		return -1;
	}
	auto platform = std::move(plat_res.value());

	nisaba::backend_os::WindowConfig win_cfg;
	win_cfg.title = "NovaStore - Nisaba Sovereign E-Commerce & Image Decoder Showcase";
	win_cfg.width = 1440;
	win_cfg.height = 920;
	win_cfg.resizable = true;
	win_cfg.vsync = g_app.vsync;

	auto win_res = nisaba::backend_os::Window::create(*platform, win_cfg);
	if (!win_res.isOk()) {
		std::fprintf(stderr, "Failed to create Nisaba native window: %s\n", win_res.error().message.c_str());
		return -1;
	}
	auto window = std::move(win_res.value());
	window->makeCurrent();

#ifdef NISABA_GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK && err != GLEW_ERROR_NO_GLX_DISPLAY) {
		std::fprintf(stderr, "Warning/Failed to initialize GLEW: %s (continuing)\n", (const char*)glewGetErrorString(err));
	}
	while (glGetError() != GL_NO_ERROR) {}
#endif

	// Setup Native Event Listeners
	platform->onMouseMove().connect([](float x, float y) {
		g_mouseX = x;
		g_mouseY = y;
	});

	platform->onMouseDown().connect([](float, float, int btn) {
		if (btn == 1) { // Left Button
			g_mouseDown = true;
			g_mouseClicked = true;
		}
	});

	platform->onMouseUp().connect([](float, float, int btn) {
		if (btn == 1) {
			g_mouseDown = false;
		}
	});

	platform->onScroll().connect([](float, float dy) {
		g_scrollDelta += dy;
		g_app.targetScrollY -= dy * 60.0f;
		if (g_app.targetScrollY < 0.0f) g_app.targetScrollY = 0.0f;
		if (g_app.targetScrollY > g_app.maxScrollY) g_app.targetScrollY = g_app.maxScrollY;
	});

	window->onClose().connect([]() {
		g_running = false;
	});

	platform->onKeyDown().connect([&](int key, int) {
		if (key == 27 || key == 0xff1b || key == 9) { // Escape
			if (g_app.orderConfirmed) {
				g_app.orderConfirmed = false;
			} else if (g_app.quickViewProductId != -1) {
				g_app.quickViewProductId = -1;
			} else if (g_app.cartOpen) {
				g_app.cartOpen = false;
			} else {
				g_running = false;
			}
		} else if (key == 'c' || key == 'C' || key == 54) {
			g_app.cartOpen = !g_app.cartOpen;
		} else if (key == 'v' || key == 'V' || key == 55) {
			g_app.vsync = !g_app.vsync;
		}
	});

	// Create Nisaba GPU Context
	auto renderer = std::make_unique<nisaba::gpu::GL3Renderer>();
	auto ctx = std::make_unique<nisaba::gpu::Context>(std::move(renderer), static_cast<int>(nisaba::gpu::CreateFlags::Antialias));

	// Load Fonts
	int fontSans = ctx->createFont("sans", resolveFont("Inter-Regular.ttf"));
	if (fontSans == -1) fontSans = ctx->createFont("sans", resolveFont("NotoSans-Regular.ttf"));

	int fontBold = ctx->createFont("bold", resolveFont("Inter-Regular.ttf"));
	if (fontBold == -1) fontBold = fontSans;

	int fontMono = ctx->createFont("mono", resolveFont("FiraMono-Medium.ttf"));
	int fontArabic = ctx->createFont("arabic", resolveFont("NotoSansArabic.ttf"));
	if (fontArabic == -1) fontArabic = ctx->createFont("arabic", resolveFont("NotoSansArabic-Regular.ttf"));

	(void)fontMono;
	if (fontArabic >= 0) {
		if (fontSans >= 0) ctx->addFallbackFontId(fontSans, fontArabic);
		if (fontBold >= 0) ctx->addFallbackFontId(fontBold, fontArabic);
	}

	// -------------------------------------------------------------------------
	// Catalog Initialization & Native Image Decoding
	// -------------------------------------------------------------------------
	g_app.products = {
		{
			1,
			"Nisaba Quantum GPU V1",
			"GPU Accelerators",
			"showcase/nisaba_gpu_showcase.png",
			"",
			"PNG 100% Bit-Exact",
			nisaba::gpu::Color::rgba(0, 240, 255, 255),
			1899.00,
			5.0f,
			412,
			"FLAGSHIP",
			"Next-Gen 16K Streaming Processor",
			"Engineered for hyper-parallel graphics tessellation and ultra-low latency compute pipelines. Seamlessly decodes and renders lossless textures at lightning speeds.",
			"16,384 Cores | 32GB HBM3e | 1.2 TB/s Bandwidth | PCIe 5.0"
		},
		{
			2,
			"Titan Workstation Pro",
			"Workstations",
			"showcase/showcase_vaxp.jpg",
			"",
			"JPEG SIMD SSE2 (74.4 dB)",
			nisaba::gpu::Color::rgba(255, 180, 0, 255),
			4299.00,
			4.9f,
			188,
			"PRO TIER",
			"Ultra-Density Dual-Socket Rig",
			"High-end workstation powered by SIMD-accelerated image decoders. Decodes high-resolution JPEG imagery with zero quality loss and extreme responsiveness.",
			"128 Cores / 256 Threads | 512GB DDR5-6400 | Quad Liquid Loop"
		},
		{
			3,
			"Sovereign Silicon Core",
			"Neural Silicon",
			"assets/vaxp.png",
			"",
			"Lossless Sovereign PNG",
			nisaba::gpu::Color::rgba(0, 255, 160, 255),
			2450.00,
			5.0f,
			520,
			"BESTSELLER",
			"Zero Third-Party Dependency SoC",
			"Pure C++20 sovereign architecture. 100% bit-for-bit lossless accuracy proven across millions of test pixels with infinite PSNR.",
			"7nm GAAFET | 32MB L3 Cache | Native Nisaba Kernel | 250W TDP"
		},
		{
			4,
			"NovaDisplay 8K Pro",
			"Pro Displays",
			"showcase/nisaba_showcase.png",
			"",
			"Native High-DPI PNG",
			nisaba::gpu::Color::rgba(180, 100, 255, 255),
			1299.00,
			4.8f,
			95,
			"NEW RELEASE",
			"Color-Accurate Master Reference",
			"Factory calibrated for 99.8% DCI-P3 gamut. Showcases Nisaba's vector and raster capabilities in pristine, crisp clarity.",
			"32-inch 7680x4320 | 165Hz Refresh | 1200 nits Peak | Mini-LED"
		},
		{
			5,
			"Neural Mesh Co-Processor",
			"Neural Silicon",
			"showcase/nisaba_mesh_showcase.png",
			"",
			"Vector Mesh PNG",
			nisaba::gpu::Color::rgba(255, 80, 140, 255),
			849.00,
			4.7f,
			64,
			"SPECIAL EDITION",
			"Hardware-Accelerated Tessellation",
			"Dedicated co-processor handling real-time Bézier curves, non-linear mesh deformation, and multi-threaded shape rasterization.",
			"64 Co-Processing Tiles | 16GB Fast SRAM | 0.4ms Latency"
		},
		{
			6,
			"QOI Stream Accelerator",
			"GPU Accelerators",
			"showcase/showcase_vaxp.qoi",
			"",
			"QOI Ultra-Fast Lossless",
			nisaba::gpu::Color::rgba(255, 220, 50, 255),
			599.00,
			5.0f,
			310,
			"HOT ITEM",
			"Zero-Overhead Lossless Streaming",
			"Harnesses the sheer speed of Nisaba's native QOI decoder for instant UI asset streaming and real-time texture swapping.",
			"10x Faster Decode | Lossless 24-bit RGB | Zero Memory Fragmentation"
		},
		{
			7,
			"Audiophile Studio DAC",
			"Audio & Media",
			"showcase/nisaba_media_showcase.png",
			"",
			"Lossless Media PNG",
			nisaba::gpu::Color::rgba(0, 190, 255, 255),
			799.00,
			4.9f,
			142,
			"STUDIO GRADE",
			"Pure Discrete Class-A Architecture",
			"Engineered for pristine studio monitoring and high-fidelity audio streams. Decoded and rendered cleanly with Nisaba media subsystem.",
			"384kHz / 32-bit Native | SNR 132dB | Balanced XLR & 4.4mm Outputs"
		},
		{
			8,
			"Perspective Spatial Sensor",
			"Pro Displays",
			"showcase/nisaba_perspective_showcase.png",
			"",
			"Spatial Transform PNG",
			nisaba::gpu::Color::rgba(0, 255, 230, 255),
			649.00,
			4.8f,
			78,
			"VR READY",
			"6-DOF Sub-Millimeter Spatial Tracker",
			"Ultra-precise spatial awareness camera array with real-time perspective distortion correction and hardware matrix acceleration.",
			"240 FPS Camera Feed | 0.2mm Tracking Error | USB-C 40Gbps"
		}
	};

	// -------------------------------------------------------------------------
	// Load Images via Nisaba's Sovereign Image Engine
	// -------------------------------------------------------------------------
	std::printf("====================================================\n");
	std::printf("NovaStore: Loading Product Images via Nisaba Native Decoders\n");
	std::printf("====================================================\n");

	for (auto& p : g_app.products) {
		p.resolvedPath = resolveResource(p.imageRelPath);
		auto t0 = std::chrono::high_resolution_clock::now();

		// Context::createImage natively triggers nisaba::image::load_image_file!
		p.textureId = ctx->createImage(p.resolvedPath.c_str(), nisaba::gpu::ImageFlags::ImageGenerateMipmaps);
		auto t1 = std::chrono::high_resolution_clock::now();
		p.decodeTimeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

		if (p.textureId > 0) {
			ctx->imageSize(p.textureId, p.imgW, p.imgH);
			p.isLoaded = true;
			std::printf("  [OK] %-26s | %4dx%-4d | %6.2f ms | TextID: %d | %s\n",
			            p.name.c_str(), p.imgW, p.imgH, p.decodeTimeMs, p.textureId, p.formatBadge.c_str());
		} else {
			std::printf("  [FAIL] Could not load image: %s (%s)\n", p.imageRelPath.c_str(), p.resolvedPath.c_str());
		}
	}
	std::printf("====================================================\n");

	// Initial cart item
	g_app.addToCart(1);
	g_app.addToCart(3);

	// Performance Tracking
	double benchStartTime = platform->getTime();
	double lastTime = benchStartTime;
	double smoothedFps = 60.0;
	double smoothedCpuMs = 1.0;
	int frameCount = 0;

	// -------------------------------------------------------------------------
	// Main Render & Interaction Loop
	// -------------------------------------------------------------------------
	while (g_running) {
		if (!platform->pollEvents()) {
			break;
		}

		double now = platform->getTime();
		float dt = static_cast<float>(now - lastTime);
		if (dt > 0.1f) dt = 0.1f;
		lastTime = now;

		// Smooth scroll interpolation
		g_app.scrollY += (g_app.targetScrollY - g_app.scrollY) * std::min(dt * 12.0f, 1.0f);

		// Smooth drawer animation
		float targetCartAnim = g_app.cartOpen ? 1.0f : 0.0f;
		g_app.cartAnim += (targetCartAnim - g_app.cartAnim) * std::min(dt * 14.0f, 1.0f);

		// Smooth modal animation
		float targetModalAnim = (g_app.quickViewProductId != -1 || g_app.orderConfirmed) ? 1.0f : 0.0f;
		g_app.modalAnim += (targetModalAnim - g_app.modalAnim) * std::min(dt * 16.0f, 1.0f);

		// Toast timer
		if (g_app.toastTimer > 0.0f) {
			g_app.toastTimer -= dt;
		}

		auto winSize = window->getSize();
		auto fbSize = window->getDrawableSize();
		int winW = static_cast<int>(winSize.width);
		int winH = static_cast<int>(winSize.height);
		int fbW = static_cast<int>(fbSize.width);
		int fbH = static_cast<int>(fbSize.height);
		if (winW <= 0 || winH <= 0) continue;
		float pxRatio = static_cast<float>(fbW) / static_cast<float>(winW);

		glViewport(0, 0, fbW, fbH);
		glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ctx->beginFrame(winW, winH, pxRatio);
		g_app.shapesDrawn = 0;

		// ---------------------------------------------------------------------
		// 1. Ambient Background Atmosphere
		// ---------------------------------------------------------------------
		{
			// Subtle top-left cyan glow
			auto glow1 = ctx->radialGradient(200.0f, 150.0f, 50.0f, 600.0f,
			                                 nisaba::gpu::Color::rgba(0, 180, 255, 18),
			                                 nisaba::gpu::Color::rgba(0, 0, 0, 0));
			ctx->beginPath();
			ctx->rect(0.0f, 0.0f, static_cast<float>(winW), static_cast<float>(winH));
			ctx->fillPaint(glow1);
			ctx->fill();

			// Subtle bottom-right purple glow
			auto glow2 = ctx->radialGradient(static_cast<float>(winW) - 200.0f, static_cast<float>(winH) - 150.0f, 50.0f, 700.0f,
			                                 nisaba::gpu::Color::rgba(130, 40, 255, 15),
			                                 nisaba::gpu::Color::rgba(0, 0, 0, 0));
			ctx->beginPath();
			ctx->rect(0.0f, 0.0f, static_cast<float>(winW), static_cast<float>(winH));
			ctx->fillPaint(glow2);
			ctx->fill();
			g_app.shapesDrawn += 2;
		}

		// ---------------------------------------------------------------------
		// 2. Main Scrollable Content Area
		// ---------------------------------------------------------------------
		float headerH = 80.0f;
		float navH = 50.0f;
		float contentTop = headerH + navH;
		float contentW = std::min(static_cast<float>(winW) - 80.0f, 1320.0f);
		float contentX = (static_cast<float>(winW) - contentW) * 0.5f;

		ctx->save();
		// Scissor content area so scrolling doesn't overlap header
		ctx->scissor(0.0f, contentTop, static_cast<float>(winW), static_cast<float>(winH) - contentTop);

		float curY = contentTop + 24.0f - g_app.scrollY;

		// --- Hero Featured Product Banner ---
		if (g_app.selectedCategory == "All Products") {
			float heroH = 260.0f;
			drawGlassPanel(*ctx, contentX, curY, contentW, heroH, 18.0f,
			               nisaba::gpu::Color::rgba(18, 24, 38, 220),
			               nisaba::gpu::Color::rgba(10, 14, 24, 240),
			               nisaba::gpu::Color::rgba(0, 220, 255, 90));

			// Flagship Product details in Hero
			const Product& heroProd = g_app.products[0];

			// Hero Image on Left
			float heroImgW = 340.0f;
			float heroImgH = 220.0f;
			float heroImgX = contentX + 24.0f;
			float heroImgY = curY + 20.0f;

			if (heroProd.isLoaded) {
				auto pat = ctx->imagePattern(heroImgX, heroImgY, heroImgW, heroImgH, 0.0f, heroProd.textureId, 1.0f);
				ctx->beginPath();
				ctx->roundedRect(heroImgX, heroImgY, heroImgW, heroImgH, 12.0f);
				ctx->fillPaint(pat);
				ctx->fill();

				ctx->strokeColor(nisaba::gpu::Color::rgba(0, 240, 255, 120));
				ctx->strokeWidth(1.5f);
				ctx->stroke();
			}

			// Hero Text on Right
			float heroTextX = heroImgX + heroImgW + 36.0f;
			float heroTextY = curY + 30.0f;

			// Pill Badge
			ctx->fontSize(11.0f);
			ctx->fontFace("bold");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 255));
			ctx->text(heroTextX, heroTextY, "★ FEATURED HARDWARE • NISABA HIGH-PERFORMANCE IMAGE DECODER");

			ctx->fontSize(28.0f);
			ctx->fontFace("bold");
			ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
			ctx->text(heroTextX, heroTextY + 24.0f, heroProd.name.c_str());

			ctx->fontSize(14.0f);
			ctx->fontFace("sans");
			ctx->fillColor(nisaba::gpu::Color::rgba(160, 180, 205, 220));
			ctx->text(heroTextX, heroTextY + 62.0f, heroProd.description.c_str());

			ctx->fontSize(13.0f);
			ctx->fontFace("mono");
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 255, 180, 240));
			ctx->text(heroTextX, heroTextY + 115.0f, heroProd.specs.c_str());

			// Price & CTA Button
			char priceBuf[32];
			std::snprintf(priceBuf, sizeof(priceBuf), "$%.2f", heroProd.price);
			ctx->fontSize(26.0f);
			ctx->fontFace("bold");
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 255));
			ctx->text(heroTextX, heroTextY + 160.0f, priceBuf);

			static float heroBtnHover = 0.0f;
			if (drawButton(*ctx, heroTextX + 180.0f, heroTextY + 155.0f, 150.0f, 42.0f, "Add to Cart",
			               nisaba::gpu::Color::rgba(0, 180, 230, 255), heroBtnHover, "bold", 14.0f)) {
				g_app.addToCart(heroProd.id);
			}

			static float heroViewHover = 0.0f;
			if (drawButton(*ctx, heroTextX + 345.0f, heroTextY + 155.0f, 130.0f, 42.0f, "Quick View",
			               nisaba::gpu::Color::rgba(35, 45, 65, 255), heroViewHover, "sans", 13.0f)) {
				g_app.quickViewProductId = heroProd.id;
			}

			curY += heroH + 32.0f;
		}

		// --- Section Title & Active Filter Summary ---
		ctx->fontSize(20.0f);
		ctx->fontFace("bold");
		ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
		ctx->fillColor(nisaba::gpu::Color::rgba(240, 245, 255, 255));
		ctx->text(contentX, curY, g_app.selectedCategory.c_str());

		ctx->fontSize(13.0f);
		ctx->fontFace("mono");
		ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Top);
		ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 220));
		ctx->text(contentX + contentW, curY + 4.0f, "⚡ All Textures Decoded by Sovereign Nisaba C++20");

		curY += 36.0f;

		// --- Product Cards Grid (3 or 4 Columns) ---
		int numCols = (contentW > 1100.0f) ? 4 : 3;
		float cardGap = 20.0f;
		float cardW = (contentW - (numCols - 1) * cardGap) / numCols;
		float cardH = 390.0f;

		int visibleIdx = 0;
		for (auto& prod : g_app.products) {
			if (g_app.selectedCategory != "All Products" && prod.category != g_app.selectedCategory) {
				continue;
			}

			int col = visibleIdx % numCols;
			int row = visibleIdx / numCols;
			float cx = contentX + col * (cardW + cardGap);
			float cy = curY + row * (cardH + cardGap);

			prod.cardX = cx;
			prod.cardY = cy;
			prod.cardW = cardW;
			prod.cardH = cardH;

			// Hover detection
			bool isHover = (g_mouseX >= cx && g_mouseX <= cx + cardW &&
			                g_mouseY >= cy && g_mouseY <= cy + cardH &&
			                g_mouseY >= contentTop &&
			                g_app.quickViewProductId == -1 && !g_app.cartOpen);
			prod.hoverAnim += ((isHover ? 1.0f : 0.0f) - prod.hoverAnim) * 0.2f;

			float lift = prod.hoverAnim * 6.0f;
			float drawY = cy - lift;

			// Draw Card Glass Panel
			drawGlassPanel(*ctx, cx, drawY, cardW, cardH, 14.0f,
			               nisaba::gpu::Color::rgba(18, 22, 34, 230),
			               nisaba::gpu::Color::rgba(11, 14, 22, 245),
			               nisaba::gpu::Color::rgba(
			                   static_cast<uint8_t>(35 + 180 * prod.hoverAnim),
			                   static_cast<uint8_t>(45 + 180 * prod.hoverAnim),
			                   static_cast<uint8_t>(65 + 190 * prod.hoverAnim),
			                   static_cast<uint8_t>(100 + 100 * prod.hoverAnim)));

			// Image Container (Top of card)
			float imgPad = 12.0f;
			float imgX = cx + imgPad;
			float imgY = drawY + imgPad;
			float imgW = cardW - imgPad * 2.0f;
			float imgH = 190.0f;

			if (prod.isLoaded) {
				auto pat = ctx->imagePattern(imgX, imgY, imgW, imgH, 0.0f, prod.textureId, 1.0f);
				ctx->beginPath();
				ctx->roundedRect(imgX, imgY, imgW, imgH, 10.0f);
				ctx->fillPaint(pat);
				ctx->fill();

				// Image border
				ctx->strokeColor(nisaba::gpu::Color::rgba(255, 255, 255, 25));
				ctx->strokeWidth(1.0f);
				ctx->stroke();
			}

			// Format Badge on Top Left of Image
			float badgeW = 140.0f;
			float badgeH = 22.0f;
			float badgeX = imgX + 8.0f;
			float badgeY = imgY + 8.0f;

			auto badgeBg = ctx->linearGradient(badgeX, badgeY, badgeX + badgeW, badgeY,
			                                   nisaba::gpu::Color::rgba(10, 14, 24, 230),
			                                   nisaba::gpu::Color::rgba(18, 24, 38, 230));
			ctx->beginPath();
			ctx->roundedRect(badgeX, badgeY, badgeW, badgeH, 6.0f);
			ctx->fillPaint(badgeBg);
			ctx->fill();
			ctx->strokeColor(prod.badgeColor);
			ctx->strokeWidth(1.0f);
			ctx->stroke();

			ctx->fontSize(10.0f);
			ctx->fontFace("mono");
			ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
			ctx->fillColor(prod.badgeColor);
			ctx->text(badgeX + badgeW * 0.5f, badgeY + badgeH * 0.5f, prod.formatBadge.c_str());

			// Decode Duration Pill on Top Right of Image
			char timePill[32];
			std::snprintf(timePill, sizeof(timePill), "%.1f ms", prod.decodeTimeMs);
			float timeW = 55.0f;
			float timeX = imgX + imgW - timeW - 8.0f;
			ctx->beginPath();
			ctx->roundedRect(timeX, badgeY, timeW, badgeH, 6.0f);
			ctx->fillColor(nisaba::gpu::Color::rgba(10, 14, 24, 210));
			ctx->fill();
			ctx->strokeColor(nisaba::gpu::Color::rgba(0, 255, 180, 160));
			ctx->strokeWidth(1.0f);
			ctx->stroke();

			ctx->fontSize(10.0f);
			ctx->fontFace("mono");
			ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 255, 180, 255));
			ctx->text(timeX + timeW * 0.5f, badgeY + badgeH * 0.5f, timePill);

			// Product Category
			float textY = imgY + imgH + 14.0f;
			ctx->fontSize(11.0f);
			ctx->fontFace("sans");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
			ctx->fillColor(nisaba::gpu::Color::rgba(130, 150, 175, 200));
			ctx->text(cx + imgPad, textY, prod.category.c_str());

			// Rating stars
			ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Top);
			ctx->fillColor(nisaba::gpu::Color::rgba(255, 200, 50, 240));
			ctx->text(cx + cardW - imgPad, textY, "★★★★★ 4.9");

			// Product Name
			textY += 18.0f;
			ctx->fontSize(16.0f);
			ctx->fontFace("bold");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
			ctx->fillColor(nisaba::gpu::Color::rgba(240, 245, 255, 255));
			ctx->text(cx + imgPad, textY, prod.name.c_str());

			// Subtitle / Specs
			textY += 24.0f;
			ctx->fontSize(12.0f);
			ctx->fontFace("sans");
			ctx->fillColor(nisaba::gpu::Color::rgba(150, 165, 190, 180));
			ctx->text(cx + imgPad, textY, prod.subtitle.c_str());

			// Bottom Area: Price & Add to Cart
			float bottomY = drawY + cardH - 46.0f;
			char cardPrice[32];
			std::snprintf(cardPrice, sizeof(cardPrice), "$%.2f", prod.price);
			ctx->fontSize(18.0f);
			ctx->fontFace("bold");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 255));
			ctx->text(cx + imgPad, bottomY + 16.0f, cardPrice);

			float btnW = 100.0f;
			float btnH = 34.0f;
			float btnX = cx + cardW - imgPad - btnW;
			float btnY = bottomY;

			if (drawButton(*ctx, btnX, btnY, btnW, btnH, "+ Cart",
			               nisaba::gpu::Color::rgba(0, 160, 220, 255), prod.btnHover, "bold", 12.0f)) {
				g_app.addToCart(prod.id);
			}

			// Click on card opens Quick View
			if (isHover && g_mouseClicked && !(g_mouseX >= btnX && g_mouseX <= btnX + btnW && g_mouseY >= btnY && g_mouseY <= btnY + btnH)) {
				g_app.quickViewProductId = prod.id;
			}

			visibleIdx++;
		}

		int totalRows = (visibleIdx + numCols - 1) / numCols;
		float totalGridH = totalRows * (cardH + cardGap);
		curY += totalGridH + 60.0f;

		g_app.maxScrollY = std::max(0.0f, curY + g_app.scrollY - (static_cast<float>(winH) - 40.0f));

		ctx->restore(); // End scissor

		// ---------------------------------------------------------------------
		// 3. Header & Navigation Bar (Fixed Glass Header)
		// ---------------------------------------------------------------------
		// Top Header Bar
		drawGlassPanel(*ctx, 0.0f, 0.0f, static_cast<float>(winW), headerH, 0.0f,
		               nisaba::gpu::Color::rgba(10, 13, 20, 245),
		               nisaba::gpu::Color::rgba(8, 10, 16, 255),
		               nisaba::gpu::Color::rgba(255, 255, 255, 20));

		// Store Logo & Branding
		float logoX = 36.0f;
		float logoY = 22.0f;

		// Glowing Icon Box
		auto logoGrad = ctx->linearGradient(logoX, logoY, logoX + 36.0f, logoY + 36.0f,
		                                   nisaba::gpu::Color::rgba(0, 240, 255, 255),
		                                   nisaba::gpu::Color::rgba(130, 40, 255, 255));
		ctx->beginPath();
		ctx->roundedRect(logoX, logoY, 36.0f, 36.0f, 10.0f);
		ctx->fillPaint(logoGrad);
		ctx->fill();

		ctx->fontSize(20.0f);
		ctx->fontFace("bold");
		ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
		ctx->fillColor(nisaba::gpu::Color::rgba(10, 13, 20, 255));
		ctx->text(logoX + 18.0f, logoY + 18.0f, "N");

		ctx->fontSize(22.0f);
		ctx->fontFace("bold");
		ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
		ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
		ctx->text(logoX + 48.0f, logoY - 2.0f, "NOVASTORE");

		ctx->fontSize(11.0f);
		ctx->fontFace("mono");
		ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 220));
		ctx->text(logoX + 48.0f, logoY + 22.0f, "Nisaba C++20 Sovereign Image Engine");

		// Live Telemetry Badge (Center Header)
		float statsW = 260.0f;
		float statsX = (static_cast<float>(winW) - statsW) * 0.5f;
		float statsY = 22.0f;
		ctx->beginPath();
		ctx->roundedRect(statsX, statsY, statsW, 36.0f, 18.0f);
		ctx->fillColor(nisaba::gpu::Color::rgba(18, 24, 36, 210));
		ctx->fill();
		ctx->strokeColor(nisaba::gpu::Color::rgba(0, 255, 180, 80));
		ctx->strokeWidth(1.0f);
		ctx->stroke();

		char fpsPill[64];
		std::snprintf(fpsPill, sizeof(fpsPill), "⚡ %.1f FPS  |  %.2f ms", smoothedFps, smoothedCpuMs);
		ctx->fontSize(12.0f);
		ctx->fontFace("mono");
		ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
		ctx->fillColor(nisaba::gpu::Color::rgba(0, 255, 180, 255));
		ctx->text(statsX + statsW * 0.5f, statsY + 18.0f, fpsPill);

		// Shopping Cart Button (Top Right)
		float cartBtnW = 160.0f;
		float cartBtnH = 40.0f;
		float cartBtnX = static_cast<float>(winW) - cartBtnW - 36.0f;
		float cartBtnY = 20.0f;

		char cartLabel[64];
		std::snprintf(cartLabel, sizeof(cartLabel), "🛒 Cart (%d) $%.0f", g_app.getCartCount(), g_app.getCartSubtotal());
		static float cartBtnHover = 0.0f;
		if (drawButton(*ctx, cartBtnX, cartBtnY, cartBtnW, cartBtnH, cartLabel,
		               nisaba::gpu::Color::rgba(0, 180, 230, 255), cartBtnHover, "bold", 13.0f)) {
			g_app.cartOpen = !g_app.cartOpen;
		}

		// Category Navigation Bar (Below Header)
		drawGlassPanel(*ctx, 0.0f, headerH, static_cast<float>(winW), navH, 0.0f,
		               nisaba::gpu::Color::rgba(14, 18, 28, 240),
		               nisaba::gpu::Color::rgba(10, 13, 22, 250),
		               nisaba::gpu::Color::rgba(255, 255, 255, 15));

		float catX = contentX;
		float catY = headerH + 10.0f;
		for (const auto& cat : g_app.categories) {
			float catTextW = static_cast<float>(cat.length()) * 8.5f + 24.0f;
			bool isSelected = (g_app.selectedCategory == cat);
			bool isHover = (g_mouseX >= catX && g_mouseX <= catX + catTextW &&
			                g_mouseY >= catY && g_mouseY <= catY + 30.0f);

			if (isSelected) {
				ctx->beginPath();
				ctx->roundedRect(catX, catY, catTextW, 30.0f, 15.0f);
				ctx->fillColor(nisaba::gpu::Color::rgba(0, 200, 255, 240));
				ctx->fill();

				ctx->fontSize(13.0f);
				ctx->fontFace("bold");
				ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
				ctx->fillColor(nisaba::gpu::Color::rgba(10, 14, 22, 255));
				ctx->text(catX + catTextW * 0.5f, catY + 15.0f, cat.c_str());
			} else {
				if (isHover) {
					ctx->beginPath();
					ctx->roundedRect(catX, catY, catTextW, 30.0f, 15.0f);
					ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 20));
					ctx->fill();
				}

				ctx->fontSize(13.0f);
				ctx->fontFace("sans");
				ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
				ctx->fillColor(isHover ? nisaba::gpu::Color::rgba(255, 255, 255, 255)
				                       : nisaba::gpu::Color::rgba(170, 185, 210, 220));
				ctx->text(catX + catTextW * 0.5f, catY + 15.0f, cat.c_str());

				if (isHover && g_mouseClicked) {
					g_app.selectedCategory = cat;
					g_app.targetScrollY = 0.0f;
				}
			}

			catX += catTextW + 12.0f;
		}

		// ---------------------------------------------------------------------
		// 4. Slide-Out Shopping Cart Drawer
		// ---------------------------------------------------------------------
		if (g_app.cartAnim > 0.01f) {
			float drawerW = 420.0f;
			float drawerX = static_cast<float>(winW) - drawerW * g_app.cartAnim;

			// Dim background
			ctx->beginPath();
			ctx->rect(0.0f, 0.0f, static_cast<float>(winW), static_cast<float>(winH));
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 0, 0, static_cast<uint8_t>(140 * g_app.cartAnim)));
			ctx->fill();

			if (g_mouseClicked && g_mouseX < drawerX) {
				g_app.cartOpen = false;
			}

			// Drawer Panel
			drawGlassPanel(*ctx, drawerX, 0.0f, drawerW, static_cast<float>(winH), 0.0f,
			               nisaba::gpu::Color::rgba(16, 20, 32, 250),
			               nisaba::gpu::Color::rgba(10, 13, 22, 255),
			               nisaba::gpu::Color::rgba(0, 220, 255, 80));

			// Drawer Header
			float dHeadY = 28.0f;
			ctx->fontSize(20.0f);
			ctx->fontFace("bold");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
			ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
			ctx->text(drawerX + 28.0f, dHeadY, "Shopping Cart");

			// Close Button [X]
			float closeX = drawerX + drawerW - 56.0f;
			float closeY = dHeadY - 4.0f;
			bool closeHover = (g_mouseX >= closeX && g_mouseX <= closeX + 32.0f &&
			                   g_mouseY >= closeY && g_mouseY <= closeY + 32.0f);
			ctx->fontSize(18.0f);
			ctx->fontFace("bold");
			ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
			ctx->fillColor(closeHover ? nisaba::gpu::Color::rgba(255, 100, 100, 255)
			                          : nisaba::gpu::Color::rgba(180, 190, 210, 200));
			ctx->text(closeX + 16.0f, closeY + 16.0f, "✕");

			if (closeHover && g_mouseClicked) {
				g_app.cartOpen = false;
			}

			// Cart Item List
			float itemY = dHeadY + 50.0f;
			float itemH = 76.0f;

			if (g_app.cart.empty()) {
				ctx->fontSize(14.0f);
				ctx->fontFace("sans");
				ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
				ctx->fillColor(nisaba::gpu::Color::rgba(140, 160, 185, 180));
				ctx->text(drawerX + drawerW * 0.5f, itemY + 80.0f, "Your shopping cart is empty.");
			} else {
				for (const auto& item : g_app.cart) {
					const Product* pProd = nullptr;
					for (const auto& p : g_app.products) {
						if (p.id == item.productId) {
							pProd = &p;
							break;
						}
					}
					if (!pProd) continue;

					// Card background
					ctx->beginPath();
					ctx->roundedRect(drawerX + 20.0f, itemY, drawerW - 40.0f, itemH, 10.0f);
					ctx->fillColor(nisaba::gpu::Color::rgba(22, 28, 44, 210));
					ctx->fill();
					ctx->strokeColor(nisaba::gpu::Color::rgba(255, 255, 255, 20));
					ctx->strokeWidth(1.0f);
					ctx->stroke();

					// Thumbnail image
					float thW = 56.0f;
					float thH = 56.0f;
					float thX = drawerX + 30.0f;
					float thY = itemY + 10.0f;

					if (pProd->isLoaded) {
						auto pat = ctx->imagePattern(thX, thY, thW, thH, 0.0f, pProd->textureId, 1.0f);
						ctx->beginPath();
						ctx->roundedRect(thX, thY, thW, thH, 6.0f);
						ctx->fillPaint(pat);
						ctx->fill();
					}

					// Product Name & Price
					float textX = thX + thW + 14.0f;
					ctx->fontSize(13.0f);
					ctx->fontFace("bold");
					ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
					ctx->fillColor(nisaba::gpu::Color::rgba(240, 245, 255, 255));
					ctx->text(textX, itemY + 12.0f, pProd->name.c_str());

					char pricePer[32];
					std::snprintf(pricePer, sizeof(pricePer), "$%.2f ea", pProd->price);
					ctx->fontSize(12.0f);
					ctx->fontFace("mono");
					ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 220));
					ctx->text(textX, itemY + 32.0f, pricePer);

					// Quantity Controls (- / +)
					float qBtnW = 24.0f;
					float qBtnH = 24.0f;
					float qY = itemY + 44.0f;

					// Minus
					bool minusHover = (g_mouseX >= textX && g_mouseX <= textX + qBtnW &&
					                   g_mouseY >= qY && g_mouseY <= qY + qBtnH);
					ctx->beginPath();
					ctx->roundedRect(textX, qY, qBtnW, qBtnH, 4.0f);
					ctx->fillColor(minusHover ? nisaba::gpu::Color::rgba(255, 80, 80, 200)
					                          : nisaba::gpu::Color::rgba(40, 50, 75, 220));
					ctx->fill();
					ctx->fontSize(14.0f);
					ctx->fontFace("bold");
					ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
					ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
					ctx->text(textX + qBtnW * 0.5f, qY + qBtnH * 0.5f, "-");
					if (minusHover && g_mouseClicked) {
						g_app.updateQuantity(pProd->id, -1);
					}

					// Quantity text
					char qText[16];
					std::snprintf(qText, sizeof(qText), "%d", item.quantity);
					ctx->fontSize(12.0f);
					ctx->fontFace("mono");
					ctx->text(textX + qBtnW + 16.0f, qY + qBtnH * 0.5f, qText);

					// Plus
					float pX = textX + qBtnW + 32.0f;
					bool plusHover = (g_mouseX >= pX && g_mouseX <= pX + qBtnW &&
					                  g_mouseY >= qY && g_mouseY <= qY + qBtnH);
					ctx->beginPath();
					ctx->roundedRect(pX, qY, qBtnW, qBtnH, 4.0f);
					ctx->fillColor(plusHover ? nisaba::gpu::Color::rgba(0, 200, 255, 200)
					                         : nisaba::gpu::Color::rgba(40, 50, 75, 220));
					ctx->fill();
					ctx->fontSize(14.0f);
					ctx->fontFace("bold");
					ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
					ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
					ctx->text(pX + qBtnW * 0.5f, qY + qBtnH * 0.5f, "+");
					if (plusHover && g_mouseClicked) {
						g_app.updateQuantity(pProd->id, 1);
					}

					// Line Total
					char lineTot[32];
					std::snprintf(lineTot, sizeof(lineTot), "$%.2f", pProd->price * item.quantity);
					ctx->fontSize(14.0f);
					ctx->fontFace("bold");
					ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Middle);
					ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
					ctx->text(drawerX + drawerW - 32.0f, itemY + itemH * 0.5f, lineTot);

					itemY += itemH + 12.0f;
				}
			}

			// Cart Footer & Checkout
			float footH = 180.0f;
			float footY = static_cast<float>(winH) - footH;

			drawGlassPanel(*ctx, drawerX, footY, drawerW, footH, 0.0f,
			               nisaba::gpu::Color::rgba(14, 18, 28, 250),
			               nisaba::gpu::Color::rgba(8, 10, 16, 255),
			               nisaba::gpu::Color::rgba(255, 255, 255, 25));

			double subtotal = g_app.getCartSubtotal();
			double tax = subtotal * 0.085;
			double grandTotal = subtotal + tax;

			char subBuf[32], taxBuf[32], totBuf[32];
			std::snprintf(subBuf, sizeof(subBuf), "$%.2f", subtotal);
			std::snprintf(taxBuf, sizeof(taxBuf), "$%.2f", tax);
			std::snprintf(totBuf, sizeof(totBuf), "$%.2f", grandTotal);

			ctx->fontSize(12.0f);
			ctx->fontFace("sans");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(150, 170, 195, 200));
			ctx->text(drawerX + 28.0f, footY + 24.0f, "Subtotal");
			ctx->text(drawerX + 28.0f, footY + 48.0f, "Estimated Tax (8.5%)");
			ctx->text(drawerX + 28.0f, footY + 72.0f, "Shipping & Handling");

			ctx->fontFace("mono");
			ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Middle);
			ctx->text(drawerX + drawerW - 28.0f, footY + 24.0f, subBuf);
			ctx->text(drawerX + drawerW - 28.0f, footY + 48.0f, taxBuf);
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 255, 180, 255));
			ctx->text(drawerX + drawerW - 28.0f, footY + 72.0f, "FREE");

			ctx->fontSize(18.0f);
			ctx->fontFace("bold");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
			ctx->text(drawerX + 28.0f, footY + 104.0f, "Total Due");

			ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 255));
			ctx->text(drawerX + drawerW - 28.0f, footY + 104.0f, totBuf);

			// Checkout Button
			static float chkHover = 0.0f;
			if (drawButton(*ctx, drawerX + 28.0f, footY + 130.0f, drawerW - 56.0f, 38.0f,
			               "Proceed to Checkout →",
			               nisaba::gpu::Color::rgba(0, 200, 120, 255), chkHover, "bold", 14.0f)) {
				if (!g_app.cart.empty()) {
					g_app.orderConfirmed = true;
					g_app.cartOpen = false;
				}
			}
		}

		// ---------------------------------------------------------------------
		// 5. Quick View Product Modal & Nisaba Decoder Telemetry
		// ---------------------------------------------------------------------
		if (g_app.quickViewProductId != -1 && g_app.modalAnim > 0.01f) {
			const Product* pProd = nullptr;
			for (const auto& p : g_app.products) {
				if (p.id == g_app.quickViewProductId) {
					pProd = &p;
					break;
				}
			}

			if (pProd) {
				// Dim background
				ctx->beginPath();
				ctx->rect(0.0f, 0.0f, static_cast<float>(winW), static_cast<float>(winH));
				ctx->fillColor(nisaba::gpu::Color::rgba(0, 0, 0, static_cast<uint8_t>(180 * g_app.modalAnim)));
				ctx->fill();

				float modalW = 860.0f;
				float modalH = 540.0f;
				float modalX = (static_cast<float>(winW) - modalW) * 0.5f;
				float modalY = (static_cast<float>(winH) - modalH) * 0.5f;

				drawGlassPanel(*ctx, modalX, modalY, modalW, modalH, 18.0f,
				               nisaba::gpu::Color::rgba(18, 22, 34, 252),
				               nisaba::gpu::Color::rgba(10, 13, 20, 255),
				               nisaba::gpu::Color::rgba(0, 240, 255, 120));

				// Close Button [X]
				float mCloseX = modalX + modalW - 48.0f;
				float mCloseY = modalY + 20.0f;
				bool mCloseHover = (g_mouseX >= mCloseX && g_mouseX <= mCloseX + 28.0f &&
				                    g_mouseY >= mCloseY && g_mouseY <= mCloseY + 28.0f);
				ctx->fontSize(20.0f);
				ctx->fontFace("bold");
				ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
				ctx->fillColor(mCloseHover ? nisaba::gpu::Color::rgba(255, 80, 80, 255)
				                           : nisaba::gpu::Color::rgba(170, 185, 210, 200));
				ctx->text(mCloseX + 14.0f, mCloseY + 14.0f, "✕");
				if ((mCloseHover && g_mouseClicked) || (g_mouseClicked && (g_mouseX < modalX || g_mouseX > modalX + modalW || g_mouseY < modalY || g_mouseY > modalY + modalH))) {
					g_app.quickViewProductId = -1;
				}

				// Left: Large High-Res Product Image
				float mImgX = modalX + 32.0f;
				float mImgY = modalY + 32.0f;
				float mImgW = 380.0f;
				float mImgH = 476.0f;

				if (pProd->isLoaded) {
					auto pat = ctx->imagePattern(mImgX, mImgY, mImgW, mImgH, 0.0f, pProd->textureId, 1.0f);
					ctx->beginPath();
					ctx->roundedRect(mImgX, mImgY, mImgW, mImgH, 12.0f);
					ctx->fillPaint(pat);
					ctx->fill();
					ctx->strokeColor(nisaba::gpu::Color::rgba(0, 240, 255, 100));
					ctx->strokeWidth(1.2f);
					ctx->stroke();
				}

				// Right: Details & Telemetry
				float rX = mImgX + mImgW + 36.0f;
				float rY = modalY + 36.0f;
				float rW = modalW - mImgW - 100.0f;

				// Category & Rating
				ctx->fontSize(12.0f);
				ctx->fontFace("sans");
				ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
				ctx->fillColor(pProd->badgeColor);
				ctx->text(rX, rY, pProd->category.c_str());

				ctx->fontSize(24.0f);
				ctx->fontFace("bold");
				ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
				ctx->text(rX, rY + 22.0f, pProd->name.c_str());

				char mPrice[32];
				std::snprintf(mPrice, sizeof(mPrice), "$%.2f", pProd->price);
				ctx->fontSize(24.0f);
				ctx->fontFace("bold");
				ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 255));
				ctx->text(rX, rY + 60.0f, mPrice);

				ctx->fontSize(13.0f);
				ctx->fontFace("sans");
				ctx->fillColor(nisaba::gpu::Color::rgba(165, 180, 205, 220));
				ctx->text(rX, rY + 100.0f, pProd->description.c_str());

				// --- Nisaba Image Decoder Technical Telemetry Box ---
				float boxY = rY + 180.0f;
				float boxH = 190.0f;
				drawGlassPanel(*ctx, rX, boxY, rW, boxH, 12.0f,
				               nisaba::gpu::Color::rgba(12, 16, 26, 240),
				               nisaba::gpu::Color::rgba(8, 11, 18, 250),
				               nisaba::gpu::Color::rgba(0, 255, 180, 80));

				ctx->fontSize(12.0f);
				ctx->fontFace("bold");
				ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
				ctx->fillColor(nisaba::gpu::Color::rgba(0, 255, 180, 255));
				ctx->text(rX + 16.0f, boxY + 14.0f, "🔬 NISABA DECODER TELEMETRY");

				auto drawDiag = [&](float dy, const char* label, const std::string& val, nisaba::gpu::Color col) {
					ctx->fontSize(11.0f);
					ctx->fontFace("sans");
					ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Middle);
					ctx->fillColor(nisaba::gpu::Color::rgba(140, 160, 185, 200));
					ctx->text(rX + 16.0f, dy, label);

					ctx->fontFace("mono");
					ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Middle);
					ctx->fillColor(col);
					ctx->text(rX + rW - 16.0f, dy, val.c_str());
				};

				char dimStr[64], timeStr[64], texStr[64], memStr[64];
				std::snprintf(dimStr, sizeof(dimStr), "%d x %d px", pProd->imgW, pProd->imgH);
				std::snprintf(timeStr, sizeof(timeStr), "%.2f ms (Nisaba C++20)", pProd->decodeTimeMs);
				std::snprintf(texStr, sizeof(texStr), "Texture #%d (GL_RGBA8)", pProd->textureId);
				std::snprintf(memStr, sizeof(memStr), "%.2f MB VRAM", (pProd->imgW * pProd->imgH * 4) / (1024.0 * 1024.0));

				drawDiag(boxY + 44.0f, "Image Dimensions", dimStr, nisaba::gpu::Color::rgba(255, 255, 255, 240));
				drawDiag(boxY + 70.0f, "Decode Latency", timeStr, nisaba::gpu::Color::rgba(0, 255, 180, 255));
				drawDiag(boxY + 96.0f, "Format & Quality", pProd->formatBadge, pProd->badgeColor);
				drawDiag(boxY + 122.0f, "Hardware Texture", texStr, nisaba::gpu::Color::rgba(0, 220, 255, 240));
				drawDiag(boxY + 148.0f, "Memory Footprint", memStr, nisaba::gpu::Color::rgba(255, 200, 50, 240));

				// Modal Add to Cart Button
				static float mAddHover = 0.0f;
				if (drawButton(*ctx, rX, modalY + modalH - 74.0f, rW, 42.0f, "Add to Cart Now",
				               nisaba::gpu::Color::rgba(0, 180, 230, 255), mAddHover, "bold", 14.0f)) {
					g_app.addToCart(pProd->id);
					g_app.quickViewProductId = -1;
				}
			}
		}

		// ---------------------------------------------------------------------
		// 6. Order Confirmed Celebration Modal
		// ---------------------------------------------------------------------
		if (g_app.orderConfirmed) {
			ctx->beginPath();
			ctx->rect(0.0f, 0.0f, static_cast<float>(winW), static_cast<float>(winH));
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 0, 0, 200));
			ctx->fill();

			float cW = 540.0f;
			float cH = 320.0f;
			float cX = (static_cast<float>(winW) - cW) * 0.5f;
			float cY = (static_cast<float>(winH) - cH) * 0.5f;

			drawGlassPanel(*ctx, cX, cY, cW, cH, 20.0f,
			               nisaba::gpu::Color::rgba(18, 26, 38, 252),
			               nisaba::gpu::Color::rgba(10, 15, 24, 255),
			               nisaba::gpu::Color::rgba(0, 255, 160, 180));

			ctx->fontSize(42.0f);
			ctx->fontFace("bold");
			ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Top);
			ctx->fillColor(nisaba::gpu::Color::rgba(0, 255, 160, 255));
			ctx->text(cX + cW * 0.5f, cY + 40.0f, "✓");

			ctx->fontSize(24.0f);
			ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
			ctx->text(cX + cW * 0.5f, cY + 100.0f, "Order Confirmed!");

			ctx->fontSize(14.0f);
			ctx->fontFace("sans");
			ctx->fillColor(nisaba::gpu::Color::rgba(170, 190, 215, 220));
			ctx->text(cX + cW * 0.5f, cY + 145.0f, "Your order has been processed with sub-millisecond precision.");
			ctx->text(cX + cW * 0.5f, cY + 170.0f, "Powered by Nisaba C++20 Sovereign Graphics Architecture.");

			static float okHover = 0.0f;
			if (drawButton(*ctx, cX + (cW - 180.0f) * 0.5f, cY + 225.0f, 180.0f, 42.0f, "Continue Shopping",
			               nisaba::gpu::Color::rgba(0, 200, 120, 255), okHover, "bold", 14.0f)) {
				g_app.orderConfirmed = false;
				g_app.cart.clear();
			}
		}

		// ---------------------------------------------------------------------
		// 7. Toast Notification (Bottom Center)
		// ---------------------------------------------------------------------
		if (g_app.toastTimer > 0.0f) {
			float toastAlpha = std::min(g_app.toastTimer * 2.0f, 1.0f);
			float tW = 320.0f;
			float tH = 44.0f;
			float tX = (static_cast<float>(winW) - tW) * 0.5f;
			float tY = static_cast<float>(winH) - 70.0f;

			auto tBg = ctx->linearGradient(tX, tY, tX, tY + tH,
			                              nisaba::gpu::Color::rgba(0, 180, 240, static_cast<uint8_t>(230 * toastAlpha)),
			                              nisaba::gpu::Color::rgba(0, 120, 200, static_cast<uint8_t>(240 * toastAlpha)));
			ctx->beginPath();
			ctx->roundedRect(tX, tY, tW, tH, 22.0f);
			ctx->fillPaint(tBg);
			ctx->fill();

			ctx->fontSize(13.0f);
			ctx->fontFace("bold");
			ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, static_cast<uint8_t>(255 * toastAlpha)));
			ctx->text(tX + tW * 0.5f, tY + tH * 0.5f, g_app.toastText.c_str());
		}

		ctx->endFrame();

		// Update metrics
		double frameTimeMs = dt * 1000.0;
		double currentFps = (dt > 0.0001f) ? (1.0 / dt) : 60.0;
		smoothedFps += (currentFps - smoothedFps) * std::min(dt * 5.0f, 1.0f);
		smoothedCpuMs += (frameTimeMs - smoothedCpuMs) * std::min(dt * 5.0f, 1.0f);

		g_mouseClicked = false;

		window->swapBuffers();

		if (targetFrames > 0 && ++frameCount >= targetFrames) {
			break;
		}
	}

	double totalSec = platform->getTime() - benchStartTime;
	if (frameCount > 0 && totalSec > 0.0) {
		std::printf("====================================================\n");
		std::printf("NovaStore Benchmark Completed (%d frames):\n", frameCount);
		std::printf("Average FPS        : %.1f FPS\n", frameCount / totalSec);
		std::printf("Average Frame Time : %.2f ms\n", (totalSec / frameCount) * 1000.0);
		std::printf("====================================================\n");
	}

	window.reset();
	platform.reset();
	return 0;
}
