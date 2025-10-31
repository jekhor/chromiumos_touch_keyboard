// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "logging.h"
#include <cstddef>

#include "haptic/touch_ff_manager.h"

namespace {
// Default magnitude for haptic feedback.
constexpr double kDefaultHapticMagnitue = 1.0;
// Default duration for haptic feedback in ms.
constexpr int kDefaultHapticDurationMs = 4;
// Path for left and right vibrators.
const char kLeftVibratorPath[] = "/dev/left_vibrator";
const char kRightVibratorPath[] = "/dev/right_vibrator";
}

namespace touch_keyboard {

TouchFFManager::TouchFFManager(int max_x, int max_y, int rotation) {
    switch (rotation) {
      case 0:
      case 180:
        touch_max_x_ = max_x;
        break;
      case 90:
      case 270:
        touch_max_x_ = max_y;
        break;
      default:
        LOG(ERROR) << "Invalid rotation angle: " << rotation << "\n";
    }

    touch_rotation_ = rotation;

    if (!left_driver_.Init(kLeftVibratorPath)) {
      LOG(ERROR) << "Cannot find left motor\n";
    }

    if (!right_driver_.Init(kRightVibratorPath)) {
      LOG(ERROR) << "Cannot find right motor\n";
    }

    RegisterFF(TouchKeyboardEvent::FingerDown, kDefaultHapticMagnitue,
               kDefaultHapticDurationMs);
}

void TouchFFManager::RegisterFF(TouchKeyboardEvent event,
                                double magnitude, int length_ms) {
  int tmp_event_id;
  // The effect is uploaded to both drivers.
  tmp_event_id = left_driver_.UploadEffect(magnitude, length_ms);
  left_driver_fflib_[event] = tmp_event_id;

  tmp_event_id = right_driver_.UploadEffect(magnitude, length_ms);
  right_driver_fflib_[event] = tmp_event_id;
}

void TouchFFManager::EventTriggered(TouchKeyboardEvent event, int x, int y) {
  // Play ff effects based on the location of the event. Currently, we drive
  // left OR right motor depend on the event possition. When the event is on the
  // left half of touch surface, only the left vibrator will run.
  int val;

  switch (touch_rotation_) {
    case 0:
      val = x;
      break;
    case 90:
      val = touch_max_x_ - y;
      break;
    case 180:
      val = touch_max_x_ - x;
      break;
    case 270:
      val = y;
      break;
    default:
      val = x;
  }

  if (val < touch_max_x_ / 2) {
    PlayEffectOfEvent(event, &left_driver_, &left_driver_fflib_);
  } else {
    PlayEffectOfEvent(event, &right_driver_, &right_driver_fflib_);
  }
}

void TouchFFManager::PlayEffectOfEvent(
    TouchKeyboardEvent event,
    FFDriver* driver,
    std::unordered_map<TouchKeyboardEvent, int, TouchKeyboardEventHash>* lib) {
  auto iter = lib->find(event);
  if (iter == lib->end()) {
    return;
  }
  driver->PlayEffect(iter->second);
}

}  // namespace touch_keyboard
