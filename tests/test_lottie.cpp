/// @file test_lottie.cpp
/// @brief Comprehensive automated unit tests for sovereign JSON & Lottie animation engine.

#include "nisaba/json/json.hpp"
#include "nisaba/lottie/lottie.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/pixmap.hpp"

#include <iostream>
#include <cassert>
#include <cmath>
#include <string_view>

using namespace nisaba;
using namespace nisaba::lottie;
using namespace nisaba::json;

void test_json_parser() {
    std::cout << "[TEST] Running Sovereign JSON Parser tests...\n";

    // 1. Primitive types
    {
        auto v1 = JsonValue::parse("42.5");
        assert(v1.has_value() && v1->is_number());
        assert(std::abs(v1->to_float() - 42.5f) < 1e-4f);
        assert(v1->to_int() == 42);

        auto v2 = JsonValue::parse("\"Hello, \\\"Nisaba\\\"!\\n\"");
        assert(v2.has_value() && v2->is_string());
        assert(v2->as_string() == "Hello, \"Nisaba\"!\n");

        auto v3 = JsonValue::parse("true");
        assert(v3.has_value() && v3->is_bool() && v3->to_bool() == true);

        auto v4 = JsonValue::parse("false");
        assert(v4.has_value() && v4->is_bool() && v4->to_bool() == false);

        auto v5 = JsonValue::parse("null");
        assert(v5.has_value() && v5->is_null());
    }

    // 2. Arrays and comments
    {
        std::string_view json = R"(
            // Line comment
            [
                10,
                20.5,
                /* Block comment */
                "text",
                true,
                null
            ]
        )";
        auto arr_val = JsonValue::parse(json);
        assert(arr_val.has_value() && arr_val->is_array());
        const auto& arr = arr_val->as_array();
        assert(arr.size() == 5);
        assert(arr[0].to_int() == 10);
        assert(std::abs(arr[1].to_float() - 20.5f) < 1e-4f);
        assert(arr[2].as_string() == "text");
        assert(arr[3].to_bool() == true);
        assert(arr[4].is_null());
    }

    // 3. Nested objects
    {
        std::string_view json = R"({
            "name": "Nisaba Lottie",
            "version": "1.0",
            "fps": 60,
            "dimensions": {
                "w": 800,
                "h": 600
            },
            "tags": ["graphics", "animation", "sovereign"]
        })";
        auto obj = JsonValue::parse(json);
        assert(obj.has_value() && obj->is_object());
        assert(obj->get_string("name") == "Nisaba Lottie");
        assert(obj->get_int("fps") == 60);

        const auto* dims = obj->get("dimensions");
        assert(dims && dims->is_object());
        assert(dims->get_float("w") == 800.0f);
        assert(dims->get_float("h") == 600.0f);

        const auto* tags = obj->get("tags");
        assert(tags && tags->is_array());
        assert(tags->as_array().size() == 3);
        assert(tags->as_array()[0].as_string() == "graphics");
    }

    // 4. Unicode escape sequences \uXXXX
    {
        auto u_val = JsonValue::parse(R"("\u0041\u0042\u0043")"); // "ABC"
        assert(u_val.has_value() && u_val->is_string());
        assert(u_val->as_string() == "ABC");
    }

    std::cout << "[PASS] JSON Parser tests passed.\n";
}

void test_lottie_properties_and_easing() {
    std::cout << "[TEST] Running Lottie Properties & Bézier Easing tests...\n";

    // 1. Static property
    {
        FloatProperty p(100.0f);
        assert(!p.is_animated());
        assert(p.evaluate(0.0f) == 100.0f);
        assert(p.evaluate(50.0f) == 100.0f);
    }

    // 2. Animated keyframe with linear easing
    {
        FloatProperty p;
        Keyframe<float> kf1;
        kf1.time = 0.0f;
        kf1.start_value = 0.0f;
        kf1.end_value = 100.0f;
        kf1.out_handle = {0.0f, 0.0f};
        kf1.in_handle = {1.0f, 1.0f};
        p.add_keyframe(kf1);

        Keyframe<float> kf2;
        kf2.time = 1.0f;
        kf2.start_value = 100.0f;
        kf2.end_value = 100.0f;
        p.add_keyframe(kf2);

        assert(p.is_animated());
        assert(std::abs(p.evaluate(0.0f) - 0.0f) < 1e-3f);
        assert(std::abs(p.evaluate(0.5f) - 50.0f) < 1.0f);
        assert(std::abs(p.evaluate(1.0f) - 100.0f) < 1e-3f);
    }

    // 3. Easing curve solver test
    {
        EasingHandle out_h{0.42f, 0.0f};
        EasingHandle in_h{0.58f, 1.0f};
        float y0 = solve_bezier_easing(0.0f, out_h, in_h);
        float y1 = solve_bezier_easing(1.0f, out_h, in_h);
        float y_mid = solve_bezier_easing(0.5f, out_h, in_h);

        assert(std::abs(y0 - 0.0f) < 1e-3f);
        assert(std::abs(y1 - 1.0f) < 1e-3f);
        assert(y_mid > 0.0f && y_mid < 1.0f);
    }

    // 4. LottieTransform evaluation
    {
        LottieTransform tr;
        tr.position = PointProperty(Point::from_xy(100.0f, 200.0f));
        tr.scale = PointProperty(Point::from_xy(200.0f, 200.0f)); // 2x scale
        tr.rotation = FloatProperty(0.0f);

        Transform m = tr.matrix(0.0f);
        Point pt = Point::from_xy(10.0f, 10.0f);
        m.map_point(pt);
        // (10 * 2) + 100 = 120, (10 * 2) + 200 = 220
        assert(std::abs(pt.x - 120.0f) < 1e-3f);
        assert(std::abs(pt.y - 220.0f) < 1e-3f);
    }

    std::cout << "[PASS] Lottie Properties & Bézier Easing tests passed.\n";
}

void test_lottie_shapes() {
    std::cout << "[TEST] Running Lottie Shapes & Geometry tests...\n";

    // 1. RectShape geometry
    {
        RectShape rect;
        rect.position = PointProperty(Point::from_xy(100.0f, 100.0f));
        rect.size = PointProperty(Point::from_xy(80.0f, 60.0f));
        rect.roundness = FloatProperty(0.0f);

        Path p;
        rect.collect_geometry(0.0f, p);
        assert(!p.is_empty());
        auto bounds = p.compute_tight_bounds();
        assert(bounds.has_value());
        assert(std::abs(bounds->width() - 80.0f) < 1e-2f);
        assert(std::abs(bounds->height() - 60.0f) < 1e-2f);
    }

    // 2. EllipseShape geometry
    {
        EllipseShape el;
        el.position = PointProperty(Point::from_xy(50.0f, 50.0f));
        el.size = PointProperty(Point::from_xy(40.0f, 40.0f));

        Path p;
        el.collect_geometry(0.0f, p);
        assert(!p.is_empty());
        auto bounds = p.compute_tight_bounds();
        assert(bounds.has_value());
        assert(std::abs(bounds->width() - 40.0f) < 1.0f);
        assert(std::abs(bounds->height() - 40.0f) < 1.0f);
    }

    // 3. TrimPathShape modifier
    {
        TrimPathShape trim;
        trim.start = FloatProperty(0.0f);
        trim.end = FloatProperty(50.0f); // Trim to first half

        PathBuilder b;
        b.move_to(0.0f, 0.0f);
        b.line_to(100.0f, 0.0f);
        auto opt_p = b.finish();
        assert(opt_p.has_value());

        Path trimmed = trim.apply_trim(*opt_p, 0.0f);
        assert(!trimmed.is_empty());
        auto b_trim = trimmed.compute_tight_bounds();
        assert(b_trim.has_value());
        assert(std::abs(b_trim->width() - 50.0f) < 1.0f);
    }

    std::cout << "[PASS] Lottie Shapes & Geometry tests passed.\n";
}

void test_lottie_animation_and_player() {
    std::cout << "[TEST] Running Lottie Animation Document & Player tests...\n";

    // Minimal valid Lottie animation JSON (a pulsing blue circle)
    std::string_view lottie_json = R"({
        "v": "5.7.4",
        "fr": 30,
        "ip": 0,
        "op": 60,
        "w": 200,
        "h": 200,
        "nm": "PulseCircle",
        "ddd": 0,
        "assets": [],
        "layers": [
            {
                "ddd": 0,
                "ind": 1,
                "ty": 4,
                "nm": "CircleLayer",
                "sr": 1,
                "ks": {
                    "o": {"a": 0, "k": 100},
                    "r": {"a": 0, "k": 0},
                    "p": {"a": 0, "k": [100, 100, 0]},
                    "a": {"a": 0, "k": [0, 0, 0]},
                    "s": {
                        "a": 1,
                        "k": [
                            {
                                "t": 0,
                                "s": [50, 50, 100],
                                "e": [150, 150, 100],
                                "o": {"x": 0.4, "y": 0.0},
                                "i": {"x": 0.6, "y": 1.0}
                            },
                            {
                                "t": 60,
                                "s": [150, 150, 100],
                                "e": [50, 50, 100]
                            }
                        ]
                    }
                },
                "shapes": [
                    {
                        "ty": "gr",
                        "nm": "Group 1",
                        "it": [
                            {
                                "ty": "el",
                                "nm": "Ellipse 1",
                                "p": {"a": 0, "k": [0, 0]},
                                "s": {"a": 0, "k": [60, 60]}
                            },
                            {
                                "ty": "fl",
                                "nm": "Fill 1",
                                "c": {"a": 0, "k": [0.1, 0.5, 0.9, 1.0]},
                                "o": {"a": 0, "k": 100},
                                "r": 1
                            },
                            {
                                "ty": "tr",
                                "p": {"a": 0, "k": [0, 0]},
                                "a": {"a": 0, "k": [0, 0]},
                                "s": {"a": 0, "k": [100, 100]},
                                "r": {"a": 0, "k": 0},
                                "o": {"a": 0, "k": 100}
                            }
                        ]
                    }
                ],
                "ip": 0,
                "op": 60,
                "st": 0
            }
        ]
    })";

    auto anim = Animation::load_from_data(lottie_json);
    assert(anim != nullptr);
    assert(anim->width() == 200.0f);
    assert(anim->height() == 200.0f);
    assert(anim->in_point() == 0.0f);
    assert(anim->out_point() == 60.0f);
    assert(anim->frame_rate() == 30.0f);
    assert(anim->total_frames() == 60.0f);
    assert(std::abs(anim->duration_seconds() - 2.0f) < 1e-4f);
    assert(anim->layers().size() == 1);

    // Test rendering frame to Canvas
    auto pixmap = Pixmap::create(200, 200);
    assert(pixmap.has_value());
    pixmap->fill(Color::WHITE);
    Canvas canvas(*pixmap);

    anim->render(canvas, 0.0f);  // Start frame
    anim->render(canvas, 30.0f); // Mid frame
    anim->render(canvas, 60.0f); // End frame

    // Test Player playback controller
    Player player(anim);
    assert(player.is_playing());
    assert(player.is_looping());
    assert(player.current_frame() == 0.0f);
    assert(player.progress() == 0.0f);

    // Advance by 1 second (30 frames at 30fps)
    player.advance(1.0f);
    assert(std::abs(player.current_frame() - 30.0f) < 1e-2f);
    assert(std::abs(player.progress() - 0.5f) < 1e-2f);

    // Seek to 75%
    player.seek_progress(0.75f);
    assert(std::abs(player.progress() - 0.75f) < 1e-2f);
    assert(std::abs(player.current_frame() - 45.0f) < 1e-2f);

    // Advance beyond end with looping enabled
    player.advance(2.0f); // 60 frames -> loops back around
    assert(player.is_playing());
    assert(player.current_frame() >= 0.0f && player.current_frame() <= 60.0f);

    // Test fitted render
    auto dest = Rect::from_xywh(10, 10, 100, 100);
    assert(dest.has_value());
    player.render(canvas, *dest, true);

    std::cout << "[PASS] Lottie Animation Document & Player tests passed.\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "   Nisaba Sovereign Lottie & JSON Test Suite\n";
    std::cout << "========================================================\n";

    test_json_parser();
    test_lottie_properties_and_easing();
    test_lottie_shapes();
    test_lottie_animation_and_player();

    std::cout << "========================================================\n";
    std::cout << "   ALL LOTTIE & JSON TESTS PASSED SUCCESSFULLY! (100%)\n";
    std::cout << "========================================================\n";
    return 0;
}
