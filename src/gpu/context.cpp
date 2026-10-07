#include "nisaba/gpu/context.hpp"
#include "tessellator.hpp"
#include "glyph_atlas.hpp"
#include "nisaba/text/bidi.hpp"
#include "nisaba/image/image_io.hpp"

#include <cmath>
#include <cstring>
#include <stdexcept>
#include <algorithm>

namespace nisaba::gpu {

static inline float distPtSeg(float x, float y, float px, float py, float qx, float qy) {
	float pqx = qx - px;
	float pqy = qy - py;
	float dx = x - px;
	float dy = y - py;
	float d = pqx * pqx + pqy * pqy;
	float t = pqx * dx + pqy * dy;
	if (d > 0.0f) t /= d;
	if (t < 0.0f) t = 0.0f;
	else if (t > 1.0f) t = 1.0f;
	dx = px + t * pqx - x;
	dy = py + t * pqy - y;
	return dx * dx + dy * dy;
}

static inline float cross2D(float dx0, float dy0, float dx1, float dy1) {
	return dx1 * dy0 - dx0 * dy1;
}

static inline bool isTransformFlipped(const Transform2D& xform) {
	float det = xform.m[0] * xform.m[3] - xform.m[2] * xform.m[1];
	return det < 0.0f;
}

static inline bool ptEquals(float x1, float y1, float x2, float y2, float tol) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	return dx * dx + dy * dy < tol * tol;
}

static inline float normalizeVec(float* x, float* y) {
	float d = std::sqrt((*x) * (*x) + (*y) * (*y));
	if (d > 1e-6f) {
		float id = 1.0f / d;
		*x *= id;
		*y *= id;
	}
	return d;
}

static inline float quantize(float a, float d) {
	return (static_cast<int>(a / d + 0.5f)) * d;
}

// -------------------------------------------------------------
// Context Lifecycle
// -------------------------------------------------------------
Context::Context(std::unique_ptr<Renderer> renderer, int flags)
	: m_renderer(std::move(renderer)), m_flags(flags) {
	if (!m_renderer) {
		throw std::runtime_error("Renderer cannot be null");
	}

	m_tessellator = new Tessellator();
	m_commands.reserve(256);

	// Setup Initial State
	m_states.reserve(32);
	m_states.emplace_back();
	reset();

	if (!m_renderer->init()) {
		throw std::runtime_error("Failed to initialize GPU renderer");
	}

	// Initialize Sovereign Glyph Atlas & Font Texture
	m_atlas = std::make_unique<SovereignGlyphAtlas>(1024, 1024);
	m_fontTextureId = m_renderer->createTexture(TextureType::Alpha, m_atlas->width(), m_atlas->height(), 0, m_atlas->data());
}

Context::~Context() {
	if (m_fontTextureId != 0 && m_renderer) {
		m_renderer->deleteTexture(m_fontTextureId);
		m_fontTextureId = 0;
	}
	delete m_tessellator;
	if (m_renderer) {
		m_renderer->shutdown();
	}
}

// -------------------------------------------------------------
// Frame Lifecycle
// -------------------------------------------------------------
void Context::beginFrame(float windowWidth, float windowHeight, float devicePixelRatio) {
	m_states.clear();
	m_states.emplace_back();
	reset();

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	m_devicePixelRatio = devicePixelRatio;
	m_tessTol = 0.25f / devicePixelRatio;
	m_distTol = 0.01f / devicePixelRatio;
	m_fringeWidth = 1.0f / devicePixelRatio;

	m_tessellator->reset(m_distTol, m_tessTol, m_fringeWidth);
	m_renderer->viewport(windowWidth, windowHeight, devicePixelRatio);
}

void Context::cancelFrame() {
	m_renderer->cancel();
}

void Context::endFrame() {
	m_renderer->flush();
}

// -------------------------------------------------------------
// State Handling
// -------------------------------------------------------------
void Context::save() {
	if (m_states.size() >= 32) return;
	m_states.push_back(m_states.back());
}

void Context::restore() {
	if (m_states.size() <= 1) return;
	m_states.pop_back();
}

void Context::reset() {
	State& s = currentState();
	s = State();
	setPaintColor(s.fill, Color::rgba(255, 255, 255, 255));
	setPaintColor(s.stroke, Color::rgba(0, 0, 0, 255));
	s.compositeOperation = CompositeOperationState::fromCompositeOp(CompositeOperation::SourceOver);
	s.shapeAntiAlias = true;
	s.strokeWidth = 1.0f;
	s.miterLimit = 10.0f;
	s.lineCap = LineCap::Butt;
	s.lineJoin = LineJoin::Miter;
	s.alpha = 1.0f;
	s.xform = Transform2D::identity();
	s.scissor.extent[0] = -1.0f;
	s.scissor.extent[1] = -1.0f;
	s.fontSize = 16.0f;
	s.letterSpacing = 0.0f;
	s.lineHeight = 1.0f;
	s.fontBlur = 0.0f;
	s.textAlign = Align::Left | Align::Baseline;
	int defFont = findFont("sans");
	if (defFont <= 0 && !m_fontNames.empty()) {
		defFont = m_fontNames.begin()->second;
	}
	s.fontId = (defFont > 0) ? defFont : 0;
}

void Context::setPaintColor(Paint& p, Color color) {
	p.xform = Transform2D::identity();
	p.radius = 0.0f;
	p.feather = 1.0f;
	p.extent[0] = 0.0f;
	p.extent[1] = 0.0f;
	p.innerColor = color;
	p.outerColor = color;
	p.image = 0;
}

void Context::shapeAntiAlias(bool enabled) {
	currentState().shapeAntiAlias = enabled;
}

void Context::strokeColor(Color color) {
	setPaintColor(currentState().stroke, color);
}

void Context::strokePaint(const Paint& paint) {
	State& s = currentState();
	s.stroke = paint;
	s.stroke.xform = s.stroke.xform.multiply(s.xform);
}

void Context::fillColor(Color color) {
	setPaintColor(currentState().fill, color);
}

void Context::fillPaint(const Paint& paint) {
	State& s = currentState();
	s.fill = paint;
	s.fill.xform = s.fill.xform.multiply(s.xform);
}

void Context::miterLimit(float limit) {
	currentState().miterLimit = limit;
}

void Context::strokeWidth(float size) {
	currentState().strokeWidth = size;
}

void Context::lineCap(LineCap cap) {
	currentState().lineCap = cap;
}

void Context::lineCap(nisaba::LineCap cap) {
	switch (cap) {
		case nisaba::LineCap::Butt: currentState().lineCap = LineCap::Butt; break;
		case nisaba::LineCap::Round: currentState().lineCap = LineCap::Round; break;
		case nisaba::LineCap::Square: currentState().lineCap = LineCap::Square; break;
	}
}

void Context::lineJoin(LineJoin join) {
	currentState().lineJoin = join;
}

void Context::lineJoin(nisaba::LineJoin join) {
	switch (join) {
		case nisaba::LineJoin::Miter: currentState().lineJoin = LineJoin::Miter; break;
		case nisaba::LineJoin::Round: currentState().lineJoin = LineJoin::Round; break;
		case nisaba::LineJoin::Bevel: currentState().lineJoin = LineJoin::Bevel; break;
		default: currentState().lineJoin = LineJoin::Miter; break;
	}
}

void Context::globalAlpha(float alpha) {
	currentState().alpha = alpha;
}

// -------------------------------------------------------------
// Transforms
// -------------------------------------------------------------
void Context::resetTransform() {
	currentState().xform = Transform2D::identity();
}

void Context::transform(float a, float b, float c, float d, float e, float f) {
	State& s = currentState();
	Transform2D t(a, b, c, d, e, f);
	s.xform = s.xform.premultiply(t);
}

void Context::transform(const Transform2D& t) {
	State& s = currentState();
	s.xform = s.xform.premultiply(t);
}

void Context::transform(const nisaba::Transform& ts) {
	transform(Transform2D(ts));
}

void Context::translate(float x, float y) {
	State& s = currentState();
	s.xform = s.xform.premultiply(Transform2D::translate(x, y));
}

void Context::rotate(float angleRad) {
	State& s = currentState();
	s.xform = s.xform.premultiply(Transform2D::rotate(angleRad));
}

void Context::skewX(float angleRad) {
	State& s = currentState();
	s.xform = s.xform.premultiply(Transform2D::skewX(angleRad));
}

void Context::skewY(float angleRad) {
	State& s = currentState();
	s.xform = s.xform.premultiply(Transform2D::skewY(angleRad));
}

void Context::scale(float x, float y) {
	State& s = currentState();
	s.xform = s.xform.premultiply(Transform2D::scale(x, y));
}

Transform2D Context::currentTransform() const {
	return currentState().xform;
}

// -------------------------------------------------------------
// Scissoring
// -------------------------------------------------------------
void Context::scissor(float x, float y, float w, float h) {
	State& s = currentState();
	w = std::max(0.0f, w);
	h = std::max(0.0f, h);

	s.scissor.xform = Transform2D::identity();
	s.scissor.xform.m[4] = x + w * 0.5f;
	s.scissor.xform.m[5] = y + h * 0.5f;
	s.scissor.xform = s.scissor.xform.multiply(s.xform);

	s.scissor.extent[0] = w * 0.5f;
	s.scissor.extent[1] = h * 0.5f;
}

void Context::scissorScreen(float x, float y, float w, float h) {
	State& s = currentState();
	w = std::max(0.0f, w);
	h = std::max(0.0f, h);

	s.scissor.xform = Transform2D::identity();
	s.scissor.xform.m[4] = x + w * 0.5f;
	s.scissor.xform.m[5] = y + h * 0.5f;

	s.scissor.extent[0] = w * 0.5f;
	s.scissor.extent[1] = h * 0.5f;
}

static void isectRects(float* dst,
                       float ax, float ay, float aw, float ah,
                       float bx, float by, float bw, float bh) {
	float minx = std::max(ax, bx);
	float miny = std::max(ay, by);
	float maxx = std::min(ax + aw, bx + bw);
	float maxy = std::min(ay + ah, by + bh);
	dst[0] = minx;
	dst[1] = miny;
	dst[2] = std::max(0.0f, maxx - minx);
	dst[3] = std::max(0.0f, maxy - miny);
}

void Context::intersectScissor(float x, float y, float w, float h) {
	State& s = currentState();
	if (s.scissor.extent[0] < 0.0f) {
		scissor(x, y, w, h);
		return;
	}

	Transform2D invXform;
	s.xform.inverse(invXform);

	Transform2D pxform = s.scissor.xform.multiply(invXform);
	float ex = s.scissor.extent[0];
	float ey = s.scissor.extent[1];

	float tex = ex * std::abs(pxform.m[0]) + ey * std::abs(pxform.m[2]);
	float tey = ex * std::abs(pxform.m[1]) + ey * std::abs(pxform.m[3]);

	float rect[4];
	isectRects(rect, pxform.m[4] - tex, pxform.m[5] - tey, tex * 2.0f, tey * 2.0f, x, y, w, h);
	scissor(rect[0], rect[1], rect[2], rect[3]);
}

void Context::resetScissor() {
	currentState().scissor.extent[0] = -1.0f;
	currentState().scissor.extent[1] = -1.0f;
}

// -------------------------------------------------------------
// Global Compositing
// -------------------------------------------------------------
void Context::globalCompositeOperation(CompositeOperation op) {
	currentState().compositeOperation = CompositeOperationState::fromCompositeOp(op);
}

void Context::globalCompositeBlendFunc(BlendFactor sfactor, BlendFactor dfactor) {
	globalCompositeBlendFuncSeparate(sfactor, dfactor, sfactor, dfactor);
}

void Context::globalCompositeBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha) {
	State& s = currentState();
	s.compositeOperation.srcRGB = srcRGB;
	s.compositeOperation.dstRGB = dstRGB;
	s.compositeOperation.srcAlpha = srcAlpha;
	s.compositeOperation.dstAlpha = dstAlpha;
}

// -------------------------------------------------------------
// Images
// -------------------------------------------------------------
int Context::createImage(const char* filename, int imageFlags) {
	if (!filename) return 0;
	auto res = nisaba::image::load_image_file(filename);
	if (!res) return 0;
	return createImageRGBA(static_cast<int>(res->width()),
	                       static_cast<int>(res->height()),
	                       imageFlags,
	                       res->data());
}

int Context::createImageMem(int imageFlags, unsigned char* data, int ndata) {
	if (!data || ndata <= 0) return 0;
	auto res = nisaba::image::load_image_from_memory(std::span<const uint8_t>(data, static_cast<size_t>(ndata)));
	if (!res) return 0;
	return createImageRGBA(static_cast<int>(res->width()),
	                       static_cast<int>(res->height()),
	                       imageFlags,
	                       res->data());
}

int Context::createImageRGBA(int w, int h, int imageFlags, const unsigned char* data) {
	return m_renderer->createTexture(TextureType::RGBA, w, h, imageFlags, data);
}

void Context::updateImage(int image, const unsigned char* data) {
	int w, h;
	m_renderer->getTextureSize(image, w, h);
	m_renderer->updateTexture(image, 0, 0, w, h, data);
}

void Context::imageSize(int image, int& w, int& h) {
	m_renderer->getTextureSize(image, w, h);
}

void Context::deleteImage(int image) {
	m_renderer->deleteTexture(image);
}

// -------------------------------------------------------------
// Paints
// -------------------------------------------------------------
Paint Context::linearGradient(float sx, float sy, float ex, float ey, Color icol, Color ocol) {
	return Paint::linearGradient(sx, sy, ex, ey, icol, ocol);
}

Paint Context::boxGradient(float x, float y, float w, float h, float r, float f, Color icol, Color ocol) {
	return Paint::boxGradient(x, y, w, h, r, f, icol, ocol);
}

Paint Context::radialGradient(float cx, float cy, float inr, float outr, Color icol, Color ocol) {
	return Paint::radialGradient(cx, cy, inr, outr, icol, ocol);
}

Paint Context::imagePattern(float cx, float cy, float w, float h, float angleRad, int image, float alpha) {
	return Paint::imagePattern(cx, cy, w, h, angleRad, image, alpha);
}

// -------------------------------------------------------------
// Path Commands
// -------------------------------------------------------------
void Context::materializePending() {
	if (m_singleCircle) {
		m_singleCircle = false;
		ellipsePath(m_circleCx, m_circleCy, m_circleR, m_circleR);
	}
	if (m_singleEllipse) {
		m_singleEllipse = false;
		ellipsePath(m_ellipseCx, m_ellipseCy, m_ellipseRx, m_ellipseRy);
	}
	if (m_singleRect) {
		m_singleRect = false;
		rectPath(m_rectX, m_rectY, m_rectW, m_rectH);
	}
	if (m_singleRRect) {
		m_singleRRect = false;
		roundedRectVaryingPath(m_rrectX, m_rrectY, m_rrectW, m_rrectH, m_rrectR, m_rrectR, m_rrectR, m_rrectR);
	}
	if (m_singleVaryingRRect) {
		m_singleVaryingRRect = false;
		roundedRectVaryingPath(m_rrectX, m_rrectY, m_rrectW, m_rrectH, m_radTL, m_radTR, m_radBR, m_radBL);
	}
}

void Context::appendCommand(const PathCommand& cmd) {
	materializePending();
	State& s = currentState();
	PathCommand transformed = cmd;

	// Always record the current pen position in LOCAL coordinate space!
	switch (cmd.type) {
		case PathCommandType::MoveTo:
		case PathCommandType::LineTo:
			m_commandX = cmd.p0.x;
			m_commandY = cmd.p0.y;
			break;
		case PathCommandType::BezierTo:
			m_commandX = cmd.p2.x;
			m_commandY = cmd.p2.y;
			break;
		default:
			break;
	}

	if (s.xform.isIdentity()) {
		m_singleCircle = false;
		m_singleEllipse = false;
		m_singleRect = false;
		m_singleRRect = false;
		m_singleVaryingRRect = false;
		m_commands.push_back(transformed);
		return;
	}

	switch (cmd.type) {
		case PathCommandType::MoveTo:
		case PathCommandType::LineTo: {
			Point pt = s.xform.transformPoint(cmd.p0.x, cmd.p0.y);
			transformed.p0 = pt;
			break;
		}
		case PathCommandType::BezierTo: {
			Point p1 = s.xform.transformPoint(cmd.p0.x, cmd.p0.y);
			Point p2 = s.xform.transformPoint(cmd.p1.x, cmd.p1.y);
			Point p3 = s.xform.transformPoint(cmd.p2.x, cmd.p2.y);
			transformed.p0 = p1;
			transformed.p1 = p2;
			transformed.p2 = p3;
			break;
		}
		case PathCommandType::Close:
		case PathCommandType::Winding:
			break;
	}

	m_singleCircle = false;
	m_singleEllipse = false;
	m_singleRect = false;
	m_singleRRect = false;
	m_singleVaryingRRect = false;
	m_commands.push_back(transformed);
}

void Context::beginPath() {
	m_commands.clear();
	m_tessellator->clear();
	m_singleCircle = false;
	m_singleEllipse = false;
	m_singleRect = false;
	m_singleRRect = false;
	m_singleVaryingRRect = false;
}

void Context::path(const nisaba::Path& p) {
	PathSegmentsIter iter(p);
	while (auto seg = iter.next()) {
		switch (seg->type) {
			case PathSegment::Type::MoveTo:
				moveTo(seg->p0.x, seg->p0.y);
				break;
			case PathSegment::Type::LineTo:
				lineTo(seg->p0.x, seg->p0.y);
				break;
			case PathSegment::Type::QuadTo:
				quadTo(seg->p0.x, seg->p0.y, seg->p1.x, seg->p1.y);
				break;
			case PathSegment::Type::CubicTo:
				bezierTo(seg->p0.x, seg->p0.y, seg->p1.x, seg->p1.y, seg->p2.x, seg->p2.y);
				break;
			case PathSegment::Type::Close:
				closePath();
				break;
		}
	}
}

void Context::moveTo(float x, float y) {
	appendCommand(PathCommand::makeMoveTo(Point(x, y)));
}

void Context::lineTo(float x, float y) {
	appendCommand(PathCommand::makeLineTo(Point(x, y)));
}

void Context::bezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y) {
	appendCommand(PathCommand::makeBezierTo(Point(c1x, c1y), Point(c2x, c2y), Point(x, y)));
}

void Context::quadTo(float cx, float cy, float x, float y) {
	float x0 = m_commandX;
	float y0 = m_commandY;
	Point cp1(x0 + 2.0f / 3.0f * (cx - x0), y0 + 2.0f / 3.0f * (cy - y0));
	Point cp2(x + 2.0f / 3.0f * (cx - x), y + 2.0f / 3.0f * (cy - y));
	appendCommand(PathCommand::makeBezierTo(cp1, cp2, Point(x, y)));
}

void Context::arcTo(float x1, float y1, float x2, float y2, float radius) {
	if (m_commands.empty()) return;

	float x0 = m_commandX;
	float y0 = m_commandY;

	if (ptEquals(x0, y0, x1, y1, m_distTol) ||
	    ptEquals(x1, y1, x2, y2, m_distTol) ||
	    distPtSeg(x1, y1, x0, y0, x2, y2) < m_distTol * m_distTol ||
	    radius < m_distTol) {
		lineTo(x1, y1);
		return;
	}

	float dx0 = x0 - x1;
	float dy0 = y0 - y1;
	float dx1 = x2 - x1;
	float dy1 = y2 - y1;
	normalizeVec(&dx0, &dy0);
	normalizeVec(&dx1, &dy1);

	float a = std::acos(dx0 * dx1 + dy0 * dy1);
	float d = radius / std::tan(a * 0.5f);

	if (d > 10000.0f) {
		lineTo(x1, y1);
		return;
	}

	float cx, cy, a0, a1;
	Winding dir;

	if (cross2D(dx0, dy0, dx1, dy1) > 0.0f) {
		cx = x1 + dx0 * d + dy0 * radius;
		cy = y1 + dy0 * d + -dx0 * radius;
		a0 = std::atan2(dx0, -dy0);
		a1 = std::atan2(-dx1, dy1);
		dir = Winding::CounterClockwise;
	} else {
		cx = x1 + dx0 * d + -dy0 * radius;
		cy = y1 + dy0 * d + dx0 * radius;
		a0 = std::atan2(-dx0, dy0);
		a1 = std::atan2(dx1, -dy1);
		dir = Winding::Clockwise;
	}

	arc(cx, cy, radius, a0, a1, dir);
}

void Context::closePath() {
	appendCommand(PathCommand::makeClose());
}

void Context::pathWinding(Winding dir) {
	appendCommand(PathCommand::makeWinding(dir));
}

void Context::arc(float cx, float cy, float r, float a0, float a1, Winding dir) {
	float da = a1 - a0;
	if (dir == Winding::CounterClockwise) {
		if (std::abs(da) >= PI * 2.0f) da = PI * 2.0f;
		else while (da < 0.0f) da += PI * 2.0f;
	} else {
		if (std::abs(da) >= PI * 2.0f) da = -PI * 2.0f;
		else while (da > 0.0f) da -= PI * 2.0f;
	}

	int ndivs = std::max(1, std::min(static_cast<int>(std::abs(da) / (PI * 0.5f) + 0.5f), 5));
	float hda = (da / static_cast<float>(ndivs)) * 0.5f;
	float kappa = std::abs(4.0f / 3.0f * (1.0f - std::cos(hda)) / std::sin(hda));
	if (da < 0.0f) kappa = -kappa;

	float px = 0.0f, py = 0.0f, ptanx = 0.0f, ptany = 0.0f;
	int move = m_commands.empty() ? 0 : 1;

	for (int i = 0; i <= ndivs; i++) {
		float a = a0 + da * (i / static_cast<float>(ndivs));
		float dx = std::cos(a);
		float dy = std::sin(a);
		float x = cx + dx * r;
		float y = cy + dy * r;
		float tanx = -dy * r * kappa;
		float tany = dx * r * kappa;

		if (i == 0) {
			if (move == 0) moveTo(x, y);
			else lineTo(x, y);
		} else {
			bezierTo(px + ptanx, py + ptany, x - tanx, y - tany, x, y);
		}
		px = x;
		py = y;
		ptanx = tanx;
		ptany = tany;
	}
}

void Context::rect(float x, float y, float w, float h) {
	if (m_commands.empty()) {
		m_singleRect = true;
		m_rectX = x;
		m_rectY = y;
		m_rectW = w;
		m_rectH = h;
		return;
	}
	materializePending();
	rectPath(x, y, w, h);
}

void Context::rectPath(float x, float y, float w, float h) {
	moveTo(x, y);
	lineTo(x, y + h);
	lineTo(x + w, y + h);
	lineTo(x + w, y);
	closePath();
}

void Context::roundedRect(float x, float y, float w, float h, float r) {
	if (r < 0.1f) {
		rect(x, y, w, h);
		return;
	}
	if (w < 0.0f) { x += w; w = -w; }
	if (h < 0.0f) { y += h; h = -h; }
	if (m_commands.empty()) {
		m_singleRRect = true;
		m_rrectX = x;
		m_rrectY = y;
		m_rrectW = w;
		m_rrectH = h;
		m_rrectR = std::clamp(r, 0.0f, std::min(w, h) * 0.5f);
		return;
	}
	materializePending();
	roundedRectVaryingPath(x, y, w, h, r, r, r, r);
}

void Context::roundedRectVarying(float x, float y, float w, float h,
                                float radTopLeft, float radTopRight,
                                float radBottomRight, float radBottomLeft) {
	if (radTopLeft < 0.1f && radTopRight < 0.1f && radBottomRight < 0.1f && radBottomLeft < 0.1f) {
		rect(x, y, w, h);
		return;
	}
	if (radTopLeft == radTopRight && radTopRight == radBottomRight && radBottomRight == radBottomLeft &&
	    m_commands.empty() && radTopLeft >= 0.1f) {
		roundedRect(x, y, w, h, radTopLeft);
		return;
	}
	if (w < 0.0f) {
		x += w;
		w = -w;
		std::swap(radTopLeft, radTopRight);
		std::swap(radBottomLeft, radBottomRight);
	}
	if (h < 0.0f) {
		y += h;
		h = -h;
		std::swap(radTopLeft, radBottomLeft);
		std::swap(radTopRight, radBottomRight);
	}
	if (m_commands.empty()) {
		m_singleVaryingRRect = true;
		m_rrectX = x;
		m_rrectY = y;
		m_rrectW = w;
		m_rrectH = h;
		float maxR = std::min(w, h) * 0.5f;
		m_radTL = std::clamp(radTopLeft, 0.0f, maxR);
		m_radTR = std::clamp(radTopRight, 0.0f, maxR);
		m_radBR = std::clamp(radBottomRight, 0.0f, maxR);
		m_radBL = std::clamp(radBottomLeft, 0.0f, maxR);
		return;
	}
	materializePending();
	roundedRectVaryingPath(x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}

void Context::roundedRectVaryingPath(float x, float y, float w, float h,
                                    float radTopLeft, float radTopRight,
                                    float radBottomRight, float radBottomLeft) {

	float halfw = std::abs(w) * 0.5f;
	float halfh = std::abs(h) * 0.5f;
	float rxBL = std::min(radBottomLeft, halfw) * (w < 0.0f ? -1.0f : 1.0f);
	float ryBL = std::min(radBottomLeft, halfh) * (h < 0.0f ? -1.0f : 1.0f);
	float rxBR = std::min(radBottomRight, halfw) * (w < 0.0f ? -1.0f : 1.0f);
	float ryBR = std::min(radBottomRight, halfh) * (h < 0.0f ? -1.0f : 1.0f);
	float rxTR = std::min(radTopRight, halfw) * (w < 0.0f ? -1.0f : 1.0f);
	float ryTR = std::min(radTopRight, halfh) * (h < 0.0f ? -1.0f : 1.0f);
	float rxTL = std::min(radTopLeft, halfw) * (w < 0.0f ? -1.0f : 1.0f);
	float ryTL = std::min(radTopLeft, halfh) * (h < 0.0f ? -1.0f : 1.0f);

	auto addCorner = [this](float cx, float cy, float rx, float ry, float c0, float s0) {
		float maxR = std::max(std::abs(rx), std::abs(ry));
		if (maxR < 0.1f) return;
		float deltaAngle = std::acos(maxR / (maxR + m_tessTol)) * 2.0f;
		int ndivs = std::clamp(static_cast<int>(std::ceil((PI * 0.5f) / deltaAngle)), 2, 8);
		float step = (PI * 0.5f) / static_cast<float>(ndivs);
		float Cs = std::cos(step);
		float Ss = std::sin(step);
		float c = c0;
		float s = s0;
		for (int i = 1; i <= ndivs; ++i) {
			float cnext = c * Cs + s * Ss;
			float snext = s * Cs - c * Ss;
			c = cnext;
			s = snext;
			lineTo(cx + c * rx, cy + s * ry);
		}
	};

	moveTo(x, y + ryTL);
	lineTo(x, y + h - ryBL);
	if (std::abs(rxBL) >= 0.1f && std::abs(ryBL) >= 0.1f) {
		addCorner(x + rxBL, y + h - ryBL, rxBL, ryBL, -1.0f, 0.0f);
	} else {
		lineTo(x, y + h);
		lineTo(x + rxBL, y + h);
	}

	lineTo(x + w - rxBR, y + h);
	if (std::abs(rxBR) >= 0.1f && std::abs(ryBR) >= 0.1f) {
		addCorner(x + w - rxBR, y + h - ryBR, rxBR, ryBR, 0.0f, 1.0f);
	} else {
		lineTo(x + w, y + h);
		lineTo(x + w, y + h - ryBR);
	}

	lineTo(x + w, y + ryTR);
	if (std::abs(rxTR) >= 0.1f && std::abs(ryTR) >= 0.1f) {
		addCorner(x + w - rxTR, y + ryTR, rxTR, ryTR, 1.0f, 0.0f);
	} else {
		lineTo(x + w, y);
		lineTo(x + w - rxTR, y);
	}

	lineTo(x + rxTL, y);
	if (std::abs(rxTL) >= 0.1f && std::abs(ryTL) >= 0.1f) {
		addCorner(x + rxTL, y + ryTL, rxTL, ryTL, 0.0f, -1.0f);
	} else {
		lineTo(x, y);
		lineTo(x, y + ryTL);
	}

	closePath();
}

void Context::ellipse(float cx, float cy, float rx, float ry) {
	if (std::abs(rx - ry) < 1e-4f) {
		circle(cx, cy, rx);
		return;
	}
	if (m_commands.empty()) {
		m_singleEllipse = true;
		m_ellipseCx = cx;
		m_ellipseCy = cy;
		m_ellipseRx = std::abs(rx);
		m_ellipseRy = std::abs(ry);
		return;
	}
	materializePending();
	ellipsePath(cx, cy, rx, ry);
}

void Context::ellipsePath(float cx, float cy, float rx, float ry) {
	float maxR = std::max(std::abs(rx), std::abs(ry));
	if (maxR <= 0.0f) return;
	float deltaAngle = std::acos(maxR / (maxR + m_tessTol)) * 2.0f;
	int n = std::clamp(static_cast<int>(std::ceil(2.0f * PI / deltaAngle)), 12, 64);
	float step = (2.0f * PI) / static_cast<float>(n);

	float cs = std::cos(step);
	float ss = std::sin(step);
	float c = 1.0f;
	float s = 0.0f;

	moveTo(cx + rx, cy);
	for (int i = 1; i < n; ++i) {
		float cnext = c * cs - s * ss;
		float snext = s * cs + c * ss;
		c = cnext;
		s = snext;
		lineTo(cx + rx * c, cy + ry * s);
	}
	closePath();
}

void Context::circle(float cx, float cy, float r) {
	if (m_commands.empty()) {
		m_singleCircle = true;
		m_circleCx = cx;
		m_circleCy = cy;
		m_circleR = r;
		return;
	}
	materializePending();
	ellipsePath(cx, cy, r, r);
}

// -------------------------------------------------------------
// Draw Operations (Fill & Stroke)
// -------------------------------------------------------------
void Context::fill() {
	State& s = currentState();
	Paint fillPaint = s.fill;

	bool isConformal = (std::abs(s.xform[1]) < 1e-4f && std::abs(s.xform[2]) < 1e-4f &&
	                    std::abs(s.xform[0] - s.xform[3]) < 1e-4f && s.xform[0] > 0.0f);
	bool isAxisAligned = (std::abs(s.xform[1]) < 1e-4f && std::abs(s.xform[2]) < 1e-4f &&
	                      s.xform[0] > 0.0f && s.xform[3] > 0.0f);
	bool isSolid = (fillPaint.image == 0 && fillPaint.extent[0] == 0.0f && fillPaint.extent[1] == 0.0f &&
	                fillPaint.radius == 0.0f &&
	                fillPaint.innerColor.r == fillPaint.outerColor.r &&
	                fillPaint.innerColor.g == fillPaint.outerColor.g &&
	                fillPaint.innerColor.b == fillPaint.outerColor.b &&
	                fillPaint.innerColor.a == fillPaint.outerColor.a);

	if (m_renderer->backendType() == GpuBackendType::OpenGL && m_singleCircle && isConformal && isSolid) {
		float sc = s.xform[0];
		float cx = m_circleCx * sc + s.xform[4];
		float cy = m_circleCy * sc + s.xform[5];
		float r = m_circleR * sc;

		fillPaint.innerColor.a *= s.alpha;
		fillPaint.outerColor.a *= s.alpha;

		float Rq = r + 1.5f;
		float scale = Rq / r;
		uint32_t c = fillPaint.innerColor.premultiplied().toRGBA8();

		Vertex quad[6];
		// Triangle 1 (p0 -> p3 -> p2: CCW in NDC)
		quad[0] = Vertex(cx - Rq, cy - Rq, -scale, -scale, c);
		quad[1] = Vertex(cx - Rq, cy + Rq, -scale,  scale, c);
		quad[2] = Vertex(cx + Rq, cy + Rq,  scale,  scale, c);
		// Triangle 2 (p0 -> p2 -> p1: CCW in NDC)
		quad[3] = Vertex(cx - Rq, cy - Rq, -scale, -scale, c);
		quad[4] = Vertex(cx + Rq, cy + Rq,  scale,  scale, c);
		quad[5] = Vertex(cx + Rq, cy - Rq,  scale, -scale, c);

		m_renderer->renderTriangles(fillPaint, s.compositeOperation, s.scissor, quad, 6, m_fringeWidth, 7 /* ShaderCircle */);
		return;
	}

	if (m_renderer->backendType() == GpuBackendType::OpenGL && m_singleEllipse && isAxisAligned && isSolid && m_ellipseRx > 0.0f && m_ellipseRy > 0.0f) {
		float sx = s.xform[0];
		float sy = s.xform[3];
		float cx = m_ellipseCx * sx + s.xform[4];
		float cy = m_ellipseCy * sy + s.xform[5];
		float rx = m_ellipseRx * sx;
		float ry = m_ellipseRy * sy;

		fillPaint.innerColor.a *= s.alpha;
		fillPaint.outerColor.a *= s.alpha;

		float fringe = (m_renderer->edgeAntiAlias() && s.shapeAntiAlias) ? m_fringeWidth : 0.0f;
		float fx = fringe;
		float fy = fringe;

		float Rqx = rx + fx;
		float Rqy = ry + fy;
		float scaleX = Rqx / rx;
		float scaleY = Rqy / ry;
		uint32_t c = fillPaint.innerColor.premultiplied().toRGBA8();

		Vertex quad[6];
		// Triangle 1 (p0 -> p3 -> p2: CCW in NDC)
		quad[0] = Vertex(cx - Rqx, cy - Rqy, -scaleX, -scaleY, c);
		quad[1] = Vertex(cx - Rqx, cy + Rqy, -scaleX,  scaleY, c);
		quad[2] = Vertex(cx + Rqx, cy + Rqy,  scaleX,  scaleY, c);
		// Triangle 2 (p0 -> p2 -> p1: CCW in NDC)
		quad[3] = Vertex(cx - Rqx, cy - Rqy, -scaleX, -scaleY, c);
		quad[4] = Vertex(cx + Rqx, cy + Rqy,  scaleX,  scaleY, c);
		quad[5] = Vertex(cx + Rqx, cy - Rqy,  scaleX, -scaleY, c);

		m_renderer->renderTriangles(fillPaint, s.compositeOperation, s.scissor, quad, 6, m_fringeWidth, 7 /* ShaderCircle */);
		return;
	}

	if (m_renderer->backendType() == GpuBackendType::OpenGL && m_singleRect && isAxisAligned && m_rectW > 0.0f && m_rectH > 0.0f) {
		float sx = s.xform[0];
		float sy = s.xform[3];
		float fringe = (m_renderer->edgeAntiAlias() && s.shapeAntiAlias) ? m_fringeWidth : 0.0f;
		float fx = fringe / sx;
		float fy = fringe / sy;

		float xA = m_rectX - fx;
		float xB = m_rectX + m_rectW + fx;
		float yA = m_rectY - fy;
		float yB = m_rectY + m_rectH + fy;

		Point p0 = s.xform.transformPoint(xA, yA);
		Point p1 = s.xform.transformPoint(xB, yA);
		Point p2 = s.xform.transformPoint(xB, yB);
		Point p3 = s.xform.transformPoint(xA, yB);

		fillPaint.innerColor.a *= s.alpha;
		fillPaint.outerColor.a *= s.alpha;
		uint32_t c = fillPaint.innerColor.premultiplied().toRGBA8();

		bool coversViewport = (s.scissor.extent[0] < 0.0f) &&
		                      (p0.x <= 0.0f && p0.y <= 0.0f && p2.x >= m_windowWidth && p2.y >= m_windowHeight);

		if (isSolid && (fringe <= 0.0f || coversViewport)) {
			Vertex quad[6];
			quad[0] = Vertex(p0.x, p0.y, 0.5f, 1.0f, c);
			quad[1] = Vertex(p3.x, p3.y, 0.5f, 1.0f, c);
			quad[2] = Vertex(p2.x, p2.y, 0.5f, 1.0f, c);
			quad[3] = Vertex(p0.x, p0.y, 0.5f, 1.0f, c);
			quad[4] = Vertex(p2.x, p2.y, 0.5f, 1.0f, c);
			quad[5] = Vertex(p1.x, p1.y, 0.5f, 1.0f, c);
			m_renderer->renderTriangles(fillPaint, s.compositeOperation, s.scissor, quad, 6, m_fringeWidth, 4 /* ShaderSolid */);
			return;
		}

		if (!isSolid && coversViewport) {
			Vertex quad[6];
			quad[0] = Vertex(p0.x, p0.y, 0.5f, 1.0f, c);
			quad[1] = Vertex(p3.x, p3.y, 0.5f, 1.0f, c);
			quad[2] = Vertex(p2.x, p2.y, 0.5f, 1.0f, c);
			quad[3] = Vertex(p0.x, p0.y, 0.5f, 1.0f, c);
			quad[4] = Vertex(p2.x, p2.y, 0.5f, 1.0f, c);
			quad[5] = Vertex(p1.x, p1.y, 0.5f, 1.0f, c);
			m_renderer->renderTriangles(fillPaint, s.compositeOperation, s.scissor, quad, 6, m_fringeWidth, -1 /* derive from paint */);
			return;
		}

		if (isSolid) {
			float uA = (fringe > 0.0f) ? (-1.0f - (2.0f * fx) / m_rectW) : -1.0f;
			float uB = (fringe > 0.0f) ? ( 1.0f + (2.0f * fx) / m_rectW) :  1.0f;
			float vA = (fringe > 0.0f) ? (-1.0f - (2.0f * fy) / m_rectH) : -1.0f;
			float vB = (fringe > 0.0f) ? ( 1.0f + (2.0f * fy) / m_rectH) :  1.0f;

			Vertex quad[6];
			// Triangle 1 (p0 -> p3 -> p2: CCW in NDC)
			quad[0] = Vertex(p0.x, p0.y, uA, vA, c);
			quad[1] = Vertex(p3.x, p3.y, uA, vB, c);
			quad[2] = Vertex(p2.x, p2.y, uB, vB, c);
			// Triangle 2 (p0 -> p2 -> p1: CCW in NDC)
			quad[3] = Vertex(p0.x, p0.y, uA, vA, c);
			quad[4] = Vertex(p2.x, p2.y, uB, vB, c);
			quad[5] = Vertex(p1.x, p1.y, uB, vA, c);

			m_renderer->renderTriangles(fillPaint, s.compositeOperation, s.scissor, quad, 6, m_fringeWidth, 8 /* ShaderRect */);
			return;
		}
	}

	if (m_renderer->backendType() == GpuBackendType::OpenGL && (m_singleRRect || m_singleVaryingRRect) && isConformal && isSolid && m_rrectW > 0.0f && m_rrectH > 0.0f) {
		float sc = std::sqrt(s.xform[0] * s.xform[0] + s.xform[1] * s.xform[1]);
		if (sc < 1e-6f) sc = 1.0f;
		float fringe = (m_renderer->edgeAntiAlias() && s.shapeAntiAlias) ? m_fringeWidth : 0.0f;
		float fx = (fringe > 0.0f) ? (fringe / sc) : 0.0f;
		float fy = fx;

		float hw = m_rrectW * 0.5f;
		float hh = m_rrectH * 0.5f;
		float radTL = m_singleVaryingRRect ? m_radTL : m_rrectR;
		float radTR = m_singleVaryingRRect ? m_radTR : m_rrectR;
		float radBR = m_singleVaryingRRect ? m_radBR : m_rrectR;
		float radBL = m_singleVaryingRRect ? m_radBL : m_rrectR;

		float xA = m_rrectX - fx;
		float xB = m_rrectX + m_rrectW + fx;
		float yA = m_rrectY - fy;
		float yB = m_rrectY + m_rrectH + fy;

		float uA = -hw - fx;
		float uB =  hw + fx;
		float vA = -hh - fy;
		float vB =  hh + fy;

		Point p0 = s.xform.transformPoint(xA, yA);
		Point p1 = s.xform.transformPoint(xB, yA);
		Point p2 = s.xform.transformPoint(xB, yB);
		Point p3 = s.xform.transformPoint(xA, yB);

		fillPaint.innerColor.a *= s.alpha;
		fillPaint.outerColor.a *= s.alpha;
		uint32_t c = fillPaint.innerColor.premultiplied().toRGBA8();

		Vertex quad[6];
		// Triangle 1 (p0 -> p3 -> p2: CCW in NDC)
		quad[0] = Vertex(p0.x, p0.y, uA, vA, c);
		quad[1] = Vertex(p3.x, p3.y, uA, vB, c);
		quad[2] = Vertex(p2.x, p2.y, uB, vB, c);
		// Triangle 2 (p0 -> p2 -> p1: CCW in NDC)
		quad[3] = Vertex(p0.x, p0.y, uA, vA, c);
		quad[4] = Vertex(p2.x, p2.y, uB, vB, c);
		quad[5] = Vertex(p1.x, p1.y, uB, vA, c);

		fillPaint.extent[0] = hw;
		fillPaint.extent[1] = hh;
		fillPaint.radius = radTL;
		fillPaint.feather = radTR;
		fillPaint.xform[0] = radBR;
		fillPaint.xform[1] = radBL;

		m_renderer->renderTriangles(fillPaint, s.compositeOperation, s.scissor, quad, 6, m_fringeWidth, 9 /* ShaderRRect */);
		return;
	}

	materializePending();
	m_tessellator->flattenCommands(m_commands);

	float fringe = (m_renderer->edgeAntiAlias() && s.shapeAntiAlias) ? m_fringeWidth : 0.0f;
	m_tessellator->expandFill(fringe, LineJoin::Miter, 2.4f);

	fillPaint.innerColor.a *= s.alpha;
	fillPaint.outerColor.a *= s.alpha;

	const auto& paths = m_tessellator->contours();
	const auto& allVerts = m_tessellator->vertices();
	m_renderPaths.resize(paths.size());

	for (size_t i = 0; i < paths.size(); ++i) {
		const auto& tp = paths[i];
		auto& p = m_renderPaths[i];
		p.first = tp.first;
		p.count = tp.count;
		p.closed = tp.closed;
		p.nbevel = tp.nbevel;
		p.fill = (tp.fillCount > 0) ? &allVerts[tp.fillOffset] : nullptr;
		p.fillCount = tp.fillCount;
		p.stroke = (tp.strokeCount > 0) ? &allVerts[tp.strokeOffset] : nullptr;
		p.strokeCount = tp.strokeCount;
		p.winding = tp.winding;
		p.convex = tp.convex;
	}

	m_renderer->renderFill(fillPaint, s.compositeOperation, s.scissor, m_fringeWidth,
	                       m_tessellator->bounds(), m_renderPaths.data(), static_cast<int>(m_renderPaths.size()));
}

void Context::stroke() {
	State& s = currentState();
	float scale = s.xform.averageScale();
	float strokeW = std::clamp(s.strokeWidth * scale, 0.0f, 200.0f);
	Paint strokePaint = s.stroke;

	if (strokeW < m_fringeWidth) {
		float alpha = std::clamp(strokeW / m_fringeWidth, 0.0f, 1.0f);
		strokePaint.innerColor.a *= alpha * alpha;
		strokePaint.outerColor.a *= alpha * alpha;
		strokeW = m_fringeWidth;
	}

	strokePaint.innerColor.a *= s.alpha;
	strokePaint.outerColor.a *= s.alpha;

	bool isConformal = std::abs(s.xform[0] * s.xform[0] + s.xform[1] * s.xform[1] -
	                            (s.xform[2] * s.xform[2] + s.xform[3] * s.xform[3])) < 1e-4f &&
	                   std::abs(s.xform[0] * s.xform[2] + s.xform[1] * s.xform[3]) < 1e-4f;
	bool isSolid = (strokePaint.image == 0 && strokePaint.extent[0] == 0.0f && strokePaint.extent[1] == 0.0f &&
	                strokePaint.radius == 0.0f &&
	                strokePaint.innerColor.r == strokePaint.outerColor.r &&
	                strokePaint.innerColor.g == strokePaint.outerColor.g &&
	                strokePaint.innerColor.b == strokePaint.outerColor.b &&
	                strokePaint.innerColor.a == strokePaint.outerColor.a);

	if (m_renderer->backendType() == GpuBackendType::OpenGL && m_singleCircle && isConformal && isSolid && m_circleR > 0.0f) {
		float sc = std::sqrt(s.xform[0] * s.xform[0] + s.xform[1] * s.xform[1]);
		float rScreen = m_circleR * sc;
		if (rScreen <= 50.0f || rScreen <= strokeW * 3.0f) {
			Point pCenter = s.xform.transformPoint(m_circleCx, m_circleCy);
			float fringe = (m_renderer->edgeAntiAlias() && s.shapeAntiAlias) ? m_fringeWidth : 0.0f;
			float halfStroke = strokeW * 0.5f;
			float Rquad = rScreen + halfStroke + fringe;

			uint32_t c = strokePaint.innerColor.premultiplied().toRGBA8();

			Vertex quad[6];
			// Triangle 1 (p0 -> p3 -> p2: CCW in NDC)
			quad[0] = Vertex(pCenter.x - Rquad, pCenter.y - Rquad, -Rquad, -Rquad, c);
			quad[1] = Vertex(pCenter.x - Rquad, pCenter.y + Rquad, -Rquad,  Rquad, c);
			quad[2] = Vertex(pCenter.x + Rquad, pCenter.y + Rquad,  Rquad,  Rquad, c);
			// Triangle 2 (p0 -> p2 -> p1: CCW in NDC)
			quad[3] = Vertex(pCenter.x - Rquad, pCenter.y - Rquad, -Rquad, -Rquad, c);
			quad[4] = Vertex(pCenter.x + Rquad, pCenter.y + Rquad,  Rquad,  Rquad, c);
			quad[5] = Vertex(pCenter.x + Rquad, pCenter.y - Rquad,  Rquad, -Rquad, c);

			strokePaint.radius = rScreen;
			strokePaint.feather = halfStroke;

			m_renderer->renderTriangles(strokePaint, s.compositeOperation, s.scissor, quad, 6, m_fringeWidth, 10 /* ShaderRing */);
			return;
		}
	}

	materializePending();

	m_tessellator->flattenCommands(m_commands);

	float fringe = (m_renderer->edgeAntiAlias() && s.shapeAntiAlias) ? m_fringeWidth : 0.0f;
	m_tessellator->expandStroke(strokeW * 0.5f, fringe, s.lineCap, s.lineJoin, s.miterLimit);

	const auto& paths = m_tessellator->contours();
	const auto& allVerts = m_tessellator->vertices();
	m_renderPaths.resize(paths.size());

	for (size_t i = 0; i < paths.size(); ++i) {
		const auto& tp = paths[i];
		auto& p = m_renderPaths[i];
		p.first = tp.first;
		p.count = tp.count;
		p.closed = tp.closed;
		p.nbevel = tp.nbevel;
		p.fill = (tp.fillCount > 0) ? &allVerts[tp.fillOffset] : nullptr;
		p.fillCount = tp.fillCount;
		p.stroke = (tp.strokeCount > 0) ? &allVerts[tp.strokeOffset] : nullptr;
		p.strokeCount = tp.strokeCount;
		p.winding = tp.winding;
		p.convex = tp.convex;
	}

	m_renderer->renderStroke(strokePaint, s.compositeOperation, s.scissor, m_fringeWidth,
	                         strokeW, m_renderPaths.data(), static_cast<int>(m_renderPaths.size()));
}

// -------------------------------------------------------------
// Typography & Text (Sovereign Nisaba C++20 Engine)
// -------------------------------------------------------------
int Context::createFont(const char* name, const char* filename) {
	if (!filename) return -1;
	auto idOpt = m_fontSystem.load_font_file(filename);
	if (!idOpt) return -1;
	int id = static_cast<int>(*idOpt);
	if (name) {
		m_fontNames[name] = id;
	}
	if (currentState().fontId <= 0) {
		currentState().fontId = id;
	}
	return id;
}

int Context::createFontAtIndex(const char* name, const char* filename, const int fontIndex) {
	(void)fontIndex;
	return createFont(name, filename);
}

int Context::createFontMem(const char* name, unsigned char* data, int ndata, int freeData) {
	if (!data || ndata <= 0) return -1;
	auto idOpt = m_fontSystem.load_font_data(std::span<const uint8_t>(data, static_cast<size_t>(ndata)));
	if (freeData && data) {
		free(data);
	}
	if (!idOpt) return -1;
	int id = static_cast<int>(*idOpt);
	if (name) {
		m_fontNames[name] = id;
	}
	if (currentState().fontId <= 0) {
		currentState().fontId = id;
	}
	return id;
}

int Context::createFontMemAtIndex(const char* name, unsigned char* data, int ndata, int freeData, const int fontIndex) {
	(void)fontIndex;
	return createFontMem(name, data, ndata, freeData);
}

int Context::findFont(const char* name) {
	if (!name) return -1;
	auto it = m_fontNames.find(name);
	if (it != m_fontNames.end()) return it->second;
	return -1;
}

int Context::addFallbackFontId(int baseFont, int fallbackFont) {
	if (baseFont <= 0 || fallbackFont <= 0) return 0;
	m_fallbacks[baseFont].push_back(fallbackFont);
	return 1;
}

int Context::addFallbackFont(const char* baseFont, const char* fallbackFont) {
	return addFallbackFontId(findFont(baseFont), findFont(fallbackFont));
}

void Context::resetFallbackFontsId(int baseFont) {
	m_fallbacks.erase(baseFont);
}

void Context::resetFallbackFonts(const char* baseFont) {
	resetFallbackFontsId(findFont(baseFont));
}

void Context::fontSize(float size) {
	currentState().fontSize = size;
}

void Context::fontBlur(float blur) {
	currentState().fontBlur = blur;
}

void Context::textLetterSpacing(float spacing) {
	currentState().letterSpacing = spacing;
}

void Context::textLineHeight(float lineHeight) {
	currentState().lineHeight = lineHeight;
}

void Context::textAlign(Align align) {
	currentState().textAlign = align;
}

void Context::fontFaceId(int font) {
	currentState().fontId = font;
}

void Context::fontFace(const char* font) {
	currentState().fontId = findFont(font);
}

static float getFontScale(const State& state) {
	return std::min(quantize(state.xform.averageScale(), 0.01f), 4.0f);
}

const text::TtfFont* Context::resolveGlyphFont(int primaryFontId, char32_t cp, uint16_t* outGlyphId) const {
	if (primaryFontId > 0) {
		const text::TtfFont* font = m_fontSystem.get_font(static_cast<uint32_t>(primaryFontId));
		if (font) {
			uint16_t gid = font->glyph_index(cp);
			if (gid != 0) {
				*outGlyphId = gid;
				return font;
			}
		}
		auto it = m_fallbacks.find(primaryFontId);
		if (it != m_fallbacks.end()) {
			for (int fbId : it->second) {
				const text::TtfFont* fbFont = m_fontSystem.get_font(static_cast<uint32_t>(fbId));
				if (fbFont) {
					uint16_t gid = fbFont->glyph_index(cp);
					if (gid != 0) {
						*outGlyphId = gid;
						return fbFont;
					}
				}
			}
		}
	}

	auto [fbId, gid] = m_fontSystem.find_glyph_or_fallback(static_cast<uint32_t>(std::max(1, primaryFontId)), cp);
	if (gid != 0) {
		*outGlyphId = gid;
		return m_fontSystem.get_font(fbId);
	}

	const text::TtfFont* primary = m_fontSystem.get_font(static_cast<uint32_t>(primaryFontId));
	*outGlyphId = 0;
	return primary;
}

struct MeasuredGlyph {
	const text::TtfFont* font{nullptr};
	uint32_t fontId{0};
	uint16_t glyphId{0};
	float x{0.0f};
	float y{0.0f};
	float advance{0.0f};
	float scale{1.0f};
	const char* str{nullptr};
};

static inline bool isStringPureAscii(const char* str, const char* end) noexcept {
	for (const char* p = str; p < end; ++p) {
		if (static_cast<unsigned char>(*p) >= 128) return false;
	}
	return true;
}

std::vector<MeasuredGlyph> Context::layoutGlyphsInternal(
	const State& s,
	const text::TtfFont* primaryFont,
	uint32_t fontId,
	float scaledSize,
	float fontSize,
	const char* string,
	const char* end,
	float* outTotalWidth) const {

	std::vector<MeasuredGlyph> glyphs;
	if (!string || string == end || !primaryFont) {
		if (outTotalWidth) *outTotalWidth = 0.0f;
		return glyphs;
	}

	float scale = primaryFont->scale_for_size(scaledSize);
	float curX = 0.0f;
	uint16_t prevGid = 0;
	float letterSpace = s.letterSpacing * (fontSize > 0.0f ? (scaledSize / fontSize) : 1.0f);

	if (isStringPureAscii(string, end)) {
		glyphs.reserve(end - string);
		for (const char* p = string; p < end; ++p) {
			uint8_t c = static_cast<uint8_t>(*p);
			uint16_t gid = primaryFont->ascii_glyph_index(c);
			const text::TtfFont* font = primaryFont;
			uint32_t fid = fontId;

			if (gid == 0 && c != ' ' && c != '\t' && c != '\r' && c != '\n') {
				font = resolveGlyphFont(static_cast<int>(fontId), c, &gid);
				if (!font) font = primaryFont;
				fid = (font == primaryFont) ? fontId : 0;
			}

			float adv = (font == primaryFont) ?
				(static_cast<float>(primaryFont->ascii_advance(c)) * scale) :
				font->glyph_advance(gid, scaledSize);

			if (prevGid != 0 && gid != 0 && font == primaryFont) {
				int16_t kern = primaryFont->get_kerning(prevGid, gid);
				curX += static_cast<float>(kern) * scale;
			}
			prevGid = (font == primaryFont) ? gid : 0;

			MeasuredGlyph mg;
			mg.font = font;
			mg.fontId = fid;
			mg.glyphId = gid;
			mg.x = curX;
			mg.y = 0.0f;
			mg.advance = adv;
			mg.scale = scale;
			mg.str = p;
			glyphs.push_back(mg);

			curX += adv + letterSpace;
		}
	} else {
		std::string_view sv(string, end - string);
		auto bidiRuns = text::Bidi::segment_runs(sv);
		auto shaped = text::Bidi::shape_text(sv, bidiRuns);

		glyphs.reserve(shaped.size());
		for (const auto& sc : shaped) {
			uint16_t gid = 0;
			const text::TtfFont* font = resolveGlyphFont(static_cast<int>(fontId), sc.codepoint, &gid);
			if (!font) {
				gid = primaryFont->glyph_index(sc.codepoint);
				font = primaryFont;
			}

			float fScale = font->scale_for_size(scaledSize);
			float adv = font->glyph_advance_scaled(gid, fScale);

			if (prevGid != 0 && gid != 0) {
				int16_t kern = font->get_kerning(prevGid, gid);
				curX += static_cast<float>(kern) * fScale;
			}
			prevGid = gid;

			MeasuredGlyph mg;
			mg.font = font;
			mg.fontId = (font == primaryFont) ? fontId : 0;
			mg.glyphId = gid;
			mg.x = curX;
			mg.y = 0.0f;
			mg.advance = adv;
			mg.scale = fScale;
			mg.str = string + sc.byte_index;
			glyphs.push_back(mg);

			curX += adv + letterSpace;
		}
	}

	if (outTotalWidth) *outTotalWidth = curX;
	return glyphs;
}

std::vector<MeasuredGlyph> Context::layoutGlyphs(
	const State& s,
	float scaledSize,
	const char* string,
	const char* end,
	float* outTotalWidth) const {

	if (s.fontId <= 0) {
		if (outTotalWidth) *outTotalWidth = 0.0f;
		return {};
	}
	const text::TtfFont* primaryFont = m_fontSystem.get_font(static_cast<uint32_t>(s.fontId));
	return layoutGlyphsInternal(s, primaryFont, static_cast<uint32_t>(s.fontId), scaledSize, s.fontSize, string, end, outTotalWidth);
}

static void computeAlignmentOffsets(const State& s, const text::TtfFont* font, float scaledSize, float totalWidth, float& outAlignX, float& outAlignY) {
	outAlignX = 0.0f;
	if (hasFlag(s.textAlign, Align::Center)) {
		outAlignX = -totalWidth * 0.5f;
	} else if (hasFlag(s.textAlign, Align::Right)) {
		outAlignX = -totalWidth;
	}

	outAlignY = 0.0f;
	if (font) {
		float fontAscent = font->ascent(scaledSize);
		float fontDescent = font->descent(scaledSize);
		if (hasFlag(s.textAlign, Align::Top)) {
			outAlignY = fontAscent;
		} else if (hasFlag(s.textAlign, Align::Middle)) {
			outAlignY = (fontAscent + fontDescent) * 0.5f;
		} else if (hasFlag(s.textAlign, Align::Bottom)) {
			outAlignY = fontDescent;
		} else if (hasFlag(s.textAlign, Align::Baseline)) {
			outAlignY = 0.0f;
		}
	}
}

float Context::text(float x, float y, const char* string, const char* end) {
	State& s = currentState();
	if (s.fontId <= 0) {
		int defFont = findFont("sans");
		if (defFont <= 0 && !m_fontNames.empty()) defFont = m_fontNames.begin()->second;
		if (defFont > 0) s.fontId = defFont;
	}
	if (s.fontId <= 0 || !string) return x;
	if (!end) end = string + std::strlen(string);
	if (string == end) return x;

	float scale = getFontScale(s) * m_devicePixelRatio;
	float invscale = 1.0f / scale;
	float scaledSize = s.fontSize * scale;

	const text::TtfFont* primaryFont = m_fontSystem.get_font(static_cast<uint32_t>(s.fontId));
	if (!primaryFont) return x;

	float totalWidth = 0.0f;
	auto glyphs = layoutGlyphs(s, scaledSize, string, end, &totalWidth);
	if (glyphs.empty()) return x;

	float alignX = 0.0f, alignY = 0.0f;
	computeAlignmentOffsets(s, primaryFont, scaledSize, totalWidth, alignX, alignY);

	float startX = x * scale + alignX;
	float baselineY = y * scale + alignY;
	bool isFlipped = isTransformFlipped(s.xform);

	m_textVertices.clear();
	m_textVertices.reserve(glyphs.size() * 6);

	uint32_t sizeBits = 0;
	std::memcpy(&sizeBits, &scaledSize, sizeof(float));

	for (const auto& g : glyphs) {
		if (g.glyphId == 0 || !g.font) continue;

		float targetX = startX + g.x;
		float targetY = baselineY + g.y;

		auto [intX, xBin] = text::compute_subpixel_bin(targetX);
		auto [intY, yBin] = text::compute_subpixel_bin(targetY);

		text::CacheKey key{g.fontId, g.glyphId, sizeBits, xBin, yBin};
		const AtlasGlyphEntry* entry = m_atlas->find(key);

		if (!entry) {
			const text::CachedGlyph* cg = m_glyphCache.get_or_render(
				*g.font, g.fontId, g.glyphId, scaledSize, targetX, targetY);

			auto inserted = m_atlas->insert(key, cg);
			if (!inserted) {
				// Atlas is full, flush current batch and expand or reset atlas
				if (!m_textVertices.empty()) {
					m_atlas->flushToGpu(m_renderer.get(), m_fontTextureId);
					Paint p = s.fill;
					p.image = m_fontTextureId;
					p.innerColor.a *= s.alpha;
					p.outerColor.a *= s.alpha;
					m_renderer->renderTriangles(p, s.compositeOperation, s.scissor,
					                           m_textVertices.data(), static_cast<int>(m_textVertices.size()), m_fringeWidth);
					m_textVertices.clear();
				}
				if (m_atlas->width() < 2048 || m_atlas->height() < 2048) {
					if (m_fontTextureId != 0 && m_renderer) {
						m_renderer->deleteTexture(m_fontTextureId);
					}
					m_atlas->resize(2048, 2048);
					m_fontTextureId = m_renderer->createTexture(TextureType::Alpha, m_atlas->width(), m_atlas->height(), 0, m_atlas->data());
					inserted = m_atlas->insert(key, cg);
				} else {
					m_atlas->reset();
					inserted = m_atlas->insert(key, cg);
				}
			}
			entry = inserted ? &(*inserted) : nullptr;
		}

		if (!entry || entry->width == 0 || entry->height == 0) continue;

		float qx0 = static_cast<float>(intX + entry->offsetX) * invscale;
		float qy0 = static_cast<float>(intY + entry->offsetY) * invscale;
		float qx1 = qx0 + static_cast<float>(entry->width) * invscale;
		float qy1 = qy0 + static_cast<float>(entry->height) * invscale;

		float u0 = entry->u0;
		float v0 = entry->v0;
		float u1 = entry->u1;
		float v1 = entry->v1;

		if (isFlipped) {
			std::swap(qy0, qy1);
			std::swap(v0, v1);
		}

		Point c0 = s.xform.transformPoint(qx0, qy0);
		Point c1 = s.xform.transformPoint(qx1, qy0);
		Point c2 = s.xform.transformPoint(qx1, qy1);
		Point c3 = s.xform.transformPoint(qx0, qy1);

		m_textVertices.emplace_back(c0.x, c0.y, u0, v0);
		m_textVertices.emplace_back(c2.x, c2.y, u1, v1);
		m_textVertices.emplace_back(c1.x, c1.y, u1, v0);

		m_textVertices.emplace_back(c0.x, c0.y, u0, v0);
		m_textVertices.emplace_back(c3.x, c3.y, u0, v1);
		m_textVertices.emplace_back(c2.x, c2.y, u1, v1);
	}

	m_atlas->flushToGpu(m_renderer.get(), m_fontTextureId);

	if (!m_textVertices.empty()) {
		Paint p = s.fill;
		p.image = m_fontTextureId;
		p.innerColor.a *= s.alpha;
		p.outerColor.a *= s.alpha;
		m_renderer->renderTriangles(p, s.compositeOperation, s.scissor,
		                           m_textVertices.data(), static_cast<int>(m_textVertices.size()), m_fringeWidth);
	}

	return x + totalWidth * invscale;
}

float Context::textWithFont(float x, float y, const text::TtfFont& font, float fontSize, const char* string, const char* end) {
	State& s = currentState();
	if (!string) return x;
	if (!end) end = string + std::strlen(string);
	if (string == end) return x;

	float scale = getFontScale(s) * m_devicePixelRatio;
	float invscale = 1.0f / scale;
	float scaledSize = fontSize * scale;

	float totalWidth = 0.0f;
	uint32_t fontId = (static_cast<uint32_t>(std::hash<const text::TtfFont*>{}(&font)) & 0x7FFFFFFFu) | 0x80000000u;
	auto glyphs = layoutGlyphsInternal(s, &font, fontId, scaledSize, fontSize, string, end, &totalWidth);
	if (glyphs.empty()) return x;

	float alignX = 0.0f, alignY = 0.0f;
	computeAlignmentOffsets(s, &font, scaledSize, totalWidth, alignX, alignY);

	float startX = x * scale + alignX;
	float baselineY = y * scale + alignY;
	bool isFlipped = isTransformFlipped(s.xform);

	m_textVertices.clear();
	m_textVertices.reserve(glyphs.size() * 6);

	uint32_t sizeBits = 0;
	std::memcpy(&sizeBits, &scaledSize, sizeof(float));

	for (const auto& g : glyphs) {
		if (g.glyphId == 0 || !g.font) continue;

		float targetX = startX + g.x;
		float targetY = baselineY + g.y;

		auto [intX, xBin] = text::compute_subpixel_bin(targetX);
		auto [intY, yBin] = text::compute_subpixel_bin(targetY);

		text::CacheKey key{g.fontId, g.glyphId, sizeBits, xBin, yBin};
		const AtlasGlyphEntry* entry = m_atlas->find(key);

		if (!entry) {
			const text::CachedGlyph* cg = m_glyphCache.get_or_render(
				*g.font, g.fontId, g.glyphId, scaledSize, targetX, targetY);

			auto inserted = m_atlas->insert(key, cg);
			if (!inserted) {
				if (!m_textVertices.empty()) {
					m_atlas->flushToGpu(m_renderer.get(), m_fontTextureId);
					Paint p = s.fill;
					p.image = m_fontTextureId;
					p.innerColor.a *= s.alpha;
					p.outerColor.a *= s.alpha;
					m_renderer->renderTriangles(p, s.compositeOperation, s.scissor,
					                           m_textVertices.data(), static_cast<int>(m_textVertices.size()), m_fringeWidth);
					m_textVertices.clear();
				}
				if (m_atlas->width() < 2048 || m_atlas->height() < 2048) {
					if (m_fontTextureId != 0 && m_renderer) {
						m_renderer->deleteTexture(m_fontTextureId);
					}
					m_atlas->resize(2048, 2048);
					m_fontTextureId = m_renderer->createTexture(TextureType::Alpha, m_atlas->width(), m_atlas->height(), 0, m_atlas->data());
					inserted = m_atlas->insert(key, cg);
				} else {
					m_atlas->reset();
					inserted = m_atlas->insert(key, cg);
				}
			}
			entry = inserted ? &(*inserted) : nullptr;
		}

		if (!entry || entry->width == 0 || entry->height == 0) continue;

		float qx0 = static_cast<float>(intX + entry->offsetX) * invscale;
		float qy0 = static_cast<float>(intY + entry->offsetY) * invscale;
		float qx1 = qx0 + static_cast<float>(entry->width) * invscale;
		float qy1 = qy0 + static_cast<float>(entry->height) * invscale;

		float u0 = entry->u0;
		float v0 = entry->v0;
		float u1 = entry->u1;
		float v1 = entry->v1;

		if (isFlipped) {
			std::swap(qy0, qy1);
			std::swap(v0, v1);
		}

		Point c0 = s.xform.transformPoint(qx0, qy0);
		Point c1 = s.xform.transformPoint(qx1, qy0);
		Point c2 = s.xform.transformPoint(qx1, qy1);
		Point c3 = s.xform.transformPoint(qx0, qy1);

		m_textVertices.emplace_back(c0.x, c0.y, u0, v0);
		m_textVertices.emplace_back(c2.x, c2.y, u1, v1);
		m_textVertices.emplace_back(c1.x, c1.y, u1, v0);

		m_textVertices.emplace_back(c0.x, c0.y, u0, v0);
		m_textVertices.emplace_back(c3.x, c3.y, u0, v1);
		m_textVertices.emplace_back(c2.x, c2.y, u1, v1);
	}

	m_atlas->flushToGpu(m_renderer.get(), m_fontTextureId);

	if (!m_textVertices.empty()) {
		Paint p = s.fill;
		p.image = m_fontTextureId;
		p.innerColor.a *= s.alpha;
		p.outerColor.a *= s.alpha;
		m_renderer->renderTriangles(p, s.compositeOperation, s.scissor,
		                           m_textVertices.data(), static_cast<int>(m_textVertices.size()), m_fringeWidth);
	}

	return x + totalWidth * invscale;
}

void Context::drawTextBuffer(const text::Buffer& buffer, text::FontSystem& fontSystem, Point pos, Color defaultColor) {
	State& s = currentState();

	bool need_shaping = false;
	for (const auto& line : buffer.lines()) {
		if (!line.is_shaped()) {
			need_shaping = true;
			break;
		}
	}
	if (need_shaping) {
		const_cast<text::Buffer&>(buffer).shape_until_scroll(fontSystem);
	}

	auto runs = buffer.layout_runs();
	if (runs.empty()) return;

	float scale = getFontScale(s) * m_devicePixelRatio;
	float invscale = 1.0f / scale;
	bool isFlipped = isTransformFlipped(s.xform);

	m_textVertices.clear();

	uint32_t defColorRgba = defaultColor.premultiplied().toRGBA8();

	auto flushBatch = [&]() {
		if (!m_textVertices.empty()) {
			m_atlas->flushToGpu(m_renderer.get(), m_fontTextureId);
			Paint p;
			p.image = m_fontTextureId;
			p.innerColor = Color(1.0f, 1.0f, 1.0f, s.alpha);
			p.outerColor = Color(1.0f, 1.0f, 1.0f, s.alpha);
			m_renderer->renderTriangles(p, s.compositeOperation, s.scissor,
			                           m_textVertices.data(), static_cast<int>(m_textVertices.size()), m_fringeWidth);
			m_textVertices.clear();
		}
	};

	for (const auto& run : runs) {
		float line_baseline = pos.y + run.line_top + buffer.metrics().font_size;

		for (const auto& glyph : run.glyphs) {
			if (glyph.glyph_id == 0) continue;

			const text::TtfFont* font = fontSystem.get_font(glyph.font_id);
			if (!font) {
				if (auto defId = fontSystem.default_font_id()) {
					font = fontSystem.get_font(*defId);
				}
			}
			if (!font) continue;

			float scaledSize = glyph.font_size * scale;
			uint32_t sizeBits = 0;
			std::memcpy(&sizeBits, &scaledSize, sizeof(float));

			float glyph_x = pos.x + glyph.x + glyph.x_offset;
			float glyph_y = line_baseline + glyph.y + glyph.y_offset;

			float targetX = glyph_x * scale;
			float targetY = glyph_y * scale;

			auto [intX, xBin] = text::compute_subpixel_bin(targetX);
			auto [intY, yBin] = text::compute_subpixel_bin(targetY);

			text::CacheKey key{glyph.font_id, glyph.glyph_id, sizeBits, xBin, yBin};
			const AtlasGlyphEntry* entry = m_atlas->find(key);

			if (!entry) {
				const text::CachedGlyph* cg = m_glyphCache.get_or_render(
					*font, glyph.font_id, glyph.glyph_id, scaledSize, targetX, targetY);

				auto inserted = m_atlas->insert(key, cg);
				if (!inserted) {
					if (m_atlas->width() < 2048 || m_atlas->height() < 2048) {
						flushBatch();
						if (m_fontTextureId != 0 && m_renderer) {
							m_renderer->deleteTexture(m_fontTextureId);
						}
						m_atlas->resize(2048, 2048);
						m_fontTextureId = m_renderer->createTexture(TextureType::Alpha, 2048, 2048, 0, m_atlas->data());
						inserted = m_atlas->insert(key, cg);
					} else {
						flushBatch();
						m_atlas->reset();
						inserted = m_atlas->insert(key, cg);
					}
				}
				entry = inserted ? &(*inserted) : nullptr;
			}

			if (!entry || entry->width == 0 || entry->height == 0) continue;

			float qx0 = static_cast<float>(intX + entry->offsetX) * invscale;
			float qy0 = static_cast<float>(intY + entry->offsetY) * invscale;
			float qx1 = qx0 + static_cast<float>(entry->width) * invscale;
			float qy1 = qy0 + static_cast<float>(entry->height) * invscale;

			float u0 = entry->u0;
			float v0 = entry->v0;
			float u1 = entry->u1;
			float v1 = entry->v1;

			if (isFlipped) {
				std::swap(qy0, qy1);
				std::swap(v0, v1);
			}

			Point c0 = s.xform.transformPoint(qx0, qy0);
			Point c1 = s.xform.transformPoint(qx1, qy0);
			Point c2 = s.xform.transformPoint(qx1, qy1);
			Point c3 = s.xform.transformPoint(qx0, qy1);

			uint32_t vcolor = defColorRgba;
			if (glyph.color_opt.has_value()) {
				nisaba::Color c = glyph.color_opt->to_color();
				vcolor = Color(c).premultiplied().toRGBA8();
			}

			m_textVertices.emplace_back(c0.x, c0.y, u0, v0, vcolor);
			m_textVertices.emplace_back(c2.x, c2.y, u1, v1, vcolor);
			m_textVertices.emplace_back(c1.x, c1.y, u1, v0, vcolor);

			m_textVertices.emplace_back(c0.x, c0.y, u0, v0, vcolor);
			m_textVertices.emplace_back(c3.x, c3.y, u0, v1, vcolor);
			m_textVertices.emplace_back(c2.x, c2.y, u1, v1, vcolor);
		}
	}

	flushBatch();
}


float Context::textBounds(float x, float y, const char* string, const char* end, float* bounds) {
	State& s = currentState();
	if (s.fontId <= 0 || !string) {
		if (bounds) bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0f;
		return x;
	}
	if (!end) end = string + std::strlen(string);
	if (string == end) {
		if (bounds) bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0f;
		return x;
	}

	float scale = getFontScale(s) * m_devicePixelRatio;
	float invscale = 1.0f / scale;
	float scaledSize = s.fontSize * scale;

	const text::TtfFont* primaryFont = m_fontSystem.get_font(static_cast<uint32_t>(s.fontId));
	if (!primaryFont) {
		if (bounds) bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0f;
		return x;
	}

	float totalWidth = 0.0f;
	auto glyphs = layoutGlyphs(s, scaledSize, string, end, &totalWidth);

	float alignX = 0.0f, alignY = 0.0f;
	computeAlignmentOffsets(s, primaryFont, scaledSize, totalWidth, alignX, alignY);

	if (bounds) {
		float fontAscent = primaryFont->ascent(scaledSize);
		float fontDescent = primaryFont->descent(scaledSize);

		float minX = x * scale + alignX;
		float maxX = minX + totalWidth;
		float minY = y * scale + alignY - fontAscent;
		float maxY = y * scale + alignY - fontDescent;

		bounds[0] = minX * invscale;
		bounds[1] = minY * invscale;
		bounds[2] = maxX * invscale;
		bounds[3] = maxY * invscale;
	}

	return totalWidth * invscale;
}

void Context::textMetrics(float* ascender, float* descender, float* lineh) {
	State& s = currentState();
	if (s.fontId <= 0) {
		if (ascender) *ascender = 0.0f;
		if (descender) *descender = 0.0f;
		if (lineh) *lineh = 0.0f;
		return;
	}

	const text::TtfFont* font = m_fontSystem.get_font(static_cast<uint32_t>(s.fontId));
	if (!font) {
		if (ascender) *ascender = 0.0f;
		if (descender) *descender = 0.0f;
		if (lineh) *lineh = 0.0f;
		return;
	}

	float scale = font->scale_for_size(s.fontSize);
	if (ascender) *ascender = font->ascent() * scale;
	if (descender) *descender = font->descent() * scale;
	if (lineh) *lineh = (font->ascent() - font->descent() + font->line_gap()) * scale;
}

int Context::textBreakLines(const char* string, const char* end, float breakRowWidth, TextRow* rows, int maxRows) {
	State& s = currentState();
	if (maxRows <= 0 || s.fontId <= 0 || !string) return 0;
	if (!end) end = string + std::strlen(string);
	if (string == end) return 0;

	float scale = getFontScale(s) * m_devicePixelRatio;
	float invscale = 1.0f / scale;
	float scaledSize = s.fontSize * scale;
	breakRowWidth *= scale;

	float totalWidth = 0.0f;
	auto glyphs = layoutGlyphs(s, scaledSize, string, end, &totalWidth);
	if (glyphs.empty()) return 0;

	int nrows = 0;
	const char* rowStart = nullptr;
	const char* rowEnd = nullptr;
	float rowStartX = 0.0f;
	float rowWidth = 0.0f;

	const char* wordStart = nullptr;
	float wordStartX = 0.0f;
	const char* breakEnd = nullptr;
	float breakWidth = 0.0f;

	for (size_t i = 0; i < glyphs.size(); ++i) {
		const auto& g = glyphs[i];
		char c = *g.str;
		bool isSpace = (c == ' ' || c == '\t');
		bool isNewline = (c == '\n' || c == '\r');

		if (isNewline) {
			if (c == '\r' && (i + 1 < glyphs.size()) && *glyphs[i + 1].str == '\n') {
				// Skip \r in \r\n
				continue;
			}
			rows[nrows].start = rowStart ? rowStart : g.str;
			rows[nrows].end = rowEnd ? rowEnd : g.str;
			rows[nrows].width = rowWidth * invscale;
			rows[nrows].minx = 0.0f;
			rows[nrows].maxx = rows[nrows].width;
			rows[nrows].next = g.str + 1;
			nrows++;
			if (nrows >= maxRows) return nrows;

			rowStart = rowEnd = nullptr;
			rowWidth = 0.0f;
			wordStart = breakEnd = nullptr;
			breakWidth = 0.0f;
			continue;
		}

		if (!rowStart) {
			if (!isSpace) {
				rowStart = g.str;
				rowEnd = g.str + 1;
				rowStartX = g.x;
				rowWidth = g.advance;
				wordStart = g.str;
				wordStartX = g.x;
				breakEnd = rowStart;
				breakWidth = 0.0f;
			}
		} else {
			float nextWidth = (g.x + g.advance) - rowStartX;
			if (!isSpace) {
				rowEnd = g.str + 1;
				rowWidth = nextWidth;
			}
			if (isSpace) {
				breakEnd = g.str;
				breakWidth = rowWidth;
			} else if (i > 0 && (*glyphs[i - 1].str == ' ' || *glyphs[i - 1].str == '\t')) {
				wordStart = g.str;
				wordStartX = g.x;
			}

			if (!isSpace && nextWidth > breakRowWidth) {
				if (breakEnd && breakEnd != rowStart) {
					rows[nrows].start = rowStart;
					rows[nrows].end = breakEnd;
					rows[nrows].width = breakWidth * invscale;
					rows[nrows].minx = 0.0f;
					rows[nrows].maxx = rows[nrows].width;
					rows[nrows].next = wordStart ? wordStart : g.str;
					nrows++;
					if (nrows >= maxRows) return nrows;

					rowStart = wordStart ? wordStart : g.str;
					rowStartX = wordStartX;
					rowEnd = g.str + 1;
					rowWidth = (g.x + g.advance) - rowStartX;
				} else {
					rows[nrows].start = rowStart;
					rows[nrows].end = g.str;
					rows[nrows].width = rowWidth * invscale;
					rows[nrows].minx = 0.0f;
					rows[nrows].maxx = rows[nrows].width;
					rows[nrows].next = g.str;
					nrows++;
					if (nrows >= maxRows) return nrows;

					rowStart = g.str;
					rowStartX = g.x;
					rowEnd = g.str + 1;
					rowWidth = g.advance;
					wordStart = g.str;
					wordStartX = g.x;
				}
				breakEnd = rowStart;
				breakWidth = 0.0f;
			}
		}
	}

	if (rowStart) {
		rows[nrows].start = rowStart;
		rows[nrows].end = rowEnd ? rowEnd : end;
		rows[nrows].width = rowWidth * invscale;
		rows[nrows].minx = 0.0f;
		rows[nrows].maxx = rows[nrows].width;
		rows[nrows].next = end;
		nrows++;
	}

	return nrows;
}

void Context::textBox(float x, float y, float breakRowWidth, const char* string, const char* end) {
	State& s = currentState();
	if (s.fontId <= 0 || !string) return;

	float lineh = 0.0f;
	textMetrics(nullptr, nullptr, &lineh);

	Align oldAlign = s.textAlign;
	bool halignLeft = hasFlag(oldAlign, Align::Left);
	bool halignCenter = hasFlag(oldAlign, Align::Center);
	bool halignRight = hasFlag(oldAlign, Align::Right);

	s.textAlign = Align::Left | (oldAlign & (Align::Top | Align::Middle | Align::Bottom | Align::Baseline));

	TextRow rows[2];
	int nrows = 0;

	while ((nrows = textBreakLines(string, end, breakRowWidth, rows, 2))) {
		for (int i = 0; i < nrows; i++) {
			const TextRow& row = rows[i];
			if (halignLeft)
				text(x, y, row.start, row.end);
			else if (halignCenter)
				text(x + breakRowWidth * 0.5f - row.width * 0.5f, y, row.start, row.end);
			else if (halignRight)
				text(x + breakRowWidth - row.width, y, row.start, row.end);
			y += lineh * s.lineHeight;
		}
		string = rows[nrows - 1].next;
	}

	s.textAlign = oldAlign;
}

void Context::textBoxBounds(float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds) {
	State& s = currentState();
	if (s.fontId <= 0 || !string) {
		if (bounds) bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0f;
		return;
	}

	float lineh = 0.0f;
	textMetrics(nullptr, nullptr, &lineh);

	float minx = x, miny = y, maxx = x, maxy = y;
	TextRow rows[2];
	int nrows = 0;

	while ((nrows = textBreakLines(string, end, breakRowWidth, rows, 2))) {
		for (int i = 0; i < nrows; i++) {
			const TextRow& row = rows[i];
			float rminx = x + row.minx;
			float rmaxx = x + row.maxx;
			minx = std::min(minx, rminx);
			maxx = std::max(maxx, rmaxx);
			maxy = y + lineh;
			y += lineh * s.lineHeight;
		}
		string = rows[nrows - 1].next;
	}

	if (bounds) {
		bounds[0] = minx;
		bounds[1] = miny;
		bounds[2] = maxx;
		bounds[3] = maxy;
	}
}

int Context::textGlyphPositions(float x, float y, const char* string, const char* end, GlyphPosition* positions, int maxPositions) {
	(void)y;
	State& s = currentState();
	if (s.fontId <= 0 || !string || maxPositions <= 0) return 0;
	if (!end) end = string + std::strlen(string);
	if (string == end) return 0;

	float scale = getFontScale(s) * m_devicePixelRatio;
	float invscale = 1.0f / scale;
	float scaledSize = s.fontSize * scale;

	const text::TtfFont* primaryFont = m_fontSystem.get_font(static_cast<uint32_t>(s.fontId));
	if (!primaryFont) return 0;

	float totalWidth = 0.0f;
	auto glyphs = layoutGlyphs(s, scaledSize, string, end, &totalWidth);
	if (glyphs.empty()) return 0;

	float alignX = 0.0f, alignY = 0.0f;
	computeAlignmentOffsets(s, primaryFont, scaledSize, totalWidth, alignX, alignY);

	float startX = x * scale + alignX;
	int count = std::min(static_cast<int>(glyphs.size()), maxPositions);

	for (int i = 0; i < count; ++i) {
		const auto& g = glyphs[i];
		positions[i].str = g.str;
		positions[i].x = (startX + g.x) * invscale;
		positions[i].minx = positions[i].x;
		positions[i].maxx = (startX + g.x + g.advance) * invscale;
	}

	return count;
}

} // namespace nisaba::gpu
