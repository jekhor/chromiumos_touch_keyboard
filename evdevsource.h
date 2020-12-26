// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_EVDEVSOURCE_H_
#define TOUCH_KEYBOARD_EVDEVSOURCE_H_

#include <base/logging.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>

#include "touch_keyboard/syscallhandler.h"

namespace touch_keyboard {

// Timeout value to use when you want the EvdevSource to block indefinitely
// when calling GetNextEvent().
constexpr int kNoTimeout = -1;

class EvdevSource {
 /* A class that uses an Evdev device as an event source
  *
  * This class opens an Evdev device and allows you to easily process the
  * events it is producing.  Generally speaking, you should probably derive
  * from this class instead of instantiating it directly.  Essentially you
  * should call OpenSourceDevice() at the beginning, then repeatedly
  * call GetNextEvent() to collect up the individual events being produced
  * by the Evdev device you selected.
  */
 public:
  EvdevSource() : syscall_handler_(&default_syscall_handler),
                  source_fd_(-1) { }
  explicit EvdevSource(SyscallHandler *syscall_handler) :
      syscall_handler_(syscall_handler), source_fd_(-1) {
    // This constructor allows you to pass in a SyscallHandler when unit
    // testing this class.  For real use, allow it to use the default value
    // by using the constructor with no arguments.
    if (syscall_handler_ == NULL) {
      LOG(INFO) << "NULL syscall_handler specified, using default.";
      syscall_handler_ = &default_syscall_handler;
    }
  }

 protected:
  // Open the device file on disk and store the descriptor in this object.
  bool OpenSourceDevice(std::string const &source_device_path);
  // Wait for a new event to come from the source and populate *ev with it.
  bool GetNextEvent(int timeout_ms, struct input_event *ev) const;

  SyscallHandler *syscall_handler_;
  int source_fd_;
};

}  // namespace touch_keyboard

#endif  // TOUCH_KEYBOARD_EVDEVSOURCE_H_
