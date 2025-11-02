// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_HAPTIC_TOUCH_FF_MANAGER_H_
#define TOUCH_KEYBOARD_HAPTIC_TOUCH_FF_MANAGER_H_

#include <unordered_map>

#include "base_macros.h"
#include "haptic/ff_driver.h"

namespace touch_keyboard {

// This is a set of events that might be interested to TouchFFManager.
// When the event happens, it should report the event to TouchFFManager, then
// TouchFFManager can decide how to play/cancel ff effect.
enum class TouchKeyboardEvent {
  FingerDown,
  SendKey,
  FingerUp,
};

// Default magnitude for haptic feedback.
constexpr double kDefaultHapticMagnitude = 1.0;
// Default duration for haptic feedback in ms.
constexpr int kDefaultHapticDurationMs = 6;

class TouchFFManager {
  /* TouchFFManager (touch force feeback manager) class manages the haptic
   * feedback for the touch keyboard. It is initialized with the length of x
   * axis of the touch surface. When some TouchFFEvent is happening,
   * TouchFFManager should be notified with the x position, then it will decide
   * when and how to fire the vibration.
   */
 public:
  // Init TouchFFManager with max_x, where max_x is the max possible x
  // of the touch surface.
  explicit TouchFFManager(int max_x, int max_y, int rotation,
                          double magnitude = kDefaultHapticMagnitude,
                          int duration_ms = kDefaultHapticDurationMs);

  // Inform the TouchFFManager that a particular kind of keyboard event as
  // happened at which x location. This may or may not trigger haptic feedback.
  // X value should be between 0 and max_x. X = 0 means the left side of the
  // touch surface.
  void EventTriggered(TouchKeyboardEvent event, int x, int y);

  // Register force feeback effect for the touch keyboard event. When the event
  // is triggered later, the effect will be played.
  void RegisterFF(TouchKeyboardEvent event, double magnitude, int length_ms);

 private:
  // This hash functor makes sure enum class works with unordered map.
  struct TouchKeyboardEventHash {
    std::size_t operator()(TouchKeyboardEvent event) const {
      return static_cast<std::size_t>(event);
    }
  };

  // This helper function get the effect id of the event from lib and then play
  // the effect on the driver.
  void PlayEffectOfEvent(
      TouchKeyboardEvent event, FFDriver* driver,
      std::unordered_map<TouchKeyboardEvent, int, TouchKeyboardEventHash>* lib);

  FFDriver left_driver_;
  FFDriver right_driver_;

  std::unordered_map<TouchKeyboardEvent, int, TouchKeyboardEventHash>
      left_driver_fflib_;
  std::unordered_map<TouchKeyboardEvent, int, TouchKeyboardEventHash>
      right_driver_fflib_;

  // This is the max x axis value of the touchpad.
  int touch_max_x_;

  // Touchscreen rotation angle, CW
  int touch_rotation_;

  double magnitude_;
  int duration_ms_;

  DISALLOW_COPY_AND_ASSIGN(TouchFFManager);
};

}  // namespace touch_keyboard

#endif  // TOUCH_KEYBOARD_HAPTIC_TOUCH_FF_MANAGER_H_
