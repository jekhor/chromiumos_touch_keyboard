// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <gtest/gtest.h>

#include "touch_keyboard/syscallhandler_mock.h"
#include "touch_keyboard/uinputdevice.h"

namespace touch_keyboard {

constexpr char kTestDeviceName[] = "test device name";
constexpr int kTestValidFD = 4;
constexpr int kTestX = 22;
constexpr int kTestY = 22;

using ::testing::_;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrEq;

class UinputDeviceTest : public ::testing::Test {};

TEST_F(UinputDeviceTest, UinputControlOpeningTest) {
  // This tests confirms that when you try to open a new uinput control file
  // descriptor it behaives correctly.
  MockSyscallHandler mock_syscall_handler;
  // Open() should be called once with these arguments only.  To simulate
  // success it is configured to return a valid file descriptor.
  EXPECT_CALL(mock_syscall_handler,
              open(StrEq(kUinputControlFilename), O_WRONLY | O_NONBLOCK))
      .WillOnce(Return(kTestValidFD));
  // The UI_DEV_DESTROY ioctl will be called in the device's destructor.
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_DEV_DESTROY)).WillOnce(Return(0));

  // Given that open() succeeds, this function should return true to indicate to
  // us that everything went as expected.
  UinputDevice dev(&mock_syscall_handler);
  EXPECT_TRUE(dev.CreateUinputFD());

  // Finally, check that the class stored the file descriptor correctly.
  EXPECT_EQ(dev.uinput_fd_, kTestValidFD);
}

TEST_F(UinputDeviceTest, UinputControlOpeningFailureTest) {
  // This tests confirms that when you try to open a new uinput control file
  // descriptor it correctly identifies failures.
  MockSyscallHandler mock_syscall_handler;
  // Open() should be called once with these arguments only. To simulate failure
  // we set it to return an error.
  EXPECT_CALL(mock_syscall_handler,
              open(StrEq(kUinputControlFilename), O_WRONLY | O_NONBLOCK))
      .WillOnce(Return(-ENOENT));

  // Given that we've set up open to fail, this function should return false
  // to indicate to us that something went awry.
  UinputDevice dev(&mock_syscall_handler);
  EXPECT_FALSE(dev.CreateUinputFD());
}

TEST_F(UinputDeviceTest, EnableEventTypeTest) {
  // This tests the ability to enable event types for the UinputDevice.  To do
  // so, we simulate enabling several types that both succeed and fail.
  int ev_types[] = {EV_ABS, EV_KEY, EV_SYN};
  int num_ev_types = sizeof(ev_types) / sizeof(*ev_types);

  MockSyscallHandler mock_syscall_handler;
  EXPECT_CALL(mock_syscall_handler, open(_, _)).WillOnce(Return(kTestValidFD));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_DEV_DESTROY)).WillOnce(Return(0));
  // Ioctl()s are used to enable an event type, so we expect this input and set
  // up all but the last one to succeed.
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_SET_EVBIT, Matcher<uint64_t>(_)))
      .WillOnce(Return(-EPERM));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_SET_EVBIT, Matcher<uint64_t>(_)))
      .Times(num_ev_types - 1).WillRepeatedly(Return(0)).RetiresOnSaturation();

  UinputDevice dev(&mock_syscall_handler);
  dev.CreateUinputFD();

  // Going through and enabling several event types should all work until the
  // last one, which should fail.
  for (int i = 0; i < num_ev_types - 1; i++) {
    EXPECT_TRUE(dev.EnableEventType(ev_types[i]));
  }
  EXPECT_FALSE(dev.EnableEventType(ev_types[num_ev_types - 1]));
}

TEST_F(UinputDeviceTest, EnableKeyEventTest) {
  // This tests the ability to enable key events for the UinputDevice.  To do
  // so, we simulate enabling events for several keys that succeed and fail.
  int key_codes[] = {KEY_ESC, KEY_BACKSPACE, BTN_TOUCH, BTN_TOOL_FINGER};
  int num_key_codes = sizeof(key_codes) / sizeof(*key_codes);

  MockSyscallHandler mock_syscall_handler;
  EXPECT_CALL(mock_syscall_handler, open(_, _)).WillOnce(Return(kTestValidFD));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_DEV_DESTROY)).WillOnce(Return(0));
  // Ioctl()s are used to enable a key event, so we expect this input.  Setting
  // up the first (num_key_codes - 1) attempts to succeed and the last one
  // will fail.
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_SET_KEYBIT, Matcher<uint64_t>(_)))
      .WillRepeatedly(Return(-EPERM));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_SET_KEYBIT, Matcher<uint64_t>(_)))
      .Times(num_key_codes - 1).WillRepeatedly(Return(0)).RetiresOnSaturation();

  UinputDevice dev(&mock_syscall_handler);
  dev.CreateUinputFD();

  // Going through and enabling several key events should work until the last
  // one, which should fail.
  for (int i = 0; i < num_key_codes - 1; i++) {
    EXPECT_TRUE(dev.EnableKeyEvent(key_codes[i]));
  }
  EXPECT_FALSE(dev.EnableKeyEvent(key_codes[num_key_codes - 1]));
}

TEST_F(UinputDeviceTest, EnableAbsEventTest) {
  // This tests the ability to enable abs events (like those used by touchpads
  // and touchscreens) for the UinputDevice.  To do so, we simulate enabling
  // everal kinds of abs events that both succeed and fail.
  int abs_codes[] = {ABS_PRESSURE, ABS_MT_TOUCH_MAJOR,
                     ABS_MT_POSITION_X, ABS_X};
  int num_abs_codes = sizeof(abs_codes) / sizeof(*abs_codes);

  MockSyscallHandler mock_syscall_handler;
  EXPECT_CALL(mock_syscall_handler, open(_, _)).WillOnce(Return(kTestValidFD));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_DEV_DESTROY)).WillOnce(Return(0));
  // Ioctls are used to enable an abs event, so we expect this input -- setting
  // up the first (num_abs_codes - 1) attempts to succeed and the last one
  // will fail.
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_SET_ABSBIT, Matcher<uint64_t>(_)))
      .WillRepeatedly(Return(-EPERM));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_SET_ABSBIT, Matcher<uint64_t>(_)))
      .Times(num_abs_codes - 1).WillRepeatedly(Return(0)).RetiresOnSaturation();

  UinputDevice dev(&mock_syscall_handler);
  dev.CreateUinputFD();

  // Going through and enabling several key events should work until the last
  // one, which should fail.
  for (int i = 0; i < num_abs_codes - 1; i++) {
    EXPECT_TRUE(dev.EnableAbsEvent(abs_codes[i]));
  }
  EXPECT_FALSE(dev.EnableAbsEvent(abs_codes[num_abs_codes - 1]));
}

TEST_F(UinputDeviceTest, FinalizeUinputCreationSuccessTest) {
  // This tests how the class finalized the creation of a uinput device.  This
  // would normally be the last setup step when making a uinput device and it
  // tells the kernel to actually create the device on disk.
  MockSyscallHandler mock_syscall_handler;
  // Since the order of the ioctls is essential, configure this test to only
  // accept them in the order I specify below.
  ::testing::InSequence seq;
  EXPECT_CALL(mock_syscall_handler, open(_, _)).WillOnce(Return(kTestValidFD));
  EXPECT_CALL(mock_syscall_handler, ioctl(kTestValidFD, UI_DEV_SETUP,
                                          Matcher<struct uinput_setup*>(_)))
      .WillOnce(Return(0));
  EXPECT_CALL(mock_syscall_handler, ioctl(kTestValidFD, UI_DEV_CREATE))
      .WillOnce(Return(0));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_DEV_DESTROY)).WillOnce(Return(0));

  UinputDevice dev(&mock_syscall_handler);
  dev.CreateUinputFD();
  EXPECT_TRUE(dev.FinalizeUinputCreation(kTestDeviceName));
}

TEST_F(UinputDeviceTest, FinalizeUinputCreationDEV_SETUPFailTest) {
  // This tests how FinalizeUinputCreation handles a failure while trying to
  // setup the device details with the UI_DEV_SETUP ioctl().
  MockSyscallHandler mock_syscall_handler;
  // Since the order of the ioctls is essential, configure this test to only
  // accept them in the order specified below.
  ::testing::InSequence seq;
  EXPECT_CALL(mock_syscall_handler, open(_, _)).WillOnce(Return(kTestValidFD));
  EXPECT_CALL(mock_syscall_handler, ioctl(kTestValidFD, UI_DEV_SETUP,
                                          Matcher<struct uinput_setup*>(_)))
      .WillOnce(Return(-EPERM));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_DEV_DESTROY)).WillOnce(Return(0));

  UinputDevice dev(&mock_syscall_handler);
  dev.CreateUinputFD();
  EXPECT_FALSE(dev.FinalizeUinputCreation(kTestDeviceName));
}

TEST_F(UinputDeviceTest, FinalizeUinputCreationDEV_CREATEFailTest) {
  // This tests how FinalizeUinputCreation handles a failure while trying to
  // tell the kernel to the create device details with the UI_DEV_CREATE ioctl
  MockSyscallHandler mock_syscall_handler;
  // Since the order of the ioctls is essential, configure this test to only
  // accept them in the order I specify below.
  ::testing::InSequence seq;
  EXPECT_CALL(mock_syscall_handler, open(_, _)).WillOnce(Return(kTestValidFD));
  EXPECT_CALL(mock_syscall_handler, ioctl(kTestValidFD, UI_DEV_SETUP,
                                          Matcher<struct uinput_setup*>(_)))
      .WillOnce(Return(0));
  EXPECT_CALL(mock_syscall_handler, ioctl(kTestValidFD, UI_DEV_CREATE))
      .WillOnce(Return(-EPERM));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_DEV_DESTROY)).WillOnce(Return(0));

  UinputDevice dev(&mock_syscall_handler);
  dev.CreateUinputFD();
  EXPECT_FALSE(dev.FinalizeUinputCreation(kTestDeviceName));
}

TEST_F(UinputDeviceTest, SendEventTest) {
  // This test makes sure that sending events works as expected.  They should
  // be send using the write() command, so this tests how the class handles
  // success and failure of that write.
  MockSyscallHandler mock_syscall_handler;
  // Set up open() to work as normal so the class can set itself up.
  EXPECT_CALL(mock_syscall_handler, open(_, _)).WillOnce(Return(kTestValidFD));
  EXPECT_CALL(mock_syscall_handler,
              ioctl(kTestValidFD, UI_DEV_DESTROY)).WillOnce(Return(0));
  // The first call to write an event to the file descriptor should succeed by
  // returning the full size of the struct, but after that it is configured to
  // return an error on subsequent calls.
  EXPECT_CALL(mock_syscall_handler,
              write(kTestValidFD, _, sizeof(struct input_event)))
      .WillRepeatedly(Return(-EPERM));
  EXPECT_CALL(mock_syscall_handler,
              write(kTestValidFD, _, sizeof(struct input_event)))
      .WillOnce(Return(sizeof(struct input_event))).RetiresOnSaturation();

  UinputDevice dev(&mock_syscall_handler);
  dev.CreateUinputFD();

  // Given how write was configured for this test, only the first event should
  // send correctly.
  EXPECT_TRUE(dev.SendEvent(EV_ABS, ABS_MT_POSITION_X, kTestX));
  EXPECT_FALSE(dev.SendEvent(EV_ABS, ABS_MT_POSITION_X, kTestX));
  EXPECT_FALSE(dev.SendEvent(EV_ABS, ABS_MT_POSITION_Y, kTestY));
}

}  // namespace touch_keyboard
