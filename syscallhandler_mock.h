// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_SYSCALLHANDLER_MOCK_H_
#define TOUCH_KEYBOARD_SYSCALLHANDLER_MOCK_H_

#include <gmock/gmock.h>

#include "touch_keyboard/syscallhandler.h"

namespace touch_keyboard {

class MockSyscallHandler : public SyscallHandler {
 public:
  MOCK_CONST_METHOD2(open, int(const char* pathname, int flags));
  MOCK_CONST_METHOD3(read, ssize_t(int fd, void *buf, size_t count));
  MOCK_CONST_METHOD3(write, ssize_t(int fd, const void *buf, size_t count));
  MOCK_CONST_METHOD5(select, int(int nfds, fd_set *readfds, fd_set *writefds,
                                 fd_set *except_fds, struct timeval *timeout));

  MOCK_CONST_METHOD2(ioctl, int(int fd, long request_code));
  MOCK_CONST_METHOD3(ioctl, int(int fd, long request_code, uint64_t arg1));
  MOCK_CONST_METHOD3(ioctl, int(int fd, long request_code,
                                struct uinput_setup *arg1));
};

}  // namespace touch_keyboard

#endif  // TOUCH_KEYBOARD_SYSCALLHANDLER_MOCK_H_
