/*
 * Differential-drive recruitment task
 *
 * The communication and decoding stages provide a target coordinate. Implement
 * drive_to_target() so the simulated differential-drive rover reaches the
 * target using valid left and right wheel velocities.
 */

#include <math.h>
#include <stdbool.h>

#include "drive.h"

#define PI_F 3.14159265358979323846f
#define WHEEL_RADIUS 0.15f
#define WHEEL_SEPARATION 0.77f
#define MAX_LINEAR_VELOCITY 1.0f
#define MAX_ANGULAR_VELOCITY 2.0f
#define MAX_WHEEL_VELOCITY 10.0f
#define HEADING_GAIN 1.25f
#define TARGET_TOLERANCE 0.10f
#define DRIVE_DT_SECONDS 0.02f
#define MAX_DRIVE_STEPS 6000

static float clampf(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

static float normlize_angle(float angle) {
  while (angle > PI_F) {
    angle -= 2.0f * PI_F;
  }
  while (angle < -PI_F) {
    angle += 2.0f * PI_F;
  }
  return angle;
}

static bool coordinate_is_finite(const struct coordinate *coordinate) {
  if (coordinate == NULL) {
    return false;
  }
  return isfinite(coordinate->latitude) &&
         isfinite(coordinate->longitude) &&
         isfinite(coordinate->altitude);
}

static bool rover_is_valid(const struct rover_state *rover) {
  if (rover == NULL) {
    return false;
  }
  return coordinate_is_finite(&rover->position) && isfinite(rover->heading_rad);
}

static struct wheel_velocity limit_wheel_velocities(struct wheel_velocity velocity) {
  float max_mag = fmaxf(fabsf(velocity.left), fabsf(velocity.right));
  if (max_mag > MAX_WHEEL_VELOCITY) {
    float scale = MAX_WHEEL_VELOCITY / max_mag;
    velocity.left  *= scale;
    velocity.right *= scale;
  }
  return velocity;
}

static bool apply_wheel_velocities(struct rover_state *rover,
                                   struct wheel_velocity velocity) {
  if (!isfinite(velocity.left) || !isfinite(velocity.right) ||
      fabsf(velocity.left) > MAX_WHEEL_VELOCITY ||
      fabsf(velocity.right) > MAX_WHEEL_VELOCITY) {
    return false;
  }
  const float linear_velocity =
      WHEEL_RADIUS * (velocity.left + velocity.right) / 2.0f;
  const float angular_velocity =
      WHEEL_RADIUS * (velocity.right - velocity.left) / WHEEL_SEPARATION;
  rover->heading_rad = normlize_angle(
      rover->heading_rad + angular_velocity * DRIVE_DT_SECONDS);
  rover->position.longitude +=
      linear_velocity * cosf(rover->heading_rad) * DRIVE_DT_SECONDS;
  rover->position.latitude +=
      linear_velocity * sinf(rover->heading_rad) * DRIVE_DT_SECONDS;
  return true;
}

enum drive_status drive_to_target(struct rover_state *rover,
                                  const struct coordinate *target) {
  if (!rover_is_valid(rover) || !coordinate_is_finite(target)) {
    return DRIVE_INVALID_INPUT;
  }

  for (int step = 0; step < MAX_DRIVE_STEPS; step++) {
    float dlat = target->latitude  - rover->position.latitude;
    float dlon = target->longitude - rover->position.longitude;
    float distance = hypotf(dlat, dlon);

    if (distance <= TARGET_TOLERANCE) {
      return DRIVE_REACHED_TARGET;
    }

    float target_heading = atan2f(dlat, dlon);
    float heading_error = normlize_angle(target_heading - rover->heading_rad);

    float angular_velocity = clampf(HEADING_GAIN * heading_error,
                                     -MAX_ANGULAR_VELOCITY, MAX_ANGULAR_VELOCITY);

    float linear_velocity = MAX_LINEAR_VELOCITY * cosf(heading_error);
    if (linear_velocity < 0.0f) {
      linear_velocity = 0.0f;
    }

    struct wheel_velocity velocity;
    velocity.right = (2.0f * linear_velocity + angular_velocity * WHEEL_SEPARATION)
                      / (2.0f * WHEEL_RADIUS);
    velocity.left  = (2.0f * linear_velocity - angular_velocity * WHEEL_SEPARATION)
                      / (2.0f * WHEEL_RADIUS);

    velocity = limit_wheel_velocities(velocity);

    if (!apply_wheel_velocities(rover, velocity)) {
      return DRIVE_INVALID_COMMAND;
    }
  }

  return DRIVE_MAX_STEPS_EXCEEDED;
}
