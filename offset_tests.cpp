#include <cstddef>
#include <cstdio>

#ifndef AMS2_SHARED_MEMORY_HEADER
#define AMS2_SHARED_MEMORY_HEADER "SharedMemory.h"
#endif
#include AMS2_SHARED_MEMORY_HEADER

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
    unsigned version, build_version, game_state, session_state, race_state;
    int viewed_participant, participant_count;
    ParticipantInfoPrefix participants[64];
    float unfiltered_throttle, unfiltered_brake, unfiltered_steering, unfiltered_clutch;
    char car_name[64], car_class_name[64];
    unsigned laps_in_event;
    char track_location[64], track_variation[64];
    float track_length;
    int num_sectors;
    bool lap_invalidated;
    float best_lap_time, last_lap_time, current_time, split_time_ahead, split_time_behind;
    float split_time, event_time_remaining, personal_fastest_lap_time, world_fastest_lap_time;
    float current_sector1_time, current_sector2_time, current_sector3_time;
    float fastest_sector1_time, fastest_sector2_time, fastest_sector3_time;
    float personal_fastest_sector1_time, personal_fastest_sector2_time, personal_fastest_sector3_time;
    float world_fastest_sector1_time, world_fastest_sector2_time, world_fastest_sector3_time;
    unsigned highest_flag_colour, highest_flag_reason, pit_mode, pit_schedule, car_flags;
    float oil_temp, oil_pressure, water_temp, water_pressure, fuel_pressure, fuel_level, fuel_capacity;
    float speed, rpm, max_rpm, brake, throttle, clutch, steering;
    int gear, num_gears;
    float odometer;
    bool abs_active;
    int last_collision_index;
    float last_collision_magnitude;
    bool boost_active;
    float boost_amount;
    float orientation[3], local_velocity[3], world_velocity[3], angular_velocity[3];
    float local_acceleration[3];
};

#define CHECK_OFFSET(ours, official) do { \
    const auto a = offsetof(SharedMemoryPrefix, ours); \
    const auto b = offsetof(SharedMemory, official); \
    if (a != b) { std::printf(#ours " offset %zu != %zu\n", a, b); return 1; } \
} while (false)

int main() {
    CHECK_OFFSET(version, mVersion);
    CHECK_OFFSET(game_state, mGameState);
    CHECK_OFFSET(unfiltered_steering, mUnfilteredSteering);
    CHECK_OFFSET(speed, mSpeed);
    CHECK_OFFSET(local_velocity, mLocalVelocity);
    CHECK_OFFSET(angular_velocity, mAngularVelocity);
    CHECK_OFFSET(local_acceleration, mLocalAcceleration);
    std::puts("AMS2 shared-memory offsets verified");
    return 0;
}
