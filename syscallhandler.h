// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_SYSCALLHANDLER_H_
#define TOUCH_KEYBOARD_SYSCALLHANDLER_H_

#include <fcntl.h>
#include <linux/uinput.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "touch_keyboard/uinput_definitions.h"

namespace touch_keyboard {

class SyscallHandler {
  /* This class wraps raw syscall access when using them in this module.
   *
   * This class wraps the basic low-level system calls you would make to
   * raw file descriptors when interacting with devices on the OS.  The
   * intention is to allow for us to mock these functions away for improved
   * testing, otherwise it's very difficult to write unit tests for
   * classes that use open(), read(), ioctl(), etc.
   *
   * All functions directly mirror the standard syscalls.
   */
 public:
    virtual int open(const char* pathname, int flags) const {
      return ::open(pathname, flags);
    }

    virtual ssize_t write(int fd, const void *buf, size_t count) const {
      return ::write(fd, buf, count);
    }

    virtual ssize_t read(int fd, void *buf, size_t count) const {
      return ::read(fd, buf, count);
    }

    virtual int select(int nfds, fd_set *readfds, fd_set *writefds,
                       fd_set *exceptfds, struct timeval *timeout) const {
      return ::select(nfds, readfds, writefds, exceptfds, timeout);
    }

    virtual int ioctl(int fd, long request_code) const {
      return ::ioctl(fd, request_code);
    }

    virtual int ioctl(int fd, long request_code, uint64_t arg1) const {
      return ::ioctl(fd, request_code, arg1);
    }

    virtual int ioctl(int fd, long request_code, int64_t *arg1) const {
      return ::ioctl(fd, request_code, arg1);
    }

    virtual int ioctl(int fd, long request_code,
                      struct input_absinfo *arg1) const {
      return ::ioctl(fd, request_code, arg1);
    }

    virtual int ioctl(int fd, long request_code,
                      struct uinput_abs_setup *arg1) const {
      return ::ioctl(fd, request_code, arg1);
    }

    virtual int ioctl(int fd, long request_code,
                      struct uinput_setup *arg1) const {
      return ::ioctl(fd, request_code, arg1);
    }
};

static SyscallHandler default_syscall_handler;

}  // namespace touch_keyboard

#endif  // TOUCH_KEYBOARD_SYSCALLHANDLER_H_
