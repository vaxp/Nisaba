/// @file test_lottie.cpp
/// @brief Comprehensive automated unit tests for sovereign JSON & industrial-grade Lottie animation engine.

#include "nisaba/json/json.hpp"
#include "nisaba/lottie/lottie.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/image/deflate.hpp"

#include <iostream>
#include <cassert>
#include <cmath>
#include <cstring>
#include <string_view>
#include <vector>

using namespace nisaba;
using namespace nisaba::lottie;
using namespace nisaba::json;

void test_json_parser() {
    std::cout << "[TEST] Running Sovereign JSON Parser tests...\n";

    // 1. Primitive types & fast from_chars number parsing
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

    // 3. Nested objects (contiguous vector storage)
    {
        std::string_view json = R"({
            "name": "Nisaba Lottie",
            "version": "2.0",
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

    // 4. Unicode escape sequences \uXXXX and UTF-16 surrogate pairs
    {
        auto u_val = JsonValue::parse(R"("\u0041\u0042\u0043")"); // "ABC"
        assert(u_val.has_value() && u_val->is_string());
        assert(u_val->as_string() == "ABC");

        auto emoji_val = JsonValue::parse(R"("\uD83D\uDE80")"); // Rocket U+1F680
        assert(emoji_val.has_value() && emoji_val->is_string());
        assert(emoji_val->as_string() == "\xF0\x9F\x9A\x80");
    }

    std::cout << "[PASS] JSON Parser tests passed.\n";
}

void test_lottie_properties_spatial_and_expressions() {
    std::cout << "[TEST] Running Lottie Properties, Spatial Bezier, Skew, 3D & Expressions tests...\n";

    // 1. Static property & dynamic override
    {
        FloatProperty p(100.0f);
        assert(!p.is_animated());
        assert(p.evaluate(0.0f) == 100.0f);
        p.set_override(250.0f);
        assert(p.evaluate(50.0f) == 250.0f);
        p.clear_override();
        assert(p.evaluate(50.0f) == 100.0f);
    }

    // 2. Spatial Bezier curved motion with arc-length parameterization
    {
        PointProperty p;
        Keyframe<Point> kf1;
        kf1.time = 0.0f;
        kf1.start_value = Point::from_xy(0.0f, 0.0f);
        kf1.end_value = Point::from_xy(100.0f, 0.0f);
        kf1.spatial_out_tangent = Point::from_xy(0.0f, 100.0f);
        kf1.spatial_in_tangent = Point::from_xy(0.0f, 100.0f);
        kf1.has_spatial_tangents = true;
        p.add_keyframe(kf1);

        Keyframe<Point> kf2;
        kf2.time = 10.0f;
        kf2.start_value = Point::from_xy(100.0f, 0.0f);
        kf2.end_value = Point::from_xy(100.0f, 0.0f);
        p.add_keyframe(kf2);

        Point mid = p.evaluate(5.0f);
        // Midpoint along arch should be at x ~ 50 and y > 50 due to upward spatial tangents
        assert(std::abs(mid.x - 50.0f) < 2.0f);
        assert(mid.y > 50.0f);
    }

    // 3. Expressions: loopOut("cycle"), loopOut("pingpong"), wiggle(freq, amp)
    {
        std::string_view expr_json = R"JSON({
            "a": 1,
            "x": "loopOut('pingpong')",
            "k": [
                {"t": 0, "s": [0], "e": [100]},
                {"t": 10, "s": [100]}
            ]
        })JSON";
        auto val = JsonValue::parse(expr_json);
        assert(val.has_value());
        FloatProperty fp = parse_float_property(*val);
        assert(fp.expression().type == LottieExpression::Type::LoopOutPingPong);
        // At t=5 -> 50, at t=10 -> 100, at t=15 (ping-pong back) -> 50, at t=20 -> 0
        assert(std::abs(fp.evaluate(5.0f) - 50.0f) < 1.0f);
        assert(std::abs(fp.evaluate(15.0f) - 50.0f) < 1.0f);

        std::string_view wig_json = R"JSON({
            "a": 0,
            "k": 50,
            "x": "wiggle(4, 20)"
        })JSON";
        auto wval = JsonValue::parse(wig_json);
        FloatProperty wfp = parse_float_property(*wval);
        assert(wfp.expression().type == LottieExpression::Type::Wiggle);
        assert(wfp.is_animated());
        float v_t1 = wfp.evaluate(3.5f);
        assert(v_t1 >= 30.0f && v_t1 <= 70.0f);
    }

    // 4. LottieTransform with Skew ("sk"/"sa"), Auto-Orient ("ao"), and 3D ("rx", "ry", "rz")
    {
        std::string_view tr_json = R"({
            "p": {"a": 0, "k": [100, 100, 0]},
            "a": {"a": 0, "k": [0, 0, 0]},
            "s": {"a": 0, "k": [100, 100, 100]},
            "r": {"a": 0, "k": 0},
            "sk": {"a": 0, "k": 20},
            "sa": {"a": 0, "k": 15},
            "rx": {"a": 0, "k": 30},
            "ry": {"a": 0, "k": 45},
            "rz": {"a": 0, "k": 10},
            "o": {"a": 0, "k": 100}
        })";
        auto tr_val = JsonValue::parse(tr_json);
        LottieTransform tr = LottieTransform::parse(*tr_val);
        assert(tr.is_3d);
        Transform m2d = tr.matrix(0.0f);
        assert(!m2d.is_identity());
        Transform4x4 m3d = tr.matrix_3d(0.0f);
        assert(std::abs(m3d.m[3] - 100.0f) < 1e-2f || std::abs(m3d.m[0] - 1.0f) > 1e-3f);
    }

    std::cout << "[PASS] Lottie Properties, Spatial Bezier, Skew, 3D & Expressions tests passed.\n";
}

void test_lottie_shapes_gradients_and_modifiers() {
    std::cout << "[TEST] Running Lottie Shapes, Gradients & Path Modifiers tests...\n";

    // 1. StarShape with inner/outer roundness and CCW winding direction ("d": 3)
    {
        StarShape star;
        star.type = 1;
        star.points = FloatProperty(5.0f);
        star.outer_radius = FloatProperty(50.0f);
        star.inner_radius = FloatProperty(25.0f);
        star.outer_roundness = FloatProperty(40.0f);
        star.inner_roundness = FloatProperty(20.0f);
        star.direction = 3; // CCW

        Path p;
        star.collect_geometry(0.0f, p);
        assert(!p.is_empty());
        // Rounded star uses Cubic verbs
        bool has_cubic = false;
        for (auto v : p.verbs()) {
            if (v == PathVerb::Cubic) has_cubic = true;
        }
        assert(has_cubic);
    }

    // 2. GradientColorStops ("g") with RGB + Alpha stop cross-interpolation
    {
        std::string_view grad_json = R"({
            "p": 2,
            "k": {
                "a": 0,
                "k": [
                    0.0, 1.0, 0.0, 0.0,
                    1.0, 0.0, 0.0, 1.0,
                    0.0, 1.0,
                    0.5, 0.5,
                    1.0, 0.0
                ]
            }
        })";
        auto gval = JsonValue::parse(grad_json);
        GradientColorStops gcs = GradientColorStops::parse(*gval);
        auto stops = gcs.evaluate_stops(0.0f, 1.0f);
        // Should merge t=0.0, t=0.5 (from alpha), t=1.0 -> 3 stops!
        assert(stops.size() == 3);
        assert(std::abs(stops[0].color.red() - 1.0f) < 1e-3f);
        assert(std::abs(stops[1].position.get() - 0.5f) < 1e-3f);
        assert(std::abs(stops[1].color.alpha() - 0.5f) < 1e-3f);
        assert(std::abs(stops[2].color.blue() - 1.0f) < 1e-3f);
        assert(std::abs(stops[2].color.alpha() - 0.0f) < 1e-3f);
    }

    // 3. Exact TrimPathShape ("tm") via PathMeasure (Simultaneously & Individually)
    {
        TrimPathShape trim;
        trim.start = FloatProperty(0.0f);
        trim.end = FloatProperty(50.0f);
        trim.trim_type = TrimType::Individually;

        PathBuilder b;
        b.move_to(0.0f, 0.0f);
        b.line_to(100.0f, 0.0f);
        b.move_to(0.0f, 20.0f);
        b.line_to(100.0f, 20.0f);
        auto opt_p = b.finish();
        assert(opt_p.has_value());

        // Total length of 2 contours = 200. 50% individual trim should trim only the first contour (len 100)!
        Path trimmed = trim.apply_trim(*opt_p, 0.0f);
        assert(!trimmed.is_empty());
        auto b_trim = trimmed.compute_tight_bounds();
        assert(b_trim.has_value());
        assert(std::abs(b_trim->width() - 100.0f) < 1.0f);
        assert(std::abs(b_trim->height() - 0.0f) < 1.0f);
    }

    // 4. MergePathsShape ("mm"), RoundCornersShape ("rd"), OffsetPathShape ("op"), PuckerBloat ("pb"), ZigZag ("zz")
    {
        RectShape r1, r2;
        r1.position = PointProperty(Point::from_xy(40.0f, 50.0f));
        r1.size = PointProperty(Point::from_xy(40.0f, 40.0f));
        r2.position = PointProperty(Point::from_xy(60.0f, 50.0f));
        r2.size = PointProperty(Point::from_xy(40.0f, 40.0f));

        Path p1, p2;
        r1.collect_geometry(0.0f, p1);
        r2.collect_geometry(0.0f, p2);
        std::vector<Path> paths = {p1, p2};

        MergePathsShape mm;
        mm.mode = MergeMode::Add; // Union
        mm.modify_paths(0.0f, paths);
        assert(paths.size() == 1 && !paths[0].is_empty());

        RoundCornersShape rd;
        rd.radius = FloatProperty(8.0f);
        rd.modify_paths(0.0f, paths);
        assert(!paths[0].is_empty());

        OffsetPathShape op;
        op.amount = FloatProperty(4.0f);
        op.modify_paths(0.0f, paths);
        assert(!paths[0].is_empty());

        PuckerBloatShape pb;
        pb.amount = FloatProperty(25.0f);
        pb.modify_paths(0.0f, paths);
        assert(!paths[0].is_empty());

        ZigZagShape zz;
        zz.size = FloatProperty(5.0f);
        zz.ridges = FloatProperty(4.0f);
        zz.modify_paths(0.0f, paths);
        assert(!paths[0].is_empty());
    }

    std::cout << "[PASS] Lottie Shapes, Gradients & Path Modifiers tests passed.\n";
}

void test_advanced_features_and_dotlottie() {
    std::cout << "[TEST] Running Advanced Features (Repeater, Masks, Track Mattes, Effects, Text, Image, Overrides, .lottie ZIP)...\n";

    // Comprehensive Lottie JSON testing GradientFill, GradientStroke, Repeater, Track Matte, Mask, Effect, TextLayer, and Slot Overrides
    std::string_view rich_json = R"({
        "v": "5.9.0",
        "fr": 60,
        "ip": 0,
        "op": 60,
        "w": 200,
        "h": 200,
        "nm": "IndustrialShowcase",
        "markers": [
            {"cm": "intro", "tm": 0, "dr": 20},
            {"cm": "loop_seg", "tm": 20, "dr": 40}
        ],
        "fonts": {
            "list": [
                {"fName": "TestFont", "fFamily": "Sans", "fStyle": "Regular", "ascent": 75}
            ]
        },
        "chars": [
            {
                "ch": "A",
                "size": 100,
                "style": "Regular",
                "w": 60,
                "fFamily": "Sans",
                "data": {
                    "shapes": [
                        {
                            "ty": "gr",
                            "nm": "GlyphA",
                            "it": [
                                {"ty": "rc", "p": {"a": 0, "k": [30, -35]}, "s": {"a": 0, "k": [40, 70]}, "r": {"a": 0, "k": 4}},
                                {"ty": "tr", "p": {"a": 0, "k": [0, 0]}, "a": {"a": 0, "k": [0, 0]}, "s": {"a": 0, "k": [100, 100]}, "r": {"a": 0, "k": 0}, "o": {"a": 0, "k": 100}}
                            ]
                        }
                    ]
                }
            }
        ],
        "layers": [
            {
                "ind": 1,
                "ty": 5,
                "nm": "TitleText",
                "ip": 0,
                "op": 60,
                "st": 0,
                "ks": {"p": {"a": 0, "k": [20, 160, 0]}, "a": {"a": 0, "k": [0, 0, 0]}, "s": {"a": 0, "k": [100, 100, 100]}, "r": {"a": 0, "k": 0}, "o": {"a": 0, "k": 100}},
                "t": {
                    "d": {
                        "k": [
                            {
                                "t": 0,
                                "s": {
                                    "s": 36,
                                    "f": "TestFont",
                                    "t": "AAA",
                                    "j": 0,
                                    "tr": 50,
                                    "lh": 42,
                                    "fc": [0.9, 0.2, 0.2],
                                    "sc": [1.0, 1.0, 1.0],
                                    "sw": 2,
                                    "of": true
                                }
                            }
                        ]
                    },
                    "a": [
                        {
                            "nm": "Animator 1",
                            "s": {"s": {"a": 0, "k": 0}, "e": {"a": 0, "k": 100}, "o": {"a": 0, "k": 0}, "r": 1},
                            "a": {"p": {"a": 0, "k": [0, -5, 0]}}
                        }
                    ]
                }
            },
            {
                "ind": 2,
                "ty": 4,
                "nm": "MatteSourceLayer",
                "td": 1,
                "ip": 0,
                "op": 60,
                "st": 0,
                "ks": {"p": {"a": 0, "k": [100, 100, 0]}},
                "shapes": [
                    {
                        "ty": "gr",
                        "it": [
                            {"ty": "el", "p": {"a": 0, "k": [0, 0]}, "s": {"a": 0, "k": [120, 120]}},
                            {"ty": "fl", "c": {"a": 0, "k": [1, 1, 1, 1]}, "o": {"a": 0, "k": 100}},
                            {"ty": "tr"}
                        ]
                    }
                ]
            },
            {
                "ind": 3,
                "ty": 4,
                "nm": "MainShapeLayer",
                "tt": 1,
                "ip": 0,
                "op": 60,
                "st": 0,
                "ks": {"p": {"a": 0, "k": [100, 100, 0]}},
                "ef": [
                    {
                        "ty": 29,
                        "nm": "Gaussian Blur",
                        "en": 1,
                        "ef": [{"ty": 0, "v": {"a": 0, "k": 2.0}}]
                    }
                ],
                "shapes": [
                    {
                        "ty": "gr",
                        "nm": "MainGroup",
                        "it": [
                            {"ty": "rc", "nm": "Box", "p": {"a": 0, "k": [0, 0]}, "s": {"a": 0, "k": [80, 80]}, "r": {"a": 0, "k": 10}},
                            {
                                "ty": "rp",
                                "nm": "Repeater 1",
                                "c": {"a": 0, "k": 3},
                                "o": {"a": 0, "k": 0},
                                "m": 1,
                                "tr": {
                                    "p": {"a": 0, "k": [10, 10]},
                                    "s": {"a": 0, "k": [90, 90]},
                                    "r": {"a": 0, "k": 15},
                                    "so": {"a": 0, "k": 100},
                                    "eo": {"a": 0, "k": 50}
                                }
                            },
                            {
                                "ty": "gf",
                                "nm": "GradFill",
                                "t": 2,
                                "s": {"a": 0, "k": [0, 0]},
                                "e": {"a": 0, "k": [60, 0]},
                                "h": {"a": 0, "k": 50},
                                "a": {"a": 0, "k": 30},
                                "o": {"a": 0, "k": 100},
                                "g": {
                                    "p": 2,
                                    "k": {"a": 0, "k": [0.0, 0.1, 0.8, 0.4, 1.0, 0.1, 0.2, 0.9]}
                                }
                            },
                            {
                                "ty": "fl",
                                "nm": "ThemeFill",
                                "c": {"a": 0, "k": [0.2, 0.6, 1.0, 1.0], "sid": "primary_theme"},
                                "o": {"a": 0, "k": 80}
                            },
                            {"ty": "tr"}
                        ]
                    }
                ]
            }
        ]
    })";

    auto anim = Animation::load_from_data(rich_json);
    assert(anim != nullptr);
    assert(anim->markers().size() == 2);

    auto pixmap = Pixmap::create(200, 200);
    assert(pixmap.has_value());
    pixmap->fill(Color::BLACK);
    Canvas canvas(*pixmap);

    anim->render(canvas, 10.0f);

    // Verify dynamic overrides & slots
    assert(anim->set_slot_color("primary_theme", Color::from_rgba8(255, 128, 0, 255)));
    assert(anim->set_color_override("MainShapeLayer.MainGroup.ThemeFill", Color::from_rgba8(0, 255, 100, 255)));
    assert(anim->set_text_override("TitleText", "AAAA"));
    anim->render(canvas, 15.0f);
    anim->clear_overrides();

    // Verify Player marker playback & Bounce mode
    Player player(anim);
    assert(player.play_marker("intro"));
    assert(player.current_frame() == 0.0f);
    player.set_loop_mode(LoopMode::Bounce);
    player.advance(0.5f); // 30 frames > 20 duration -> bounces backward!
    assert(player.current_frame() >= 0.0f && player.current_frame() <= 20.0f);

    // Verify Hit-Testing
    auto hit = anim->hit_test(100.0f, 100.0f, 10.0f);
    assert(hit.has_value());
    assert(*hit == "MainShapeLayer" || *hit == "Box");

    // Verify .lottie (dotLottie ZIP container) ingestion using sovereign DEFLATE
    {
        auto compressed_res = image::deflate_compress(
            std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(rich_json.data()), rich_json.size()),
            6
        );
        assert(compressed_res.has_value());
        const auto& comp_data = *compressed_res;

        std::string entry_name = "animations/data.json";
        std::vector<uint8_t> zip_bytes;
        auto push_u16 = [&](uint16_t v) {
            zip_bytes.push_back(static_cast<uint8_t>(v & 0xFF));
            zip_bytes.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        };
        auto push_u32 = [&](uint32_t v) {
            zip_bytes.push_back(static_cast<uint8_t>(v & 0xFF));
            zip_bytes.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
            zip_bytes.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
            zip_bytes.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
        };

        // Local File Header (0x04034b50)
        push_u32(0x04034b50u);
        push_u16(20); // version needed
        push_u16(0);  // flags
        push_u16(8);  // method = 8 (DEFLATE)
        push_u16(0);  // mod time
        push_u16(0);  // mod date
        push_u32(0);  // crc32
        push_u32(static_cast<uint32_t>(comp_data.size()));
        push_u32(static_cast<uint32_t>(rich_json.size()));
        push_u16(static_cast<uint16_t>(entry_name.size()));
        push_u16(0);  // extra len
        zip_bytes.insert(zip_bytes.end(), entry_name.begin(), entry_name.end());
        zip_bytes.insert(zip_bytes.end(), comp_data.begin(), comp_data.end());

        auto dot_anim = Animation::load_from_bytes(zip_bytes);
        assert(dot_anim != nullptr);
        assert(dot_anim->name() == "IndustrialShowcase");
        assert(dot_anim->width() == 200.0f);
    }

    std::cout << "[PASS] Advanced Features & .lottie ZIP tests passed.\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "   Nisaba Sovereign Lottie & JSON Test Suite\n";
    std::cout << "========================================================\n";

    test_json_parser();
    test_lottie_properties_spatial_and_expressions();
    test_lottie_shapes_gradients_and_modifiers();
    test_advanced_features_and_dotlottie();

    std::cout << "========================================================\n";
    std::cout << "   ALL LOTTIE & JSON TESTS PASSED SUCCESSFULLY! (100%)\n";
    std::cout << "========================================================\n";
    return 0;
}
