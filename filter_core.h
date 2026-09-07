#pragma once

#include <cmath>
#include <algorithm>

namespace allydrive {

struct TelemetrySample {
    bool valid = false;
    float speed_mps = 0.0f;
    float lateral_velocity_mps = 0.0f;
    float forward_velocity_mps = 0.0f;
    float yaw_rate_rad_s = 0.0f;
    float longitudinal_accel_mps2 = 0.0f;
};

struct FilterConfig {
    float inner_deadzone = 0.035f;
    float curve_exponent = 1.85f;
    float low_speed_kph = 10.0f;
    float speed_curve_kph = 55.0f;
    float high_speed_lock = 0.30f;
    float low_speed_turn_rate = 4.6f;
    float high_speed_turn_rate = 1.45f;
    float recenter_rate = 4.2f;
    float countersteer_rate = 3.2f;
    float countersteer_cross_rate = 2.0f;
    float countersteer_min_lock = 0.08f;
    float countersteer_yaw_gain = 0.16f;
    float countersteer_slip_gain = 1.10f;
    float smoothing_tau_low = 0.035f;
    float smoothing_tau_high = 0.085f;
    float slip_threshold_rad = 0.055f;
    float yaw_threshold_rad_s = 0.55f;
    float yaw_lateral_accel_threshold = 11.0f;
    float stability_min_speed_kph = 45.0f;
    float destabilizing_scale = 0.20f;
};

struct FilterState {
    float rate_limited = 0.0f;
    float output = 0.0f;
};

inline float clamp01(float x) { return std::max(0.0f, std::min(1.0f, x)); }
inline float sign(float x) { return x < 0.0f ? -1.0f : (x > 0.0f ? 1.0f : 0.0f); }

inline float remap_deadzone(float input, float deadzone) {
    const float magnitude = std::fabs(input);
    if (magnitude <= deadzone) return 0.0f;
    return sign(input) * clamp01((magnitude - deadzone) / (1.0f - deadzone));
}

inline float speed_factor(float speed_kph, const FilterConfig& cfg) {
    if (speed_kph <= cfg.low_speed_kph) return 0.0f;
    // Exponential falloff starts early and approaches the high-speed limit
    // smoothly. This avoids the old curve keeping too much steering until
    // roughly 150-200 km/h.
    return clamp01(1.0f - std::exp(-(speed_kph - cfg.low_speed_kph) /
                                    std::max(1.0f, cfg.speed_curve_kph)));
}

inline float steering_cap(float speed_kph, const FilterConfig& cfg) {
    const float t = speed_factor(speed_kph, cfg);
    return 1.0f + (cfg.high_speed_lock - 1.0f) * t;
}

inline float step_filter(float raw, const TelemetrySample& telemetry, float dt,
                         FilterState& state, const FilterConfig& cfg) {
    dt = std::max(0.0001f, std::min(0.050f, dt));
    raw = std::max(-1.0f, std::min(1.0f, raw));
    const float shaped = sign(raw) * std::pow(std::fabs(remap_deadzone(raw, cfg.inner_deadzone)),
                                               cfg.curve_exponent);

    const float speed_kph = telemetry.valid ? std::fabs(telemetry.speed_mps) * 3.6f : 0.0f;
    const float sf = speed_factor(speed_kph, cfg);
    float target = shaped * steering_cap(speed_kph, cfg);

    float instability = 0.0f;
    bool countersteering = false;
    float slip = 0.0f;
    if (telemetry.valid && std::fabs(telemetry.forward_velocity_mps) > 5.0f) {
        slip = std::atan2(telemetry.lateral_velocity_mps,
                          std::max(5.0f, std::fabs(telemetry.forward_velocity_mps)));
        const float yaw_threshold = std::max(
            cfg.yaw_threshold_rad_s,
            cfg.yaw_lateral_accel_threshold /
                std::max(5.0f, std::fabs(telemetry.forward_velocity_mps)));
        const float slip_strength = clamp01(
            (std::fabs(slip) - cfg.slip_threshold_rad) / 0.12f);
        const float yaw_strength = clamp01(
            (std::fabs(telemetry.yaw_rate_rad_s) - yaw_threshold) /
            std::max(0.35f, yaw_threshold));
        if (speed_kph >= cfg.stability_min_speed_kph)
            instability = std::max(slip_strength, yaw_strength);

        if (instability > 0.0f && std::fabs(raw) > 0.04f &&
            std::fabs(telemetry.yaw_rate_rad_s) > 0.08f) {
            countersteering = sign(raw) != sign(telemetry.yaw_rate_rad_s);
            if (countersteering) {
                // Normal-style countersteer: allow only as much opposite lock
                // as the current yaw/slip needs. A mild slide therefore cannot
                // become a full lock-to-lock flick from a short thumb movement.
                const float allowed = std::min(
                    steering_cap(speed_kph, cfg),
                    cfg.countersteer_min_lock +
                    std::fabs(telemetry.yaw_rate_rad_s) * cfg.countersteer_yaw_gain +
                    std::fabs(slip) * cfg.countersteer_slip_gain);
                const float limited = sign(raw) * std::min(std::fabs(target), allowed);
                target += (limited - target) * instability;
            } else {
                // Once yaw changes direction, rapidly remove the old correction
                // instead of feeding the pendulum effect.
                target *= 1.0f - instability * (1.0f - cfg.destabilizing_scale);
            }
        }
    }

    float rate = cfg.low_speed_turn_rate +
                 (cfg.high_speed_turn_rate - cfg.low_speed_turn_rate) * sf;
    if (std::fabs(target) < std::fabs(state.rate_limited) || sign(target) != sign(state.rate_limited))
        rate = cfg.recenter_rate;
    if (countersteering) {
        const bool crossing_centre = std::fabs(state.rate_limited) > 0.01f &&
                                     sign(target) != sign(state.rate_limited);
        const float assisted_rate = crossing_centre ? cfg.countersteer_cross_rate
                                                    : cfg.countersteer_rate;
        rate += (assisted_rate - rate) * instability;
    }

    const float delta = target - state.rate_limited;
    const float max_delta = rate * dt;
    state.rate_limited += std::max(-max_delta, std::min(max_delta, delta));

    const float tau = cfg.smoothing_tau_low +
                      (cfg.smoothing_tau_high - cfg.smoothing_tau_low) * sf;
    const float alpha = 1.0f - std::exp(-dt / std::max(0.001f, tau));
    state.output += (state.rate_limited - state.output) * alpha;
    if (std::fabs(raw) < cfg.inner_deadzone * 0.5f && std::fabs(state.output) < 0.0005f) {
        state.rate_limited = 0.0f;
        state.output = 0.0f;
    }
    return std::max(-1.0f, std::min(1.0f, state.output));
}

} // namespace allydrive
