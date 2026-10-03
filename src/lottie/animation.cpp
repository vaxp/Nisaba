/// @file animation.cpp
/// @brief Implementation of root Lottie animation document, parsing, and rendering.

#include "nisaba/lottie/animation.hpp"
#include "nisaba/json/json.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace nisaba::lottie {

std::shared_ptr<Animation> Animation::load_from_data(std::string_view json_content) {
    auto opt_root = json::JsonValue::parse(json_content);
    if (!opt_root || !opt_root->is_object()) {
        return nullptr;
    }
    const auto& root = *opt_root;

    auto anim = std::make_shared<Animation>();
    anim->version_ = std::string(root.get_string("v", ""));
    anim->name_ = std::string(root.get_string("nm", ""));
    anim->width_ = root.get_float("w", 0.0f);
    anim->height_ = root.get_float("h", 0.0f);
    anim->in_point_ = root.get_float("ip", 0.0f);
    anim->out_point_ = root.get_float("op", 0.0f);
    anim->frame_rate_ = root.get_float("fr", 60.0f);
    if (anim->frame_rate_ <= 0.0f) anim->frame_rate_ = 60.0f;

    // Parse precomp / image assets
    if (const auto* assets_val = root.get("assets")) {
        if (assets_val->is_array()) {
            for (const auto& asset : assets_val->as_array()) {
                if (!asset.is_object()) continue;
                std::string id = std::string(asset.get_string("id", ""));
                if (id.empty()) continue;

                if (const auto* layers_val = asset.get("layers")) {
                    if (layers_val->is_array()) {
                        std::vector<std::shared_ptr<LottieLayer>> sub_layers;
                        for (const auto& l_val : layers_val->as_array()) {
                            if (auto l = LottieLayer::parse(l_val)) {
                                sub_layers.push_back(std::move(l));
                            }
                        }
                        anim->precomp_assets_[std::move(id)] = std::move(sub_layers);
                    }
                }
            }
        }
    }

    // Parse root layers
    if (const auto* layers_val = root.get("layers")) {
        if (layers_val->is_array()) {
            for (const auto& l_val : layers_val->as_array()) {
                if (auto l = LottieLayer::parse(l_val)) {
                    anim->layers_.push_back(std::move(l));
                }
            }
        }
    }

    // Parse markers if available
    if (const auto* markers_val = root.get("markers")) {
        if (markers_val->is_array()) {
            for (const auto& m_val : markers_val->as_array()) {
                if (!m_val.is_object()) continue;
                Marker m;
                m.name = std::string(m_val.get_string("cm", ""));
                m.time = m_val.get_float("tm", 0.0f);
                m.duration = m_val.get_float("dr", 0.0f);
                anim->markers_.push_back(std::move(m));
            }
        }
    }

    return anim;
}

std::shared_ptr<Animation> Animation::load_from_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return nullptr;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();
    return load_from_data(content);
}

void Animation::render(ICanvas& canvas, float frame) const {
    render_layer_tree(canvas, layers_, frame, precomp_assets_, 1.0f);
}

void Animation::render(ICanvas& canvas, float frame, const Rect& dest_rect, bool preserve_aspect_ratio) const {
    if (width_ <= 0.0f || height_ <= 0.0f || dest_rect.width() <= 0.0f || dest_rect.height() <= 0.0f) {
        return;
    }

    canvas.save();
    float sx = dest_rect.width() / width_;
    float sy = dest_rect.height() / height_;

    if (preserve_aspect_ratio) {
        float s = std::min(sx, sy);
        float ox = dest_rect.x() + (dest_rect.width() - width_ * s) * 0.5f;
        float oy = dest_rect.y() + (dest_rect.height() - height_ * s) * 0.5f;
        canvas.translate(ox, oy);
        canvas.scale(s, s);
    } else {
        canvas.translate(dest_rect.x(), dest_rect.y());
        canvas.scale(sx, sy);
    }

    render(canvas, frame);
    canvas.restore();
}

} // namespace nisaba::lottie
