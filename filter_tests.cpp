#include <cassert>
#include <cmath>
#include <cstdio>
#include "filter_core.h"

using namespace allydrive;

static float run(float raw, float speed_kph, float seconds, FilterState& state,
                 float yaw = 0.0f, float lateral = 0.0f, float hz = 240.0f) {
    TelemetrySample t{};
    t.valid = true;
    t.speed_mps = speed_kph / 3.6f;
    t.forward_velocity_mps = std::max(6.0f, t.speed_mps);
    t.lateral_velocity_mps = lateral;
    t.yaw_rate_rad_s = yaw;
    FilterConfig cfg{};
    float out = 0.0f;
    const int steps = static_cast<int>(seconds * hz);
    for (int i = 0; i < steps; ++i) out = step_filter(raw, t, 1.0f / hz, state, cfg);
    return out;
}

int main() {
    FilterConfig cfg{};
    assert(remap_deadzone(0.02f, cfg.inner_deadzone) == 0.0f);
    assert(steering_cap(5.0f, cfg) > 0.99f);
    assert(steering_cap(50.0f, cfg) > 0.62f && steering_cap(50.0f, cfg) < 0.65f);
    assert(steering_cap(100.0f, cfg) > 0.43f && steering_cap(100.0f, cfg) < 0.45f);
    assert(steering_cap(150.0f, cfg) > 0.35f && steering_cap(150.0f, cfg) < 0.36f);
    assert(steering_cap(200.0f, cfg) > 0.32f && steering_cap(200.0f, cfg) < 0.33f);
    assert(steering_cap(260.0f, cfg) < 0.31f);

    FilterState slow{};
    FilterState fast{};
    const float slow_out = run(1.0f, 30.0f, 0.50f, slow);
    const float fast_out = run(1.0f, 240.0f, 0.50f, fast);
    assert(slow_out > 0.73f);
    assert(fast_out < 0.34f);
    assert(fast_out > 0.20f);

    FilterState recenter{};
    run(1.0f, 160.0f, 0.40f, recenter);
    const float before = recenter.output;
    const float after = run(0.0f, 160.0f, 0.20f, recenter);
    assert(std::fabs(after) < std::fabs(before));

    FilterState destabilizing{};
    FilterState stable{};
    const float same_yaw = run(1.0f, 150.0f, 0.50f, destabilizing, 0.9f, 4.0f);
    const float normal = run(1.0f, 150.0f, 0.50f, stable, 0.0f, 0.0f);
    assert(same_yaw < normal);

    FilterState counter{};
    const float counter_out = run(-1.0f, 150.0f, 0.15f, counter, 0.9f, 4.0f);
    assert(counter_out < -0.08f);
    assert(counter_out > -0.24f);

    // Crossing from cornering lock into opposite lock is deliberately slower
    // than applying countersteer after the axis is already near centre.
    FilterState crossing{};
    run(1.0f, 150.0f, 0.20f, crossing, 0.0f, 0.0f);
    const float crossed = run(-1.0f, 150.0f, 0.10f, crossing, 0.9f, 4.0f);
    FilterState centred{};
    const float from_centre = run(-1.0f, 150.0f, 0.10f, centred, 0.9f, 4.0f);
    assert(crossed > from_centre);

    // A mild slide receives less opposite lock than a severe one.
    FilterState mild{};
    FilterState severe{};
    const float mild_out = run(-1.0f, 120.0f, 0.30f, mild, 0.58f, 1.8f);
    const float severe_out = run(-1.0f, 120.0f, 0.30f, severe, 1.5f, 6.0f);
    assert(std::fabs(mild_out) < std::fabs(severe_out));

    // A normal high-yaw hairpin at low speed must not be mistaken for a slide.
    FilterState slow_turn{};
    FilterState slow_reference{};
    const float slow_assisted = run(0.70f, 40.0f, 0.35f, slow_turn, 0.85f, 0.2f);
    const float slow_plain = run(0.70f, 40.0f, 0.35f, slow_reference, 0.0f, 0.0f);
    assert(std::fabs(slow_assisted - slow_plain) < 0.001f);

    // Time-based integration must feel effectively the same regardless of how
    // often the game polls XInput.
    FilterState at60{};
    FilterState at240{};
    const float out60 = run(0.72f, 130.0f, 0.75f, at60, 0.0f, 0.0f, 60.0f);
    const float out240 = run(0.72f, 130.0f, 0.75f, at240, 0.0f, 0.0f, 240.0f);
    assert(std::fabs(out60 - out240) < 0.012f);

    std::puts("AllyDrive filter tests passed");
    return 0;
}
