// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "touch_keyboard/evdevsource.h"

namespace touch_keyboard {

bool EvdevSource::OpenSourceDevice(std::string const &source_device_path) {
  source_fd_ = syscall_handler_->open(source_device_path.c_str(), O_RDONLY);
  if (source_fd_ < 0) {
    PLOG(ERROR) << "Failed to open() source device " << source_device_path << ". (" << source_fd_ << ")";
    return false;
  }
  return true;
}

bool EvdevSource::GetNextEvent(int timeout_ms, struct input_event *ev) const {
  if (timeout_ms > 0) {
    int num_ready;
    struct timeval timeout = {0, timeout_ms * 1000};
    fd_set set;

    // Block until there's something to read or we hit a timeout.
    FD_ZERO(&set);
    FD_SET(source_fd_, &set);
    num_ready = syscall_handler_->select(source_fd_ + 1, &set,
                                         NULL, NULL, &timeout);

    // If the timeout triggered, return false instead of waiting forever.
    if (num_ready != 1) {
      return false;
    }
  }

  int num_bytes_read = syscall_handler_->read(source_fd_, ev, sizeof(*ev));
  if (num_bytes_read != sizeof(*ev)) {
    PLOG(ERROR) << "ERROR: A read failed to read an entire event. Only read " <<
                   num_bytes_read << " of " << sizeof(*ev) << "expected bytes.";
    return false;
  }
  return true;
}

}  // namespace touch_keyboard
