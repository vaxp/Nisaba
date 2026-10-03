#include <iostream>
#include <cassert>
#include <cmath>
#include <chrono>
#include <string>

#include "nisaba/animation/animation.hpp"

using namespace nisaba;
using namespace nisaba::animation;

static void test_curves() {
    std::cout << "[*] Running Animation Curves Test...\n";

    // Linear
    assert(std::abs(Curves::linear.evaluate(0.0) - 0.0) < 1e-6);
    assert(std::abs(Curves::linear.evaluate(0.5) - 0.5) < 1e-6);
    assert(std::abs(Curves::linear.evaluate(1.0) - 1.0) < 1e-6);

    // EaseIn vs EaseOut
    assert(Curves::easeIn.evaluate(0.5) < 0.5);
    assert(Curves::easeOut.evaluate(0.5) > 0.5);

    // EaseInOut symmetry
    assert(std::abs(Curves::easeInOut.evaluate(0.5) - 0.5) < 1e-5);
    assert(Curves::easeInOut.evaluate(0.2) < 0.2);
    assert(Curves::easeInOut.evaluate(0.8) > 0.8);

    // Bounce and BackOut
    assert(std::abs(Curves::bounceOut.evaluate(1.0) - 1.0) < 1e-4);
    assert(Curves::backOut.evaluate(0.8) > 0.8);

    // CubicBezier
    CubicBezierCurve bezier(0.25, 0.1, 0.25, 1.0);
    assert(std::abs(bezier.evaluate(0.0) - 0.0) < 1e-5);
    assert(std::abs(bezier.evaluate(1.0) - 1.0) < 1e-5);

    std::cout << "  [+] Curves tests passed!\n";
}

static void test_tweens() {
    std::cout << "[*] Running Tweens Test...\n";

    // Float Tween
    Tween<float> float_tween(10.0f, 20.0f);
    assert(std::abs(float_tween.evaluate(0.0) - 10.0f) < 1e-5);
    assert(std::abs(float_tween.evaluate(0.5) - 15.0f) < 1e-5);
    assert(std::abs(float_tween.evaluate(1.0) - 20.0f) < 1e-5);

    // Color Tween
    Color c1 = Color::from_rgba8(100, 50, 200, 255);
    Color c2 = Color::from_rgba8(200, 150, 100, 255);
    Tween<Color> color_tween(c1, c2);
    Color mid_color = color_tween.evaluate(0.5);
    assert(std::abs(mid_color.red() - 150.0f / 255.0f) < 0.02f);
    assert(std::abs(mid_color.green() - 100.0f / 255.0f) < 0.02f);

    // Point Tween
    Point p1(10.0f, 20.0f);
    Point p2(30.0f, 60.0f);
    Tween<Point> pt_tween(p1, p2);
    Point mid_pt = pt_tween.evaluate(0.5);
    assert(std::abs(mid_pt.x - 20.0f) < 1e-5);
    assert(std::abs(mid_pt.y - 40.0f) < 1e-5);

    // Rect Tween
    auto r1 = Rect::from_xywh(0.0f, 0.0f, 100.0f, 100.0f).value();
    auto r2 = Rect::from_xywh(50.0f, 50.0f, 200.0f, 300.0f).value();
    Tween<Rect> rect_tween(r1, r2);
    Rect mid_rect = rect_tween.evaluate(0.5);
    assert(std::abs(mid_rect.x() - 25.0f) < 1e-5);
    assert(std::abs(mid_rect.width() - 150.0f) < 1e-5);

    std::cout << "  [+] Tweens tests passed!\n";
}

static void test_animation_controller() {
    std::cout << "[*] Running AnimationController Test...\n";

    AnimationController controller(std::chrono::milliseconds{200});
    assert(controller.is_dismissed());
    assert(controller.value() == 0.0f);

    bool listener_called = false;
    controller.add_listener([&] { listener_called = true; });

    controller.forward();
    assert(controller.status() == AnimationStatus::Forward);
    assert(controller.is_animating());

    // Advance 100ms
    controller.tick_delta(std::chrono::milliseconds{100});
    assert(listener_called);
    assert(controller.value() > 0.35f && controller.value() < 0.65f);

    // Advance remaining 120ms to complete
    controller.tick_delta(std::chrono::milliseconds{120});
    assert(controller.is_completed());
    assert(controller.value() == 1.0f);
    assert(!controller.is_animating());

    // Test reverse
    controller.reverse();
    assert(controller.status() == AnimationStatus::Reverse);
    controller.tick_delta(std::chrono::milliseconds{220});
    assert(controller.is_dismissed());
    assert(controller.value() == 0.0f);

    // Test AnimatedValue
    AnimatedValue<float> anim_val(50.0f, 150.0f, std::chrono::milliseconds{100});
    anim_val.forward();
    anim_val.controller().tick_delta(std::chrono::milliseconds{50});
    assert(anim_val.get() > 90.0f && anim_val.get() < 110.0f);

    std::cout << "  [+] AnimationController tests passed!\n";
}

static void test_spring_simulation() {
    std::cout << "[*] Running Spring Simulation Test...\n";

    // Bouncy underdamped
    SpringSimulation spring(Springs::bouncy, 0.0f, 100.0f);
    assert(spring.x(0.0f) == 0.0f);
    assert(spring.estimated_duration() > 0.2f);

    // After settling time, should reach target
    float settle_time = spring.estimated_duration() + 0.5f;
    float final_pos = spring.x(settle_time);
    assert(std::abs(final_pos - 100.0f) < 0.1f);
    assert(spring.is_done(settle_time));

    // Critically damped (smooth)
    SpringSimulation smooth(Springs::smooth, 0.0f, 50.0f);
    float smooth_final = smooth.x(1.0f);
    assert(std::abs(smooth_final - 50.0f) < 0.1f);

    std::cout << "  [+] Spring simulation tests passed!\n";
}

static void test_spring_controller() {
    std::cout << "[*] Running SpringController Test...\n";

    SpringController controller(Springs::smooth, 0.0f);
    assert(controller.value() == 0.0f);

    controller.animate_to(200.0f);
    assert(controller.is_animating());
    assert(controller.target() == 200.0f);

    // Step physics forward by 100ms
    controller.tick_delta(std::chrono::milliseconds{100});
    assert(controller.value() > 0.0f);

    // Step by 1.5s to settle
    controller.tick_delta(std::chrono::milliseconds{1500});
    assert(std::abs(controller.value() - 200.0f) < 0.05f);
    assert(!controller.is_animating());

    std::cout << "  [+] SpringController tests passed!\n";
}

static void test_timeline() {
    std::cout << "[*] Running AnimationTimeline Test...\n";

    AnimationTimeline timeline(std::chrono::milliseconds{1000});

    float track1_val = 0.0f;
    float track2_val = 0.0f;

    timeline.add(std::chrono::milliseconds{0}, std::chrono::milliseconds{500}, [&](float p) {
        track1_val = p;
    });

    timeline.add(std::chrono::milliseconds{500}, std::chrono::milliseconds{500}, [&](float p) {
        track2_val = p;
    });

    // Seek to 250ms (25% progress)
    timeline.seek(0.25f);
    assert(std::abs(track1_val - 0.5f) < 1e-3);
    assert(track2_val == 0.0f);

    // Seek to 750ms (75% progress)
    timeline.seek(0.75f);
    assert(track1_val == 1.0f);
    assert(std::abs(track2_val - 0.5f) < 1e-3);

    std::cout << "  [+] AnimationTimeline tests passed!\n";
}

static void test_stagger() {
    std::cout << "[*] Running StaggerHelper Test...\n";

    StaggerConfig cfg;
    cfg.item_duration = std::chrono::milliseconds{200};
    cfg.delay_between_items = std::chrono::milliseconds{50};

    // 4 items
    auto total = StaggerHelper::total_duration(4, cfg);
    // 3 * 50 + 200 = 350ms
    assert(total.count() == 350);

    auto [b0, e0] = StaggerHelper::item_interval(0, 4, cfg);
    assert(b0 == 0.0f);
    assert(std::abs(e0 - 200.0f / 350.0f) < 1e-4);

    // Item progress at master = 0.0 should be 0.0
    assert(StaggerHelper::item_progress(0, 4, 0.0f, cfg) == 0.0f);
    // Item progress at master = 1.0 should be 1.0
    assert(StaggerHelper::item_progress(3, 4, 1.0f, cfg) == 1.0f);

    std::cout << "  [+] StaggerHelper tests passed!\n";
}

static void test_particle_system() {
    std::cout << "[*] Running ParticleSystem Test...\n";

    ParticleSystem ps(ParticlePresets::confetti());
    assert(!ps.has_active_particles());

    ps.burst(Point(100.0f, 100.0f));
    assert(ps.has_active_particles());
    assert(ps.active_count() > 50);

    // Simulate 0.5s of physics
    ps.update(0.5f);
    assert(ps.has_active_particles());

    // Clear
    ps.clear();
    assert(!ps.has_active_particles());
    assert(ps.active_count() == 0);

    std::cout << "  [+] ParticleSystem tests passed!\n";
}

int main() {
    std::cout << "\n========================================================\n";
    std::cout << "       NISABA SOVEREIGN ANIMATION SUBSYSTEM TESTS\n";
    std::cout << "========================================================\n";

    test_curves();
    test_tweens();
    test_animation_controller();
    test_spring_simulation();
    test_spring_controller();
    test_timeline();
    test_stagger();
    test_particle_system();

    std::cout << "\n[SUCCESS] All Nisaba animation tests passed flawlessly!\n";
    return 0;
}
