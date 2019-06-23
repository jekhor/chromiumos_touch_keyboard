// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <errno.h>
#include <gtest/gtest.h>

#include "touch_keyboard/evdevsource.h"
#include "touch_keyboard/syscallhandler_mock.h"

namespace touch_keyboard {

using ::testing::Return;
using ::testing::_;

constexpr char kTestFilepath[] = "/dev/input/event3";
constexpr int kNumRepeatedTestEvents = 20;
constexpr int kTestTimeoutLenMS = 50;
constexpr int kTestValidFD = 4;

class EvdevSourceTest : public ::testing::Test {};

TEST_F(EvdevSourceTest, OpenDeviceSucceedTest) {
  // This test confirms that opening a device on disk works as expected and
  // the function returns true on success.
  MockSyscallHandler mock_syscall_handler;
  EXPECT_CALL(mock_syscall_handler, open(_, O_RDONLY))
      .WillRepeatedly(Return(kTestValidFD));
  EvdevSource source(&mock_syscall_handler);
  EXPECT_TRUE(source.OpenSourceDevice(kTestFilepath));
}

TEST_F(EvdevSourceTest, OpenDeviceFailureTest) {
  // This test confirms that when the program doesn't have access to the device
  // on disk the opening function correctly identifies the failure.
  MockSyscallHandler mock_syscall_handler;
  EXPECT_CALL(mock_syscall_handler, open(_, O_RDONLY))
      .WillRepeatedly(Return(-EPERM));
  EvdevSource source(&mock_syscall_handler);
  EXPECT_FALSE(source.OpenSourceDevice(kTestFilepath));
}

TEST_F(EvdevSourceTest, EventCaptureTimeoutDisabledTest) {
  // This test repeatedly exercises the most basic path to capturing an event
  // from an EvdevSource.  This means we use no timeout and everything succeeds
  // several times in a row to simulate a stream of events.
  MockSyscallHandler mock_syscall_handler;
  EXPECT_CALL(mock_syscall_handler, open(_, O_RDONLY))
      .WillOnce(Return(kTestValidFD));
  // Read() should be called once when we try to get a new event.  Note that
  // select is not expected to be used at all, since there is no timeout.
  EXPECT_CALL(mock_syscall_handler,
              read(kTestValidFD, _, sizeof(struct input_event)))
      .Times(kNumRepeatedTestEvents)
      .WillRepeatedly(Return(sizeof(struct input_event)));

  // Open the device.
  EvdevSource source(&mock_syscall_handler);
  EXPECT_TRUE(source.OpenSourceDevice(kTestFilepath));

  // Get an event from the device repeatedly with no timeout enabled at all.
  struct input_event ev;
  for (int i = 0; i < kNumRepeatedTestEvents; i++) {
    EXPECT_TRUE(source.GetNextEvent(kNoTimeout, &ev));
  }
}


TEST_F(EvdevSourceTest, EventCaptureTimeoutEnabledTest) {
  // This test repeatedly exercises the typical path to capturing an event
  // from an EvdevSource.  This means using a timeout, but everything succeeds
  // several times in a row to simulate a stream of events.  For this test
  // select always finds an event to read before the timeout has completed.
  MockSyscallHandler mock_syscall_handler;
  EXPECT_CALL(mock_syscall_handler, open(_, O_RDONLY))
      .WillOnce(Return(kTestValidFD));
  // Select() is used to implement a timeout before each read.  For this test
  // it should always return 1 to indicate that there is an event to read.
  EXPECT_CALL(mock_syscall_handler, select(kTestValidFD + 1, _, NULL, NULL, _))
      .Times(kNumRepeatedTestEvents) .WillRepeatedly(Return(1));
  // Read() should be called once when we try to get a new event.
  EXPECT_CALL(mock_syscall_handler,
              read(kTestValidFD, _, sizeof(struct input_event)))
      .Times(kNumRepeatedTestEvents)
      .WillRepeatedly(Return(sizeof(struct input_event)));

  // Open the device.
  EvdevSource source(&mock_syscall_handler);
  EXPECT_TRUE(source.OpenSourceDevice(kTestFilepath));

  // Get an event from the device repeatedly with a timeout enabled.
  struct input_event ev;
  for (int i = 0; i < kNumRepeatedTestEvents; i++) {
    EXPECT_TRUE(source.GetNextEvent(kTestTimeoutLenMS, &ev));
  }
}

TEST_F(EvdevSourceTest, EventCaptureTimesOutTest) {
  // This tests how an EvdevSource handles a timeout occurring before there
  // is an event ready to capture.
  MockSyscallHandler mock_syscall_handler;
  EXPECT_CALL(mock_syscall_handler, open(_, O_RDONLY))
      .WillOnce(Return(kTestValidFD));
  // Select() is used to implement a timeout before using read.  For this test
  // it should return 0 to indicate that it timed out waiting to read.
  // read should never be called in this test since there is no data to read
  // after a timeout.
  EXPECT_CALL(mock_syscall_handler, select(kTestValidFD + 1, _, NULL, NULL, _))
      .WillOnce(Return(0));

  // Open the device.
  EvdevSource source(&mock_syscall_handler);
  EXPECT_TRUE(source.OpenSourceDevice(kTestFilepath));

  // Try to get an event from the device, but it should fail as it would
  // in a timeout and return false to alert us that it failed.
  struct input_event ev;
  EXPECT_FALSE(source.GetNextEvent(kTestTimeoutLenMS, &ev));
}

}  // namespace touch_keyboard
