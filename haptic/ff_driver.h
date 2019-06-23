// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_HAPTIC_FF_DRIVER_H_
#define TOUCH_KEYBOARD_HAPTIC_FF_DRIVER_H_

#include <string>

#include "base/macros.h"

namespace touch_keyboard {

class FFDriver {
  /* FFDriver is one haptic driver.
   * Once it is initialized, you can upload effect and later play
   * the effect.
   */
 public:
  FFDriver();

  ~FFDriver();

  // Try to open the device at the path. Will return false if fails.
  bool Init(const std::string& device_path);

  // Upload an effect for the event. It will return the effect id.
  // When it fails to upload, id -1 is returned.
  int UploadEffect(float magnitude, int time_ms);

  // Play the effect loaded corresponding to the event. It will return
  // false if fails.
  bool PlayEffect(int id);

 private:
  // This is the file descriptor for the driver.
  int fd_;

  // This function close the fd
  void CloseFDIfValid();

  DISALLOW_COPY_AND_ASSIGN(FFDriver);
};

}  // namespace touch_keyboard

#endif  // TOUCH_KEYBOARD_HAPTIC_FF_DRIVER_H_
