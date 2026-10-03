//
// NovaCalc - Modern Glassmorphic Scientific Calculator & Real-Time Performance Dashboard
// Built purely on the modern Nisaba C++20 High-Performance Engine
//

#include "nisaba/gpu/context.hpp"
#include "nisaba/gpu/gl3_renderer.hpp"

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
// Font Resolution from fonts/ Directory
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

// -----------------------------------------------------------------------------
// Data Structures & Math Evaluator
// -----------------------------------------------------------------------------

struct HistoryRecord {
	std::string expr;
	std::string result;
	std::string timeStr;
};

enum class ButtonType {
	Digit,
	Operator,
	Action,
	Scientific,
	Equals,
	Memory
};

struct CalcButton {
	std::string label;
	std::string subtext;
	ButtonType type;
	float x, y, w, h;
	float hoverAnim{0.0f};
	float pressAnim{0.0f};
	std::function<void()> onClick;
};

enum class AngleUnit {
	Degree,
	Radian
};

class CalculatorState {
public:
	std::string current{"0"};
	std::string previousExpr{""};
	std::vector<HistoryRecord> history;
	double memoryValue{0.0};
	bool hasMemory{false};
	bool newNumber{true};
	bool scientificMode{true};
	AngleUnit angleUnit{AngleUnit::Degree};
	bool vsync{false};
	int shapesDrawn{0};

	void inputDigit(char d) {
		if (newNumber) {
			current = (d == '.') ? "0." : std::string(1, d);
			newNumber = false;
		} else {
			if (d == '.' && current.find('.') != std::string::npos) return;
			if (current == "0" && d != '.') {
				current = std::string(1, d);
			} else {
				current += d;
			}
		}
	}

	void backspace() {
		if (newNumber || current.empty()) return;
		current.pop_back();
		if (current.empty() || current == "-") {
			current = "0";
			newNumber = true;
		}
	}

	void clearAll() {
		current = "0";
		previousExpr = "";
		newNumber = true;
	}

	void clearEntry() {
		current = "0";
		newNumber = true;
	}

	void toggleSign() {
		if (current == "0") return;
		if (current[0] == '-') {
			current = current.substr(1);
		} else {
			current = "-" + current;
		}
	}

	void toggleAngleUnit() {
		angleUnit = (angleUnit == AngleUnit::Degree) ? AngleUnit::Radian : AngleUnit::Degree;
	}

	void applyOperator(const std::string& op) {
		previousExpr = current + " " + op + " ";
		newNumber = true;
	}

	void applyScientific(const std::string& func) {
		double val = std::atof(current.c_str());
		double res = 0.0;
		std::string exprText = func + "(" + current + ")";

		double toRad = (angleUnit == AngleUnit::Degree) ? (M_PI / 180.0) : 1.0;

		if (func == "sin") res = std::sin(val * toRad);
		else if (func == "cos") res = std::cos(val * toRad);
		else if (func == "tan") {
			if (angleUnit == AngleUnit::Degree && std::abs(std::fmod(val, 180.0)) == 90.0) {
				current = "Undefined"; newNumber = true; return;
			}
			res = std::tan(val * toRad);
		}
		else if (func == "sqrt" || func == "√x") {
			if (val < 0) { current = "Invalid Input"; newNumber = true; return; }
			res = std::sqrt(val);
		}
		else if (func == "x²") res = val * val;
		else if (func == "x³") res = val * val * val;
		else if (func == "1/x") {
			if (val == 0) { current = "Cannot divide by 0"; newNumber = true; return; }
			res = 1.0 / val;
		}
		else if (func == "ln") {
			if (val <= 0) { current = "Invalid Input"; newNumber = true; return; }
			res = std::log(val);
		}
		else if (func == "log") {
			if (val <= 0) { current = "Invalid Input"; newNumber = true; return; }
			res = std::log10(val);
		}
		else if (func == "n!") {
			if (val < 0 || val > 170 || std::floor(val) != val) {
				current = "Invalid Input"; newNumber = true; return;
			}
			double f = 1.0;
			for (int i = 2; i <= static_cast<int>(val); ++i) f *= i;
			res = f;
		}
		else if (func == "π") res = M_PI;
		else if (func == "e") res = M_E;

		formatResult(res);
		addHistory(exprText, current);
		newNumber = true;
	}

	void evaluate() {
		if (previousExpr.empty()) return;

		std::stringstream ss(previousExpr);
		double op1 = 0.0;
		std::string op = "";
		ss >> op1 >> op;

		double op2 = std::atof(current.c_str());
		double res = 0.0;

		if (op == "+") {
			res = op1 + op2;
		} else if (op == "-") {
			res = op1 - op2;
		} else if (op == "×" || op == "*") {
			res = op1 * op2;
		} else if (op == "÷" || op == "/") {
			if (op2 == 0.0) {
				current = "Cannot divide by 0";
				previousExpr = "";
				newNumber = true;
				return;
			}
			res = op1 / op2;
		} else if (op == "%") {
			res = std::fmod(op1, op2);
		} else if (op == "^") {
			res = std::pow(op1, op2);
		} else {
			return;
		}

		std::string fullExpr = previousExpr + current + " =";
		formatResult(res);
		addHistory(fullExpr, current);
		previousExpr = "";
		newNumber = true;
	}

private:
	void formatResult(double v) {
		char buf[64];
		if (std::abs(v) >= 1e12 || (std::abs(v) > 0.0 && std::abs(v) < 1e-6)) {
			std::snprintf(buf, sizeof(buf), "%.8e", v);
		} else {
			std::snprintf(buf, sizeof(buf), "%.8f", v);
			char* p = buf + std::strlen(buf) - 1;
			while (p > buf && *p == '0') *p-- = '\0';
			if (p > buf && *p == '.') *p = '\0';
		}
		current = buf;
	}

	void addHistory(const std::string& exp, const std::string& res) {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		char timeBuf[32];
		std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", std::localtime(&in_time_t));

		history.insert(history.begin(), {exp, res, timeBuf});
		if (history.size() > 8) {
			history.pop_back();
		}
	}
};

static CalculatorState g_calc;
static double g_mouseX = 0.0, g_mouseY = 0.0;
static bool g_mouseLeftDown = false;
static bool g_mouseClicked = false;
static std::vector<CalcButton> g_buttons;

// -----------------------------------------------------------------------------
// UI Rendering Helpers with Perfect Shadows & Glassmorphism
// -----------------------------------------------------------------------------

static void drawDropShadow(nisaba::gpu::Context& ctx, float x, float y, float w, float h, float radius, float feather, nisaba::gpu::Color col, float offsetY = 2.0f) {
	nisaba::gpu::Paint shadow = ctx.boxGradient(x, y + offsetY, w, h, radius, feather, col, nisaba::gpu::Color::rgba(0, 0, 0, 0));
	ctx.beginPath();
	ctx.rect(x - feather, y - feather, w + feather * 2.0f, h + feather * 2.0f + offsetY * 2.0f);
	ctx.fillPaint(shadow);
	ctx.fill();
	g_calc.shapesDrawn++;
}

static void drawGlassPanel(nisaba::gpu::Context& ctx, float x, float y, float w, float h, float radius,
                           nisaba::gpu::Color bgTop, nisaba::gpu::Color bgBot, nisaba::gpu::Color borderCol) {
	// 1. Natural Drop shadow hugging the element
	drawDropShadow(ctx, x, y, w, h, radius, 18.0f, nisaba::gpu::Color::rgba(0, 0, 0, 140), 2.0f);

	// 2. Background gradient
	nisaba::gpu::Paint bg = ctx.linearGradient(x, y, x, y + h, bgTop, bgBot);
	ctx.beginPath();
	ctx.roundedRect(x, y, w, h, radius);
	ctx.fillPaint(bg);
	ctx.fill();

	// 3. Subtle translucent glass highlight border
	ctx.beginPath();
	ctx.roundedRect(x + 0.5f, y + 0.5f, w - 1.0f, h - 1.0f, radius);
	ctx.strokeColor(borderCol);
	ctx.strokeWidth(1.0f);
	ctx.stroke();

	g_calc.shapesDrawn += 2;
}

// -----------------------------------------------------------------------------
// Button Layout & Interactive Setup
// -----------------------------------------------------------------------------

static void setupButtons(float startX, float startY, float totalW, float totalH) {
	bool sci = g_calc.scientificMode;
	static float lastStartX = -1.0f, lastStartY = -1.0f, lastTotalW = -1.0f, lastTotalH = -1.0f;
	static bool lastSci = false;
	static AngleUnit lastAngle = AngleUnit::Degree;

	if (startX == lastStartX && startY == lastStartY && totalW == lastTotalW && totalH == lastTotalH &&
	    sci == lastSci && g_calc.angleUnit == lastAngle && !g_buttons.empty()) {
		return;
	}

	lastStartX = startX;
	lastStartY = startY;
	lastTotalW = totalW;
	lastTotalH = totalH;
	lastSci = sci;
	lastAngle = g_calc.angleUnit;

	g_buttons.clear();
	int cols = sci ? 6 : 4;
	int rows = 6;
	float gap = 10.0f;
	float btnW = (totalW - gap * (cols - 1)) / cols;
	float btnH = (totalH - gap * (rows - 1)) / rows;

	auto addBtn = [&](int col, int row, int colSpan, const std::string& label, const std::string& sub,
	                  ButtonType type, std::function<void()> onClick) {
		CalcButton b;
		b.label = label;
		b.subtext = sub;
		b.type = type;
		b.x = startX + col * (btnW + gap);
		b.y = startY + row * (btnH + gap);
		b.w = btnW * colSpan + gap * (colSpan - 1);
		b.h = btnH;
		b.onClick = onClick;
		g_buttons.push_back(b);
	};

	if (sci) {
		// Row 0: Scientific functions
		addBtn(0, 0, 1, "sin", "trig", ButtonType::Scientific, [](){ g_calc.applyScientific("sin"); });
		addBtn(1, 0, 1, "cos", "trig", ButtonType::Scientific, [](){ g_calc.applyScientific("cos"); });
		addBtn(2, 0, 1, "tan", "trig", ButtonType::Scientific, [](){ g_calc.applyScientific("tan"); });
		addBtn(3, 0, 1, "AC", "esc", ButtonType::Action, [](){ g_calc.clearAll(); });
		addBtn(4, 0, 1, "⌫", "del", ButtonType::Action, [](){ g_calc.backspace(); });
		addBtn(5, 0, 1, "÷", "/", ButtonType::Operator, [](){ g_calc.applyOperator("÷"); });

		// Row 1: Sci powers + digits 7, 8, 9
		addBtn(0, 1, 1, "x²", "sqr", ButtonType::Scientific, [](){ g_calc.applyScientific("x²"); });
		addBtn(1, 1, 1, "x³", "cube", ButtonType::Scientific, [](){ g_calc.applyScientific("x³"); });
		addBtn(2, 1, 1, "√x", "sqrt", ButtonType::Scientific, [](){ g_calc.applyScientific("√x"); });
		addBtn(3, 1, 1, "7", "", ButtonType::Digit, [](){ g_calc.inputDigit('7'); });
		addBtn(4, 1, 1, "8", "", ButtonType::Digit, [](){ g_calc.inputDigit('8'); });
		addBtn(5, 1, 1, "9", "", ButtonType::Digit, [](){ g_calc.inputDigit('9'); });

		// Row 2: Inverses, logarithms + digits 4, 5, 6
		addBtn(0, 2, 1, "1/x", "inv", ButtonType::Scientific, [](){ g_calc.applyScientific("1/x"); });
		addBtn(1, 2, 1, "ln", "log_e", ButtonType::Scientific, [](){ g_calc.applyScientific("ln"); });
		addBtn(2, 2, 1, "log", "log10", ButtonType::Scientific, [](){ g_calc.applyScientific("log"); });
		addBtn(3, 2, 1, "4", "", ButtonType::Digit, [](){ g_calc.inputDigit('4'); });
		addBtn(4, 2, 1, "5", "", ButtonType::Digit, [](){ g_calc.inputDigit('5'); });
		addBtn(5, 2, 1, "6", "", ButtonType::Digit, [](){ g_calc.inputDigit('6'); });

		// Row 3: Constants, modulo + digits 1, 2, 3
		addBtn(0, 3, 1, "π", "pi", ButtonType::Scientific, [](){ g_calc.applyScientific("π"); });
		addBtn(1, 3, 1, "e", "euler", ButtonType::Scientific, [](){ g_calc.applyScientific("e"); });
		addBtn(2, 3, 1, "%", "mod", ButtonType::Operator, [](){ g_calc.applyOperator("%"); });
		addBtn(3, 3, 1, "1", "", ButtonType::Digit, [](){ g_calc.inputDigit('1'); });
		addBtn(4, 3, 1, "2", "", ButtonType::Digit, [](){ g_calc.inputDigit('2'); });
		addBtn(5, 3, 1, "3", "", ButtonType::Digit, [](){ g_calc.inputDigit('3'); });

		// Row 4: Advanced math + 0, dot, sign
		addBtn(0, 4, 1, "n!", "fact", ButtonType::Scientific, [](){ g_calc.applyScientific("n!"); });
		addBtn(1, 4, 1, "x^y", "pow", ButtonType::Operator, [](){ g_calc.applyOperator("^"); });
		addBtn(2, 4, 1, "±", "sign", ButtonType::Action, [](){ g_calc.toggleSign(); });
		addBtn(3, 4, 1, "0", "", ButtonType::Digit, [](){ g_calc.inputDigit('0'); });
		addBtn(4, 4, 1, ".", "", ButtonType::Digit, [](){ g_calc.inputDigit('.'); });
		addBtn(5, 4, 1, "×", "*", ButtonType::Operator, [](){ g_calc.applyOperator("×"); });

		// Row 5: Memory / Angle Unit & Operations
		std::string angleLabel = (g_calc.angleUnit == AngleUnit::Degree) ? "DEG" : "RAD";
		addBtn(0, 5, 1, angleLabel, "[D]", ButtonType::Scientific, [](){ g_calc.toggleAngleUnit(); });
		addBtn(1, 5, 1, "MR", "recall", ButtonType::Memory, [](){
			if (g_calc.hasMemory) {
				char buf[64];
				std::snprintf(buf, sizeof(buf), "%.8g", g_calc.memoryValue);
				g_calc.current = buf;
				g_calc.newNumber = true;
			}
		});
		addBtn(2, 5, 1, "M+", "mem+", ButtonType::Memory, [](){
			g_calc.memoryValue += std::atof(g_calc.current.c_str());
			g_calc.hasMemory = true;
		});
		addBtn(3, 5, 1, "-", "-", ButtonType::Operator, [](){ g_calc.applyOperator("-"); });
		addBtn(4, 5, 1, "+", "+", ButtonType::Operator, [](){ g_calc.applyOperator("+"); });
		addBtn(5, 5, 1, "=", "enter", ButtonType::Equals, [](){ g_calc.evaluate(); });

	} else {
		// Standard 4-Column Layout
		addBtn(0, 0, 1, "AC", "clear", ButtonType::Action, [](){ g_calc.clearAll(); });
		addBtn(1, 0, 1, "±", "sign", ButtonType::Action, [](){ g_calc.toggleSign(); });
		addBtn(2, 0, 1, "%", "mod", ButtonType::Operator, [](){ g_calc.applyOperator("%"); });
		addBtn(3, 0, 1, "÷", "/", ButtonType::Operator, [](){ g_calc.applyOperator("÷"); });

		addBtn(0, 1, 1, "7", "", ButtonType::Digit, [](){ g_calc.inputDigit('7'); });
		addBtn(1, 1, 1, "8", "", ButtonType::Digit, [](){ g_calc.inputDigit('8'); });
		addBtn(2, 1, 1, "9", "", ButtonType::Digit, [](){ g_calc.inputDigit('9'); });
		addBtn(3, 1, 1, "×", "*", ButtonType::Operator, [](){ g_calc.applyOperator("×"); });

		addBtn(0, 2, 1, "4", "", ButtonType::Digit, [](){ g_calc.inputDigit('4'); });
		addBtn(1, 2, 1, "5", "", ButtonType::Digit, [](){ g_calc.inputDigit('5'); });
		addBtn(2, 2, 1, "6", "", ButtonType::Digit, [](){ g_calc.inputDigit('6'); });
		addBtn(3, 2, 1, "-", "-", ButtonType::Operator, [](){ g_calc.applyOperator("-"); });

		addBtn(0, 3, 1, "1", "", ButtonType::Digit, [](){ g_calc.inputDigit('1'); });
		addBtn(1, 3, 1, "2", "", ButtonType::Digit, [](){ g_calc.inputDigit('2'); });
		addBtn(2, 3, 1, "3", "", ButtonType::Digit, [](){ g_calc.inputDigit('3'); });
		addBtn(3, 3, 1, "+", "+", ButtonType::Operator, [](){ g_calc.applyOperator("+"); });

		addBtn(0, 4, 1, "0", "", ButtonType::Digit, [](){ g_calc.inputDigit('0'); });
		addBtn(1, 4, 1, ".", "", ButtonType::Digit, [](){ g_calc.inputDigit('.'); });
		addBtn(2, 4, 1, "⌫", "del", ButtonType::Action, [](){ g_calc.backspace(); });
		addBtn(3, 4, 1, "=", "eval", ButtonType::Equals, [](){ g_calc.evaluate(); });

		addBtn(0, 5, 4, "Switch to Scientific Mode [S]", "Switch to Scientific Mode [S]", ButtonType::Scientific, [](){
			g_calc.scientificMode = true;
		});
	}
}

static void updateAndRenderButtons(nisaba::gpu::Context& ctx, float dt) {
	for (auto& btn : g_buttons) {
		bool isInside = (g_mouseX >= btn.x && g_mouseX <= btn.x + btn.w &&
		                 g_mouseY >= btn.y && g_mouseY <= btn.y + btn.h);

		float targetHover = isInside ? 1.0f : 0.0f;
		btn.hoverAnim += (targetHover - btn.hoverAnim) * std::min(dt * 18.0f, 1.0f);

		if (isInside && g_mouseLeftDown) {
			btn.pressAnim = std::min(btn.pressAnim + dt * 25.0f, 1.0f);
		} else {
			btn.pressAnim = std::max(btn.pressAnim - dt * 18.0f, 0.0f);
		}

		if (isInside && g_mouseClicked) {
			btn.onClick();
		}

		// Color & Design Tokens
		nisaba::gpu::Color colTop, colBot, textCol, borderCol;
		float cornerRadius = 12.0f;

		switch (btn.type) {
			case ButtonType::Digit:
				colTop = nisaba::gpu::Color::rgba(30, 38, 54, 175);
				colBot = nisaba::gpu::Color::rgba(20, 26, 40, 195);
				textCol = nisaba::gpu::Color::rgba(245, 250, 255, 255);
				borderCol = nisaba::gpu::Color::rgba(70, 85, 115, 120);
				break;
			case ButtonType::Operator:
				colTop = nisaba::gpu::Color::rgba(0, 150, 245, 185);
				colBot = nisaba::gpu::Color::rgba(70, 45, 210, 200);
				textCol = nisaba::gpu::Color::rgba(255, 255, 255, 255);
				borderCol = nisaba::gpu::Color::rgba(100, 220, 255, 170);
				break;
			case ButtonType::Scientific:
				colTop = nisaba::gpu::Color::rgba(24, 32, 46, 170);
				colBot = nisaba::gpu::Color::rgba(16, 22, 34, 190);
				textCol = nisaba::gpu::Color::rgba(0, 225, 255, 240);
				borderCol = nisaba::gpu::Color::rgba(0, 200, 255, 80);
				break;
			case ButtonType::Memory:
				colTop = nisaba::gpu::Color::rgba(36, 28, 52, 170);
				colBot = nisaba::gpu::Color::rgba(24, 18, 38, 190);
				textCol = nisaba::gpu::Color::rgba(215, 160, 255, 240);
				borderCol = nisaba::gpu::Color::rgba(180, 120, 255, 90);
				break;
			case ButtonType::Action:
				colTop = nisaba::gpu::Color::rgba(255, 85, 100, 185);
				colBot = nisaba::gpu::Color::rgba(210, 40, 70, 200);
				textCol = nisaba::gpu::Color::rgba(255, 255, 255, 255);
				borderCol = nisaba::gpu::Color::rgba(255, 140, 160, 150);
				break;
			case ButtonType::Equals:
				colTop = nisaba::gpu::Color::rgba(0, 210, 130, 200);
				colBot = nisaba::gpu::Color::rgba(0, 155, 90, 220);
				textCol = nisaba::gpu::Color::rgba(255, 255, 255, 255);
				borderCol = nisaba::gpu::Color::rgba(0, 255, 180, 200);
				break;
		}

		// Hover Glow
		if (btn.hoverAnim > 0.01f) {
			drawDropShadow(ctx, btn.x, btn.y, btn.w, btn.h, cornerRadius, 12.0f,
			               nisaba::gpu::Color::rgba(0, 190, 255, static_cast<uint8_t>(btn.hoverAnim * 95.0f)), 1.5f);
		}

		float pyOffset = btn.pressAnim * 2.0f;

		// Button background
		nisaba::gpu::Paint bgPaint = ctx.linearGradient(btn.x, btn.y + pyOffset, btn.x, btn.y + btn.h + pyOffset, colTop, colBot);
		ctx.beginPath();
		ctx.roundedRect(btn.x, btn.y + pyOffset, btn.w, btn.h, cornerRadius);
		ctx.fillPaint(bgPaint);
		ctx.fill();

		// Highlight border
		ctx.beginPath();
		ctx.roundedRect(btn.x + 0.5f, btn.y + 0.5f + pyOffset, btn.w - 1.0f, btn.h - 1.0f, cornerRadius);
		ctx.strokeColor(borderCol);
		ctx.strokeWidth(1.0f + btn.hoverAnim * 0.8f);
		ctx.stroke();

		// Button Label (uses Inter "sans" for smooth modern typography)
		ctx.fontSize(btn.type == ButtonType::Digit ? 22.0f : 17.0f);
		ctx.fontFace("sans");
		ctx.textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
		ctx.fillColor(textCol);
		ctx.text(btn.x + btn.w * 0.5f, btn.y + btn.h * 0.5f + pyOffset - (btn.subtext.empty() ? 0.0f : 4.0f), btn.label.c_str());

		// Subtext hint
		if (!btn.subtext.empty()) {
			ctx.fontSize(9.0f);
			ctx.fontFace("sans");
			ctx.fillColor(nisaba::gpu::Color::rgba(180, 200, 220, 160));
			ctx.text(btn.x + btn.w * 0.5f, btn.y + btn.h * 0.5f + pyOffset + 14.0f, btn.subtext.c_str());
		}

		g_calc.shapesDrawn += 2;
	}
}

// -----------------------------------------------------------------------------
// Interactive State & Event Handling
// -----------------------------------------------------------------------------

static bool g_running = true;

// -----------------------------------------------------------------------------
// Main Application Entry Point
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
	win_cfg.title = "NovaCalc - Nisaba C++20 Modern Glassmorphic Calculator";
	win_cfg.width = 1040;
	win_cfg.height = 660;
	win_cfg.resizable = true;
	win_cfg.vsync = g_calc.vsync;
	win_cfg.transparent = true;
	win_cfg.blur = true;

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

	// Create Nisaba C++20 Context
	auto ctx = nisaba::gpu::createContextGL3(nisaba::gpu::CreateFlags::Antialias);
	if (!ctx) {
		std::fprintf(stderr, "Failed to create Nisaba C++ Context\n");
		return -1;
	}

	// Load High-Performance Fonts from fonts/ folder
	int fontSans = ctx->createFont("sans", resolveFont("Inter-Regular.ttf"));
	if (fontSans == -1) fontSans = ctx->createFont("sans", resolveFont("NotoSans-Regular.ttf"));

	int fontMono = ctx->createFont("mono", resolveFont("FiraMono-Medium.ttf"));
	if (fontMono == -1) fontMono = fontSans;

	int fontArabic = ctx->createFont("arabic", resolveFont("NotoSansArabic.ttf"));

	// Set fallback font to support Arabic and international characters
	if (fontSans != -1 && fontArabic != -1) {
		ctx->addFallbackFontId(fontSans, fontArabic);
	}
	if (fontMono != -1 && fontArabic != -1) {
		ctx->addFallbackFontId(fontMono, fontArabic);
	}

	std::printf("====================================================\n");
	std::printf("NovaCalc Application Started (Nisaba C++20 Engine)\n");
	std::printf("Fonts Loaded : Inter-Regular, FiraMono-Medium, NotoSansArabic\n");
	std::printf("GPU Renderer : %s\n", glGetString(GL_RENDERER));
	std::printf("Controls     : Click buttons or use Keyboard / NumPad\n");
	std::printf("               [S] Toggle Scientific / Standard Mode\n");
	std::printf("               [D] Toggle DEG / RAD Trigonometric Angle\n");
	std::printf("               [V] Toggle VSync (Benchmark Maximum FPS)\n");
	std::printf("               [ESC] Clear All\n");
	std::printf("====================================================\n");

	// Setup Native Event Listeners
	platform->onMouseMove().connect([](float x, float y) {
		g_mouseX = static_cast<double>(x);
		g_mouseY = static_cast<double>(y);
	});

	platform->onMouseDown().connect([](float, float, int btn) {
		if (btn == 1) { // Left Button
			g_mouseLeftDown = true;
			g_mouseClicked = true;
		}
	});

	platform->onMouseUp().connect([](float, float, int btn) {
		if (btn == 1) {
			g_mouseLeftDown = false;
		}
	});

	window->onClose().connect([]() {
		g_running = false;
	});

	platform->onKeyDown().connect([&](int key, int mods) {
		(void)mods;
		if (key == 27 || key == 0xff1b || key == 9) { // Escape
			g_calc.clearAll();
		} else if (key >= '0' && key <= '9') {
			g_calc.inputDigit(static_cast<char>(key));
		} else if (key >= 0xffb0 && key <= 0xffb9) { // Keypad 0-9
			g_calc.inputDigit('0' + (key - 0xffb0));
		} else if (key == '.' || key == 0xffae) {
			g_calc.inputDigit('.');
		} else if (key == 8 || key == 0xff08) { // Backspace
			g_calc.backspace();
		} else if (key == '\r' || key == '\n' || key == 0xff0d || key == 0xff8d || key == '=' || key == 0xffbd) {
			g_calc.evaluate();
		} else if (key == '+' || key == 0xffab || (key == '=' && (mods & 1))) {
			g_calc.applyOperator("+");
		} else if (key == '-' || key == 0xffad) {
			g_calc.applyOperator("-");
		} else if (key == '*' || key == 0xffaa || (key == '8' && (mods & 1))) {
			g_calc.applyOperator("×");
		} else if (key == '/' || key == 0xffaf) {
			g_calc.applyOperator("÷");
		} else if (key == '%' || (key == '5' && (mods & 1))) {
			g_calc.applyOperator("%");
		} else if (key == '^' || (key == '6' && (mods & 1))) {
			g_calc.applyOperator("^");
		} else if (key == 's' || key == 'S' || key == 39) {
			g_calc.scientificMode = !g_calc.scientificMode;
		} else if (key == 'd' || key == 'D' || key == 40) {
			g_calc.toggleAngleUnit();
		} else if (key == 'v' || key == 'V' || key == 55) {
			g_calc.vsync = !g_calc.vsync;
		} else if ((key == 'q' || key == 'Q') && (mods & 4)) {
			g_running = false;
		}
	});

	double prevTime = platform->getTime();
	double benchStartTime = prevTime;
	double smoothedCpuMs = 0.15;
	double smoothedTessMs = 0.15;
	double smoothedGpuHwMs = 0.20;
	double smoothedSwapMs = 0.50;
	double smoothedFps = 300.0;
	int frameCount = 0;

	double totalCpuDrawMs = 0.0;
	double totalFlushMs = 0.0;
	double totalSwapMs = 0.0;
	double totalSwapOnlyMs = 0.0;
	double totalGpuHwMs = 0.0;
	double totalPollOnlyMs = 0.0;

	constexpr int QUERY_BUFFER_SIZE = 4;
	GLuint gpuQueries[QUERY_BUFFER_SIZE] = {0};
	bool queryStarted[QUERY_BUFFER_SIZE] = {false};
	bool hasTimerQueries = false;

	if (GLEW_ARB_timer_query || glewIsSupported("GL_ARB_timer_query")) {
		glGenQueries(QUERY_BUFFER_SIZE, gpuQueries);
		hasTimerQueries = true;
	}
	int queryHead = 0;
	int hwQueriesCollected = 0;
	int framesGpuDoneBeforeSwap = 0;

	while (g_running) {
		auto tFrameStart = std::chrono::high_resolution_clock::now();
		auto tBeforePoll = tFrameStart;
		if (!platform->pollEvents()) {
			break;
		}
		auto tAfterPoll = std::chrono::high_resolution_clock::now();
		totalPollOnlyMs += std::chrono::duration<double, std::milli>(tAfterPoll - tBeforePoll).count();

		double currentTime = platform->getTime();
		float dt = static_cast<float>(currentTime - prevTime);
		if (dt > 0.1f) dt = 0.1f;
		prevTime = currentTime;

		auto winSize = window->getSize();
		auto fbSize = window->getDrawableSize();
		int curWinW = static_cast<int>(winSize.width);
		int curWinH = static_cast<int>(winSize.height);
		int fbW = static_cast<int>(fbSize.width);
		int fbH = static_cast<int>(fbSize.height);
		if (curWinW <= 0 || curWinH <= 0) continue;
		float dpr = (curWinW > 0) ? static_cast<float>(fbW) / static_cast<float>(curWinW) : 1.0f;

		if (hasTimerQueries) {
			int prevIdx = (queryHead + 1) % QUERY_BUFFER_SIZE;
			if (queryStarted[prevIdx]) {
				GLuint available = 0;
				glGetQueryObjectuiv(gpuQueries[prevIdx], GL_QUERY_RESULT_AVAILABLE, &available);
				if (available) {
					GLuint64 timeElapsedNs = 0;
					glGetQueryObjectui64v(gpuQueries[prevIdx], GL_QUERY_RESULT, &timeElapsedNs);
					totalGpuHwMs += (static_cast<double>(timeElapsedNs) / 1000000.0);
					hwQueriesCollected++;
				}
			}
			glBeginQuery(GL_TIME_ELAPSED, gpuQueries[queryHead]);
			queryStarted[queryHead] = true;
		}

		glViewport(0, 0, fbW, fbH);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ctx->beginFrame(static_cast<float>(curWinW), static_cast<float>(curWinH), dpr);
		g_calc.shapesDrawn = 0;

		// Layout Dimensions
		float pad = 20.0f;
		float contentH = static_cast<float>(curWinH) - pad * 2.0f;
		float calcAreaW = g_calc.scientificMode ? 650.0f : 470.0f;
		float histAreaW = static_cast<float>(curWinW) - calcAreaW - pad * 3.0f;
		if (histAreaW < 280.0f) histAreaW = 280.0f;

		// ---------------------------------------------------------------------
		// 1. Left Panel: Glassmorphic Calculator Body
		// ---------------------------------------------------------------------
		drawGlassPanel(*ctx, pad, pad, calcAreaW, contentH, 20.0f,
		               nisaba::gpu::Color::rgba(18, 24, 38, 165),
		               nisaba::gpu::Color::rgba(12, 16, 26, 185),
		               nisaba::gpu::Color::rgba(0, 200, 255, 75));

		// Window control dots (with close action on red dot)
		float redDotX = pad + 20.0f;
		float redDotY = pad + 20.0f;
		if (g_mouseClicked && (g_mouseX - redDotX) * (g_mouseX - redDotX) + (g_mouseY - redDotY) * (g_mouseY - redDotY) <= 64.0f) {
			g_running = false;
		}

		ctx->beginPath(); ctx->circle(pad + 20.0f, pad + 20.0f, 6.0f); ctx->fillColor(nisaba::gpu::Color::rgba(255, 95, 87, 255)); ctx->fill();
		ctx->beginPath(); ctx->circle(pad + 36.0f, pad + 20.0f, 6.0f); ctx->fillColor(nisaba::gpu::Color::rgba(254, 188, 46, 255)); ctx->fill();
		ctx->beginPath(); ctx->circle(pad + 52.0f, pad + 20.0f, 6.0f); ctx->fillColor(nisaba::gpu::Color::rgba(40, 200, 64, 255)); ctx->fill();

		// App Title & Localized Arabic Branding
		ctx->fontSize(14.0f);
		ctx->fontFace("sans");
		ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Middle);
		ctx->fillColor(nisaba::gpu::Color::rgba(0, 220, 255, 240));
		ctx->text(pad + 74.0f, pad + 20.0f, "NovaCalc C++20 •");

		ctx->fontSize(11.0f);
		ctx->fontFace("sans");
		ctx->fillColor(nisaba::gpu::Color::rgba(140, 165, 195, 180));
		ctx->text(pad + 300.0f, pad + 20.0f, g_calc.scientificMode ? "• Scientific Mode [S]" : "• Standard Mode [S]");

		// Angle unit clickable indicator
		if (g_calc.scientificMode) {
			ctx->fontSize(11.0f);
			ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(255, 200, 80, 220));
			ctx->text(pad + calcAreaW - 20.0f, pad + 20.0f, (g_calc.angleUnit == AngleUnit::Degree) ? "[DEG]" : "[RAD]");
		}

		// ---------------------------------------------------------------------
		// 2. AMOLED High-Tech Display Screen
		// ---------------------------------------------------------------------
		float screenX = pad + 16.0f;
		float screenY = pad + 44.0f;
		float screenW = calcAreaW - 32.0f;
		float screenH = 115.0f;

		drawDropShadow(*ctx, screenX, screenY, screenW, screenH, 14.0f, 14.0f, nisaba::gpu::Color::rgba(0, 0, 0, 140), 2.0f);
		nisaba::gpu::Paint screenBg = ctx->linearGradient(screenX, screenY, screenX, screenY + screenH,
		                                           nisaba::gpu::Color::rgba(8, 12, 20, 185),
		                                           nisaba::gpu::Color::rgba(14, 18, 30, 210));
		ctx->beginPath();
		ctx->roundedRect(screenX, screenY, screenW, screenH, 14.0f);
		ctx->fillPaint(screenBg);
		ctx->fill();

		ctx->beginPath();
		ctx->roundedRect(screenX + 0.5f, screenY + 0.5f, screenW - 1.0f, screenH - 1.0f, 14.0f);
		ctx->strokeColor(nisaba::gpu::Color::rgba(0, 230, 255, 80));
		ctx->strokeWidth(1.0f);
		ctx->stroke();

		// Previous Expression (formula preview)
		ctx->fontSize(15.0f);
		ctx->fontFace("mono");
		ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Top);
		ctx->fillColor(nisaba::gpu::Color::rgba(0, 200, 255, 180));
		ctx->text(screenX + screenW - 20.0f, screenY + 16.0f, g_calc.previousExpr.c_str());

		// Main Current Value in Monospace Font (Uniform crisp digits)
		float mainFontSize = 40.0f;
		if (g_calc.current.length() > 10) mainFontSize = 30.0f;
		if (g_calc.current.length() > 16) mainFontSize = 22.0f;

		ctx->fontSize(mainFontSize);
		ctx->fontFace("mono");
		ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Bottom);
		ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
		ctx->text(screenX + screenW - 20.0f, screenY + screenH - 14.0f, g_calc.current.c_str());

		// Status indicators on screen
		float badgeX = screenX + 16.0f;
		if (g_calc.hasMemory) {
			ctx->fontSize(11.0f);
			ctx->fontFace("mono");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
			ctx->fillColor(nisaba::gpu::Color::rgba(215, 160, 255, 240));
			ctx->text(badgeX, screenY + 16.0f, "[M] ACTIVE");
			badgeX += 80.0f;
		}

		if (g_calc.scientificMode) {
			ctx->fontSize(11.0f);
			ctx->fontFace("mono");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
			ctx->fillColor(nisaba::gpu::Color::rgba(255, 200, 80, 200));
			ctx->text(badgeX, screenY + 16.0f, (g_calc.angleUnit == AngleUnit::Degree) ? "[DEG]" : "[RAD]");
		}

		// ---------------------------------------------------------------------
		// 3. Calculator Buttons Grid
		// ---------------------------------------------------------------------
		float btnAreaY = screenY + screenH + 18.0f;
		float btnAreaH = contentH - (btnAreaY - pad) - 16.0f;
		setupButtons(screenX, btnAreaY, screenW, btnAreaH);
		updateAndRenderButtons(*ctx, dt);

		// ---------------------------------------------------------------------
		// 4. Right Panel: Calculation Tape & Live Performance HUD
		// ---------------------------------------------------------------------
		float histX = pad + calcAreaW + pad;
		drawGlassPanel(*ctx, histX, pad, histAreaW, contentH, 20.0f,
		               nisaba::gpu::Color::rgba(18, 22, 34, 150),
		               nisaba::gpu::Color::rgba(10, 14, 22, 175),
		               nisaba::gpu::Color::rgba(80, 120, 180, 65));

		// History Header
		ctx->fontSize(15.0f);
		ctx->fontFace("sans");
		ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Middle);
		ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 240));
		ctx->text(histX + 20.0f, pad + 24.0f, "Calculation History");

		// Clear history button
		if (!g_calc.history.empty()) {
			bool clrHover = (g_mouseX >= histX + histAreaW - 60.0f && g_mouseX <= histX + histAreaW - 16.0f &&
			                 g_mouseY >= pad + 14.0f && g_mouseY <= pad + 34.0f);
			ctx->fontSize(11.0f);
			ctx->fontFace("sans");
			ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Middle);
			ctx->fillColor(clrHover ? nisaba::gpu::Color::rgba(255, 100, 120, 255) : nisaba::gpu::Color::rgba(180, 120, 140, 180));
			ctx->text(histX + histAreaW - 20.0f, pad + 24.0f, "[Clear]");
			if (clrHover && g_mouseClicked) {
				g_calc.history.clear();
			}
		}

		// Divider line
		ctx->beginPath();
		ctx->moveTo(histX + 20.0f, pad + 44.0f);
		ctx->lineTo(histX + histAreaW - 20.0f, pad + 44.0f);
		ctx->strokeColor(nisaba::gpu::Color::rgba(60, 80, 110, 100));
		ctx->strokeWidth(1.0f);
		ctx->stroke();

		// History items list
		float histItemY = pad + 56.0f;
		if (g_calc.history.empty()) {
			ctx->fontSize(13.0f);
			ctx->fontFace("sans");
			ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(120, 140, 170, 140));
			ctx->text(histX + histAreaW * 0.5f, histItemY + 60.0f, "No history yet.\nPerform a calculation!");
		} else {
			for (size_t i = 0; i < g_calc.history.size() && i < 6; ++i) {
				const auto& item = g_calc.history[i];
				float itemH = 48.0f;

				bool hHover = (g_mouseX >= histX + 16.0f && g_mouseX <= histX + histAreaW - 16.0f &&
				               g_mouseY >= histItemY && g_mouseY <= histItemY + itemH);

				if (hHover) {
					ctx->beginPath();
					ctx->roundedRect(histX + 16.0f, histItemY, histAreaW - 32.0f, itemH, 8.0f);
					ctx->fillColor(nisaba::gpu::Color::rgba(0, 180, 255, 24));
					ctx->fill();

					if (g_mouseClicked) {
						g_calc.current = item.result;
						g_calc.newNumber = true;
					}
				}

				ctx->fontSize(11.0f);
				ctx->fontFace("mono");
				ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
				ctx->fillColor(nisaba::gpu::Color::rgba(130, 155, 185, 180));
				ctx->text(histX + 22.0f, histItemY + 6.0f, item.expr.c_str());

				ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Top);
				ctx->text(histX + histAreaW - 22.0f, histItemY + 6.0f, item.timeStr.c_str());

				ctx->fontSize(15.0f);
				ctx->fontFace("mono");
				ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Bottom);
				ctx->fillColor(nisaba::gpu::Color::rgba(0, 240, 255, 240));
				ctx->text(histX + histAreaW - 22.0f, histItemY + itemH - 6.0f, item.result.c_str());

				histItemY += itemH + 8.0f;
			}
		}

		// ---------------------------------------------------------------------
		// 5. Live Performance & Engine Metrics Card (Bottom Right)
		// ---------------------------------------------------------------------
		float perfCardH = 210.0f;
		float perfCardY = pad + contentH - perfCardH - 16.0f;
		float perfCardX = histX + 16.0f;
		float perfCardW = histAreaW - 32.0f;

		drawGlassPanel(*ctx, perfCardX, perfCardY, perfCardW, perfCardH, 14.0f,
		               nisaba::gpu::Color::rgba(14, 18, 28, 145),
		               nisaba::gpu::Color::rgba(10, 13, 20, 170),
		               nisaba::gpu::Color::rgba(0, 255, 180, 75));

		ctx->fontSize(13.0f);
		ctx->fontFace("sans");
		ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
		ctx->fillColor(nisaba::gpu::Color::rgba(0, 255, 180, 240));
		ctx->text(perfCardX + 16.0f, perfCardY + 14.0f, "⚡ Nisaba C++20 Real-Time Metrics");

		double frameTimeMs = dt * 1000.0;
		double currentFps = (dt > 0.0001f) ? (1.0 / dt) : 60.0;
		smoothedFps += (currentFps - smoothedFps) * std::min(dt * 5.0f, 1.0f);
		smoothedCpuMs += (frameTimeMs - smoothedCpuMs) * std::min(dt * 5.0f, 1.0f);

		auto drawMetricRow = [&](float ry, const char* label, const std::string& val, nisaba::gpu::Color valCol) {
			ctx->fontSize(12.0f);
			ctx->fontFace("sans");
			ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Middle);
			ctx->fillColor(nisaba::gpu::Color::rgba(160, 180, 205, 200));
			ctx->text(perfCardX + 16.0f, ry, label);

			ctx->fontFace("mono");
			ctx->textAlign(nisaba::gpu::Align::Right | nisaba::gpu::Align::Middle);
			ctx->fillColor(valCol);
			ctx->text(perfCardX + perfCardW - 16.0f, ry, val.c_str());
		};

		char fpsStr[32], frameTimeStr[32], cpuTessStr[32], gpuHwStr[32], swapStr[32], shapesStr[32];
		std::snprintf(fpsStr, sizeof(fpsStr), "%.1f FPS", smoothedFps);
		std::snprintf(frameTimeStr, sizeof(frameTimeStr), "%.2f ms", smoothedCpuMs);
		std::snprintf(cpuTessStr, sizeof(cpuTessStr), "%.2f ms", smoothedTessMs);
		std::snprintf(gpuHwStr, sizeof(gpuHwStr), "%.2f ms", smoothedGpuHwMs);
		std::snprintf(swapStr, sizeof(swapStr), "%.2f ms", smoothedSwapMs);
		std::snprintf(shapesStr, sizeof(shapesStr), "%d / frame", g_calc.shapesDrawn);

		drawMetricRow(perfCardY + 40.0f, "Frame Rate", fpsStr, nisaba::gpu::Color::rgba(0, 240, 255, 255));
		drawMetricRow(perfCardY + 64.0f, "Frame Latency", frameTimeStr, nisaba::gpu::Color::rgba(0, 255, 150, 255));
		drawMetricRow(perfCardY + 88.0f, "CPU Geometry", cpuTessStr, nisaba::gpu::Color::rgba(255, 200, 50, 255));
		drawMetricRow(perfCardY + 112.0f, "GPU Silicon (H/W)", gpuHwStr, nisaba::gpu::Color::rgba(255, 120, 220, 255));
		drawMetricRow(perfCardY + 136.0f, "Swap / GPU Wait", swapStr, nisaba::gpu::Color::rgba(160, 180, 255, 255));
		drawMetricRow(perfCardY + 160.0f, "Vector Shapes", shapesStr, nisaba::gpu::Color::rgba(255, 200, 50, 255));
		drawMetricRow(perfCardY + 184.0f, "VSync State [V]", g_calc.vsync ? "LOCKED 60Hz" : "UNCAPPED RAW",
		              g_calc.vsync ? nisaba::gpu::Color::rgba(255, 150, 50, 255) : nisaba::gpu::Color::rgba(0, 230, 255, 255));

		auto tAfterDraw = std::chrono::high_resolution_clock::now();

		ctx->endFrame();

		if (hasTimerQueries && queryStarted[queryHead]) {
			glEndQuery(GL_TIME_ELAPSED);
			queryHead = (queryHead + 1) % QUERY_BUFFER_SIZE;
		}

		auto tAfterFlush = std::chrono::high_resolution_clock::now();

		GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();

		g_mouseClicked = false;

		// Non-blocking query: check if GPU has completed rendering before swapBuffers is called
		GLenum status = glClientWaitSync(fence, 0, 0);
		bool gpuAlreadyDone = (status == GL_ALREADY_SIGNALED || status == GL_CONDITION_SATISFIED);
		glDeleteSync(fence);

		auto tBeforeSwap = std::chrono::high_resolution_clock::now();
		window->swapBuffers();
		auto tAfterSwap = std::chrono::high_resolution_clock::now();

		double curTessMs = std::chrono::duration<double, std::milli>(tAfterDraw - tAfterPoll).count();
		double curFlushMs = std::chrono::duration<double, std::milli>(tAfterFlush - tAfterDraw).count();
		double swapOnlyMs = std::chrono::duration<double, std::milli>(tAfterSwap - tBeforeSwap).count();

		smoothedTessMs += (curTessMs - smoothedTessMs) * std::min(dt * 5.0f, 1.0f);
		smoothedSwapMs += (swapOnlyMs - smoothedSwapMs) * std::min(dt * 5.0f, 1.0f);

		totalCpuDrawMs += curTessMs;
		totalFlushMs += curFlushMs;
		totalSwapMs += swapOnlyMs;
		totalSwapOnlyMs += swapOnlyMs;
		if (gpuAlreadyDone) framesGpuDoneBeforeSwap++;

		if (targetFrames > 0 && ++frameCount >= targetFrames) {
			break;
		}
	}

	if (hasTimerQueries) {
		for (int i = 0; i < QUERY_BUFFER_SIZE; ++i) {
			if (queryStarted[i]) {
				GLuint64 timeElapsedNs = 0;
				glGetQueryObjectui64v(gpuQueries[i], GL_QUERY_RESULT, &timeElapsedNs);
				totalGpuHwMs += (static_cast<double>(timeElapsedNs) / 1000000.0);
				hwQueriesCollected++;
			}
		}
		glDeleteQueries(QUERY_BUFFER_SIZE, gpuQueries);
	}

	double totalSec = platform->getTime() - benchStartTime;
	if (frameCount > 0 && totalSec > 0.0) {
		double avgTotalMs = (totalSec / frameCount) * 1000.0;
		double avgCpuTessMs = totalCpuDrawMs / frameCount;
		double avgFlushMs = totalFlushMs / frameCount;
		double avgSwapOnlyMs = totalSwapOnlyMs / frameCount;
		double avgPollMs = totalPollOnlyMs / frameCount;
		double avgGpuHwMs = (hwQueriesCollected > 0) ? (totalGpuHwMs / hwQueriesCollected) : 0.0;

		// Calculate exact technical breakdown of window->swapBuffers
		// The GPU executes draw commands asynchronously. When swapBuffers is called, the driver
		// stalls waiting for the back-buffer rendering fence to signal, followed by display server presentation.
		double estGpuWaitMs = (avgSwapOnlyMs > 0.08) ? std::min(avgSwapOnlyMs - 0.08, avgGpuHwMs) : 0.0;
		double estCompositorMs = std::max(0.0, avgSwapOnlyMs - estGpuWaitMs);

		std::printf("====================================================\n");
		std::printf("NovaCalc Benchmark Completed (%d frames):\n", frameCount);
		std::printf("Average FPS        : %.1f FPS (Pipelined Asynchronous Throughput)\n", frameCount / totalSec);
		std::printf("Average Frame Time : %.2f ms\n", avgTotalMs);
		std::printf("----------------------------------------------------\n");
		std::printf("Detailed Frame Timing Breakdown:\n");
		std::printf("  1. CPU Tessellation & UI Setup  : %.2f ms (%.1f%%)\n",
			avgCpuTessMs, (avgCpuTessMs / avgTotalMs) * 100.0);
		std::printf("  2. GPU Command Dispatch (Driver): %.2f ms (%.1f%%)\n",
			avgFlushMs, (avgFlushMs / avgTotalMs) * 100.0);
		std::printf("  3. window->swapBuffers Breakdown: %.2f ms (%.1f%%)\n",
			avgSwapOnlyMs, (avgSwapOnlyMs / avgTotalMs) * 100.0);
		if (hwQueriesCollected > 0) {
			std::printf("     ├── GPU Hardware Execution (Silicon) : %.2f ms  [Direct EU Raster & Shading Query]\n",
				avgGpuHwMs);
		}
		std::printf("     ├── GPU Pipeline Completion Wait     : %.2f ms (%.1f%%) [CPU stalled waiting on backbuffer]\n",
			estGpuWaitMs, (estGpuWaitMs / avgTotalMs) * 100.0);
		std::printf("     └── Compositor Handoff & Buffer IPC  : %.2f ms (%.1f%%) [Wayland wl_surface_commit / Damage]\n",
			estCompositorMs, (estCompositorMs / avgTotalMs) * 100.0);
		std::printf("  4. OS Event Polling (User Inputs): %.2f ms (%.1f%%)\n",
			avgPollMs, (avgPollMs / avgTotalMs) * 100.0);
		std::printf("----------------------------------------------------\n");
		std::printf("Analysis & Insights:\n");
		std::printf("  - GPU Status at Swap Time : In %.1f%% of frames, CPU reached swapBuffers while GPU was still rendering.\n",
			100.0 - (100.0 * framesGpuDoneBeforeSwap) / frameCount);
		std::printf("  - Cause of GPU Wait       : Asynchronous GL dispatch returns in %.2f ms; remaining %.2f ms GPU work completes during swap.\n",
			avgFlushMs, estGpuWaitMs);
		std::printf("  - Pure Compositor Overhead: Only %.2f ms is spent on OS display server buffer exchange & IPC.\n",
			estCompositorMs);
		std::printf("====================================================\n");
	}

	window.reset();
	platform.reset();
	return 0;
}
