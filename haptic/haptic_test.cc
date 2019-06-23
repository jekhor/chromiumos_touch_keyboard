// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/* touchkb_haptic_test: This command line tool is designed for haptic
 * test of touch keyboard. You can run left or right motor seperately
 * with it.
 * Usage:
 * $touchkb_haptic_test --duration_ms=100 --vibrator=right --magnitude=1.0
 */
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include <brillo/flag_helper.h>
#include "touch_keyboard/haptic/ff_driver.h"

namespace {
// Support up to 10 sec of vibration.
constexpr int kMaxDuration = 10000;
// Path for both vibrators.
const char kLeftVibratorPath[] = "/dev/left_vibrator";
const char kRightVibratorPath[] = "/dev/right_vibrator";
}

int main(int argc, char **argv) {
  // Path for the selected vibrator.
  const char* vibrator_path = kLeftVibratorPath;
  DEFINE_string(vibrator, "left", "Select left or right motor.");
  DEFINE_double(magnitude, 1.0,
                "Strength of vibration, value is from 0.0 to 1.0.");
  DEFINE_int32(duration_ms, 1000,
                "Duration of the vibration in ms.");

  brillo::FlagHelper::Init(
      argc, argv, "touchkb_haptic_test, test touch keyboard haptic.");

  // Check vibrator selection.
  if (FLAGS_vibrator == "right") {
    // This will triger the right motor.
    vibrator_path = kRightVibratorPath;
  } else if (FLAGS_vibrator != "left") {
    LOG(ERROR) << "Vibrator can only be left or right";
    exit(1);
  }

  // Check magnitude.
  if (FLAGS_magnitude < 0.0 || FLAGS_magnitude > 1.0) {
    LOG(ERROR) << "Magnitude must between 0.0 and 1.0";
    exit(1);
  }

  // Check duration.
  if (FLAGS_duration_ms <= 0 || FLAGS_duration_ms >= kMaxDuration) {
    LOG(ERROR) << "Only support duraiton_ms from 0 to " << kMaxDuration;
    exit(1);
  }

  // Register event and fire the event.
  touch_keyboard::FFDriver driver;
  driver.Init(vibrator_path);
  int id = driver.UploadEffect(FLAGS_magnitude, FLAGS_duration_ms);

  if (id == -1) {
    LOG(ERROR) << "Effect upload failed";
    exit(1);
  }
  driver.PlayEffect(id);

  // Wait until the effect is played.
  usleep(FLAGS_duration_ms * 1000);
  return 0;
}
