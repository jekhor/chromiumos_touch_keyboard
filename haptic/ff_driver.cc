// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "logging.h"
#include <cstddef>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <unistd.h>

#include "haptic/ff_driver.h"

namespace {
// This value drive the vibrator at max strength.
constexpr int kMaxDriverInput = 0xffff;
}

namespace touch_keyboard {

FFDriver::FFDriver() : fd_ {-1} {}

FFDriver::~FFDriver() {
  CloseFDIfValid();
}

// Try to open the device, will return false if failed.
bool FFDriver::Init(const std::string& device_path) {
  struct input_event ie;
  // Close the fd if previously inited.
  CloseFDIfValid();

  fd_ = open(device_path.c_str(), O_RDWR | O_CLOEXEC);
  if (fd_ == -1) {
    PLOG(ERROR) << "Fail to open haptic device\n";
    return false;
  }

  ie.type = EV_FF;
  ie.code = FF_GAIN;
  ie.value = 0xFFFF;

  if (write(fd_, &ie, sizeof(ie)) == -1)
    PLOG(ERROR) << "Failed to set FF gain: " << errno << "\n";

  return true;
}

int FFDriver::UploadEffect(float magnitude, int time_ms) {
  if (fd_ == -1) {
    PLOG(DEBUG) << "Cannot upload effect cause FFDriver is not initialized\n";
    return -1;
  }
  struct ff_effect effect;
  // Set up the effect with parameters.
  effect.type = FF_RUMBLE;
  effect.id = -1;
  effect.u.rumble.strong_magnitude =
      static_cast<int>(magnitude * kMaxDriverInput);
  effect.u.rumble.weak_magnitude = 0;
  effect.replay.length = time_ms;
  effect.replay.delay = 0;

  if (ioctl(fd_, EVIOCSFF, &effect) == -1) {
    PLOG(ERROR) << "Fail to upload effect\n";
    return -1;
  }

  return effect.id;
}

bool FFDriver::PlayEffect(int id) {
  if (fd_ == -1) {
    PLOG(DEBUG) << "Cannot play effect cause FFDriver is not initialized\n";
    return false;
  }

  if (id < 0) {
    PLOG(ERROR) << "Invalid effect id\n";
    return false;
  }

  struct input_event play;
  memset(&play, 0, sizeof(play));
  play.type = EV_FF;
  play.code = id;
  play.value = 1;

  if (write(fd_, (const void*) &play, sizeof(play)) != sizeof(play)) {
    PLOG(ERROR) << "Fail to play effect\n";
    return false;
  }
  return true;
}

void FFDriver::CloseFDIfValid() {
  if (fd_ != -1) {
    close(fd_);
  }
}

}  // namespace touch_keyboard
