#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define XInputGetState AllyDrive_SDK_XInputGetState
#define XInputSetState AllyDrive_SDK_XInputSetState
#include <xinput.h>
#undef XInputGetState
#undef XInputSetState
#include <cstdint>
#include <cmath>
#include <cstdio>
#include "filter_core.h"

// Prefix of the official AMS2 SharedMemory v14 structure. The fields and
// alignment mirror Support/SharedMemory/AMS2_SharedMemoryExampleApp/SharedMemory.h.
struct ParticipantInfoPrefix {
    bool active;
    char name[64];
    float world_position[3];
    float lap_distance;
    unsigned race_position;
    unsigned laps_completed;
    unsigned current_lap;
    int current_sector;
};

struct SharedMemoryPrefix {
    unsigned version;
    unsigned build_version;
    unsigned game_state;
    unsigned session_state;
    unsigned race_state;
    int viewed_participant;
    int participant_count;
    ParticipantInfoPrefix participants[64];
    float unfiltered_throttle;
    float unfiltered_brake;
    float unfiltered_steering;
    float unfiltered_clutch;
    char car_name[64];
    char car_class_name[64];
    unsigned laps_in_event;
    char track_location[64];
    char track_variation[64];
    float track_length;
    int num_sectors;
    bool lap_invalidated;
    float best_lap_time;
    float last_lap_time;
    float current_time;
    float split_time_ahead;
    float split_time_behind;
    float split_time;
    float event_time_remaining;
    float personal_fastest_lap_time;
    float world_fastest_lap_time;
    float current_sector1_time;
    float current_sector2_time;
    float current_sector3_time;
    float fastest_sector1_time;
    float fastest_sector2_time;
    float fastest_sector3_time;
    float personal_fastest_sector1_time;
    float personal_fastest_sector2_time;
    float personal_fastest_sector3_time;
    float world_fastest_sector1_time;
    float world_fastest_sector2_time;
    float world_fastest_sector3_time;
    unsigned highest_flag_colour;
    unsigned highest_flag_reason;
    unsigned pit_mode;
    unsigned pit_schedule;
    unsigned car_flags;
    float oil_temp;
    float oil_pressure;
    float water_temp;
    float water_pressure;
    float fuel_pressure;
    float fuel_level;
    float fuel_capacity;
    float speed;
    float rpm;
    float max_rpm;
    float brake;
    float throttle;
    float clutch;
    float steering;
    int gear;
    int num_gears;
    float odometer;
    bool abs_active;
    int last_collision_index;
    float last_collision_magnitude;
    bool boost_active;
    float boost_amount;
    float orientation[3];
    float local_velocity[3];
    float world_velocity[3];
    float angular_velocity[3];
    float local_acceleration[3];
};

using GetStateFn = DWORD (WINAPI*)(DWORD, XINPUT_STATE*);
using SetStateFn = DWORD (WINAPI*)(DWORD, XINPUT_VIBRATION*);

static INIT_ONCE g_init_once = INIT_ONCE_STATIC_INIT;
static HMODULE g_real_module = nullptr;
static GetStateFn g_real_get_state = nullptr;
static SetStateFn g_real_set_state = nullptr;
static HANDLE g_mapping = nullptr;
static const SharedMemoryPrefix* g_telemetry = nullptr;
static CRITICAL_SECTION g_lock;
static LARGE_INTEGER g_frequency{};
static LARGE_INTEGER g_last_tick[4]{};
static allydrive::FilterState g_filter[4]{};
static DWORD g_packet[4]{};
static SHORT g_last_output[4]{};
static SHORT g_last_camera_x[4]{};
static SHORT g_last_camera_y[4]{};
static allydrive::FilterConfig g_config{};
static bool g_enabled = true;
static bool g_chord_was_down = false;
static wchar_t g_ini_path[MAX_PATH]{};

struct CameraConfig {
    bool enabled = true;
    bool auto_select_chase = true;
    bool auto_fov = true;
    int fov_min_steps = 100;
    int fov_target_steps = 20;
    float manual_deadzone = 0.10f;
    float yaw_gain = 0.15f;
    float slip_gain = 0.85f;
    float pitch_gain = 0.010f;
    float max_yaw = 0.24f;
    float max_pitch = 0.075f;
    float smoothing_tau = 0.20f;
    float yaw_polarity = -1.0f;
};
struct CameraState { float yaw = 0.0f; float pitch = 0.0f; };
static CameraConfig g_camera_config{};
static CameraState g_camera_state[4]{};
static int g_camera_setup_stage = 0;
static int g_camera_setup_count = 0;
static DWORD g_camera_setup_tick = 0;

static float read_ini_float(const wchar_t* key, float fallback) {
    wchar_t fallback_text[32]{};
    wchar_t value[32]{};
    swprintf_s(fallback_text, L"%.6g", fallback);
    GetPrivateProfileStringW(L"Filter", key, fallback_text, value, 32, g_ini_path);
    wchar_t* end = nullptr;
    const double parsed = wcstod(value, &end);
    return end == value ? fallback : static_cast<float>(parsed);
}

static float read_ini_float_section(const wchar_t* section, const wchar_t* key, float fallback) {
    wchar_t fallback_text[32]{}, value[32]{};
    swprintf_s(fallback_text, L"%.6g", fallback);
    GetPrivateProfileStringW(section, key, fallback_text, value, 32, g_ini_path);
    wchar_t* end = nullptr;
    const double parsed = wcstod(value, &end);
    return end == value ? fallback : static_cast<float>(parsed);
}

static void load_config() {
    g_config.inner_deadzone = read_ini_float(L"InnerDeadzone", g_config.inner_deadzone);
    g_config.curve_exponent = read_ini_float(L"CurveExponent", g_config.curve_exponent);
    g_config.low_speed_kph = read_ini_float(L"LowSpeedKph", g_config.low_speed_kph);
    g_config.speed_curve_kph = read_ini_float(L"SpeedCurveKph", g_config.speed_curve_kph);
    g_config.high_speed_lock = read_ini_float(L"HighSpeedLock", g_config.high_speed_lock);
    g_config.low_speed_turn_rate = read_ini_float(L"LowSpeedTurnRate", g_config.low_speed_turn_rate);
    g_config.high_speed_turn_rate = read_ini_float(L"HighSpeedTurnRate", g_config.high_speed_turn_rate);
    g_config.recenter_rate = read_ini_float(L"RecenterRate", g_config.recenter_rate);
    g_config.countersteer_rate = read_ini_float(L"CountersteerRate", g_config.countersteer_rate);
    g_config.countersteer_cross_rate = read_ini_float(L"CountersteerCrossRate", g_config.countersteer_cross_rate);
    g_config.countersteer_min_lock = read_ini_float(L"CountersteerMinLock", g_config.countersteer_min_lock);
    g_config.countersteer_yaw_gain = read_ini_float(L"CountersteerYawGain", g_config.countersteer_yaw_gain);
    g_config.countersteer_slip_gain = read_ini_float(L"CountersteerSlipGain", g_config.countersteer_slip_gain);
    g_config.smoothing_tau_low = read_ini_float(L"SmoothingTauLow", g_config.smoothing_tau_low);
    g_config.smoothing_tau_high = read_ini_float(L"SmoothingTauHigh", g_config.smoothing_tau_high);
    g_config.slip_threshold_rad = read_ini_float(L"SlipThresholdRad", g_config.slip_threshold_rad);
    g_config.yaw_threshold_rad_s = read_ini_float(L"YawThresholdRadS", g_config.yaw_threshold_rad_s);
    g_config.yaw_lateral_accel_threshold = read_ini_float(L"YawLateralAccelThreshold", g_config.yaw_lateral_accel_threshold);
    g_config.stability_min_speed_kph = read_ini_float(L"StabilityMinSpeedKph", g_config.stability_min_speed_kph);
    g_config.destabilizing_scale = read_ini_float(L"DestabilizingScale", g_config.destabilizing_scale);
    g_enabled = GetPrivateProfileIntW(L"Filter", L"Enabled", 1, g_ini_path) != 0;
    g_camera_config.enabled = GetPrivateProfileIntW(L"Camera", L"Enabled", 1, g_ini_path) != 0;
    g_camera_config.auto_select_chase = GetPrivateProfileIntW(L"Camera", L"AutoSelectChase", 1, g_ini_path) != 0;
    g_camera_config.auto_fov = GetPrivateProfileIntW(L"Camera", L"AutoFov", 1, g_ini_path) != 0;
    g_camera_config.fov_min_steps = GetPrivateProfileIntW(L"Camera", L"FovMinSteps", 100, g_ini_path);
    g_camera_config.fov_target_steps = GetPrivateProfileIntW(L"Camera", L"FovTargetSteps", 20, g_ini_path);
    g_camera_config.manual_deadzone = read_ini_float_section(L"Camera", L"ManualDeadzone", g_camera_config.manual_deadzone);
    g_camera_config.yaw_gain = read_ini_float_section(L"Camera", L"YawGain", g_camera_config.yaw_gain);
    g_camera_config.slip_gain = read_ini_float_section(L"Camera", L"SlipGain", g_camera_config.slip_gain);
    g_camera_config.pitch_gain = read_ini_float_section(L"Camera", L"PitchGain", g_camera_config.pitch_gain);
    g_camera_config.max_yaw = read_ini_float_section(L"Camera", L"MaxYaw", g_camera_config.max_yaw);
    g_camera_config.max_pitch = read_ini_float_section(L"Camera", L"MaxPitch", g_camera_config.max_pitch);
    g_camera_config.smoothing_tau = read_ini_float_section(L"Camera", L"SmoothingTau", g_camera_config.smoothing_tau);
    g_camera_config.yaw_polarity = read_ini_float_section(L"Camera", L"YawPolarity", g_camera_config.yaw_polarity);
}

static BOOL CALLBACK initialize(PINIT_ONCE, PVOID, PVOID*) {
    InitializeCriticalSection(&g_lock);
    QueryPerformanceFrequency(&g_frequency);

    wchar_t module_path[MAX_PATH]{};
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&initialize), &self);
    GetModuleFileNameW(self, module_path, MAX_PATH);
    wchar_t* slash = wcsrchr(module_path, L'\\');
    if (slash) {
        *(slash + 1) = L'\0';
        wcscpy_s(g_ini_path, module_path);
        wcscat_s(g_ini_path, L"allydrive.ini");
    }
    load_config();

    wchar_t system_path[MAX_PATH]{};
    GetSystemDirectoryW(system_path, MAX_PATH);
    wcscat_s(system_path, L"\\XINPUT9_1_0.dll");
    g_real_module = LoadLibraryW(system_path);
    if (g_real_module) {
        g_real_get_state = reinterpret_cast<GetStateFn>(GetProcAddress(g_real_module, "XInputGetState"));
        g_real_set_state = reinterpret_cast<SetStateFn>(GetProcAddress(g_real_module, "XInputSetState"));
    }
    return TRUE;
}

static void try_open_telemetry() {
    if (g_telemetry) return;
    if (!g_mapping) g_mapping = OpenFileMappingW(FILE_MAP_READ, FALSE, L"$pcars2$");
    if (g_mapping) {
        g_telemetry = static_cast<const SharedMemoryPrefix*>(
            MapViewOfFile(g_mapping, FILE_MAP_READ, 0, 0, sizeof(SharedMemoryPrefix)));
    }
}

static allydrive::TelemetrySample sample_telemetry() {
    try_open_telemetry();
    allydrive::TelemetrySample sample{};
    if (!g_telemetry || g_telemetry->version != 14) return sample;
    sample.valid = g_telemetry->game_state == 2 || g_telemetry->game_state == 4;
    // These fields are naturally aligned floats. Read twice and accept the
    // second snapshot if AMS2 happened to update between reads.
    sample.speed_mps = g_telemetry->speed;
    sample.lateral_velocity_mps = g_telemetry->local_velocity[0];
    sample.forward_velocity_mps = g_telemetry->local_velocity[2];
    sample.yaw_rate_rad_s = g_telemetry->angular_velocity[1];
    sample.longitudinal_accel_mps2 = g_telemetry->local_acceleration[2];
    MemoryBarrier();
    const float speed2 = g_telemetry->speed;
    const float lateral2 = g_telemetry->local_velocity[0];
    const float forward2 = g_telemetry->local_velocity[2];
    const float yaw2 = g_telemetry->angular_velocity[1];
    const float acceleration2 = g_telemetry->local_acceleration[2];
    if (speed2 != sample.speed_mps || lateral2 != sample.lateral_velocity_mps ||
        forward2 != sample.forward_velocity_mps || yaw2 != sample.yaw_rate_rad_s) {
        sample.speed_mps = speed2;
        sample.lateral_velocity_mps = lateral2;
        sample.forward_velocity_mps = forward2;
        sample.yaw_rate_rad_s = yaw2;
    }
    sample.longitudinal_accel_mps2 = acceleration2;
    if (!std::isfinite(sample.speed_mps) || !std::isfinite(sample.yaw_rate_rad_s)) sample.valid = false;
    return sample;
}

static float normalize_axis(SHORT value) {
    return value < 0 ? static_cast<float>(value) / 32768.0f
                     : static_cast<float>(value) / 32767.0f;
}

static SHORT encode_axis(float value) {
    value = std::max(-1.0f, std::min(1.0f, value));
    const float scale = value < 0.0f ? 32768.0f : 32767.0f;
    return static_cast<SHORT>(std::lround(value * scale));
}

static bool game_has_focus() {
    DWORD process_id = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &process_id);
    return process_id == GetCurrentProcessId();
}

static void send_key_combo(WORD key, bool control) {
    INPUT inputs[4]{};
    UINT count = 0;
    if (control) {
        inputs[count].type = INPUT_KEYBOARD;
        inputs[count++].ki.wVk = VK_CONTROL;
    }
    inputs[count].type = INPUT_KEYBOARD;
    inputs[count++].ki.wVk = key;
    inputs[count].type = INPUT_KEYBOARD;
    inputs[count].ki.wVk = key;
    inputs[count++].ki.dwFlags = KEYEVENTF_KEYUP;
    if (control) {
        inputs[count].type = INPUT_KEYBOARD;
        inputs[count].ki.wVk = VK_CONTROL;
        inputs[count++].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    SendInput(count, inputs, sizeof(INPUT));
}

static void advance_automatic_camera_setup(const allydrive::TelemetrySample& telemetry) {
    if ((!g_camera_config.auto_select_chase && !g_camera_config.auto_fov) ||
        g_camera_setup_stage == 4 || !telemetry.valid || !game_has_focus() ||
        std::fabs(telemetry.speed_mps) > 4.0f) return;
    const DWORD now = GetTickCount();
    if (now - g_camera_setup_tick < 35) return;
    g_camera_setup_tick = now;

    if (g_camera_setup_stage == 0) {
        if (g_camera_config.auto_select_chase) send_key_combo(L'3', false);
        g_camera_setup_stage = g_camera_config.auto_fov ? 1 : 4;
        g_camera_setup_count = 0;
        return;
    }
    if (g_camera_setup_stage == 1) {
        send_key_combo(VK_RIGHT, true); // Drive the active chase view to minimum FOV.
        if (++g_camera_setup_count >= g_camera_config.fov_min_steps) {
            g_camera_setup_stage = 2;
            g_camera_setup_count = 0;
        }
        return;
    }
    if (g_camera_setup_stage == 2) {
        send_key_combo(VK_LEFT, true); // Minimum 35 + 20 steps = FOV 55.
        if (++g_camera_setup_count >= g_camera_config.fov_target_steps)
            g_camera_setup_stage = 4;
    }
}

static void apply_camera_assist(DWORD user_index, XINPUT_STATE* state,
                                const allydrive::TelemetrySample& telemetry, float dt) {
    if (user_index == 0) advance_automatic_camera_setup(telemetry);
    if (!g_camera_config.enabled) return;
    const float manual_x = normalize_axis(state->Gamepad.sThumbRX);
    const float manual_y = normalize_axis(state->Gamepad.sThumbRY);
    auto& camera = g_camera_state[user_index];
    if (std::fabs(manual_x) > g_camera_config.manual_deadzone ||
        std::fabs(manual_y) > g_camera_config.manual_deadzone) {
        camera.yaw = manual_x;
        camera.pitch = manual_y;
        return;
    }

    float target_yaw = 0.0f, target_pitch = 0.0f;
    if (telemetry.valid && std::fabs(telemetry.forward_velocity_mps) > 3.0f) {
        const float slip = std::atan2(telemetry.lateral_velocity_mps,
                                      std::max(5.0f, std::fabs(telemetry.forward_velocity_mps)));
        const float speed_blend = 0.35f + 0.65f * allydrive::clamp01(
            std::fabs(telemetry.speed_mps) * 3.6f / 160.0f);
        target_yaw = g_camera_config.yaw_polarity * speed_blend *
                     (telemetry.yaw_rate_rad_s * g_camera_config.yaw_gain +
                      slip * g_camera_config.slip_gain);
        target_yaw = std::max(-g_camera_config.max_yaw,
                              std::min(g_camera_config.max_yaw, target_yaw));
        target_pitch = -telemetry.longitudinal_accel_mps2 * g_camera_config.pitch_gain;
        target_pitch = std::max(-g_camera_config.max_pitch,
                                std::min(g_camera_config.max_pitch, target_pitch));
    }
    const float alpha = 1.0f - std::exp(-dt / std::max(0.01f, g_camera_config.smoothing_tau));
    camera.yaw += (target_yaw - camera.yaw) * alpha;
    camera.pitch += (target_pitch - camera.pitch) * alpha;
    state->Gamepad.sThumbRX = encode_axis(camera.yaw);
    state->Gamepad.sThumbRY = encode_axis(camera.pitch);
}

extern "C" __declspec(dllexport) DWORD WINAPI XInputGetState(DWORD user_index, XINPUT_STATE* state) noexcept {
    InitOnceExecuteOnce(&g_init_once, initialize, nullptr, nullptr);
    if (!g_real_get_state) return ERROR_DEVICE_NOT_CONNECTED;
    const DWORD result = g_real_get_state(user_index, state);
    if (result != ERROR_SUCCESS || !state || user_index >= 4) return result;

    EnterCriticalSection(&g_lock);
    const WORD chord = XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_RIGHT_SHOULDER | XINPUT_GAMEPAD_BACK;
    const bool chord_down = (state->Gamepad.wButtons & chord) == chord;
    if (user_index == 0 && chord_down && !g_chord_was_down) g_enabled = !g_enabled;
    if (user_index == 0) g_chord_was_down = chord_down;
    if (!g_enabled) {
        LeaveCriticalSection(&g_lock);
        return result;
    }

    LARGE_INTEGER now{};
    QueryPerformanceCounter(&now);
    float dt = 1.0f / 120.0f;
    if (g_last_tick[user_index].QuadPart != 0) {
        dt = static_cast<float>(now.QuadPart - g_last_tick[user_index].QuadPart) /
             static_cast<float>(g_frequency.QuadPart);
    }
    g_last_tick[user_index] = now;

    const float raw = normalize_axis(state->Gamepad.sThumbLX);
    const auto telemetry = sample_telemetry();
    // Fixed substeps make the filter response independent of frame rate and
    // keep the integration stable up to 240 Hz.
    float remaining = std::max(0.0001f, std::min(0.050f, dt));
    float filtered = g_filter[user_index].output;
    while (remaining > 0.0f) {
        const float step = std::min(remaining, 1.0f / 240.0f);
        filtered = allydrive::step_filter(raw, telemetry, step, g_filter[user_index], g_config);
        remaining -= step;
    }
    const SHORT encoded = encode_axis(filtered);
    state->Gamepad.sThumbLX = encoded;
    apply_camera_assist(user_index, state, telemetry, dt);
    if (encoded != g_last_output[user_index] ||
        state->Gamepad.sThumbRX != g_last_camera_x[user_index] ||
        state->Gamepad.sThumbRY != g_last_camera_y[user_index]) {
        g_last_output[user_index] = encoded;
        g_last_camera_x[user_index] = state->Gamepad.sThumbRX;
        g_last_camera_y[user_index] = state->Gamepad.sThumbRY;
        ++g_packet[user_index];
    }
    state->dwPacketNumber += g_packet[user_index];
    LeaveCriticalSection(&g_lock);
    return result;
}

extern "C" __declspec(dllexport) DWORD WINAPI XInputSetState(DWORD user_index, XINPUT_VIBRATION* vibration) noexcept {
    InitOnceExecuteOnce(&g_init_once, initialize, nullptr, nullptr);
    return g_real_set_state ? g_real_set_state(user_index, vibration) : ERROR_DEVICE_NOT_CONNECTED;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    // Windows releases process-owned mappings and modules at shutdown. Avoid
    // loader operations from DLL_PROCESS_DETACH, where the loader lock is held.
    (void)instance;
    (void)reason;
    return TRUE;
}
