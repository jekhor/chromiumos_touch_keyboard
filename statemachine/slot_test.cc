// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "touch_keyboard/statemachine/slot.h"
#include <gtest/gtest.h>

namespace mtstatemachine {

constexpr int kTestPressure = 58;
constexpr int kTestTID = 3;
constexpr int kTestX = 100;
constexpr int kTestY = 200;

class SlotTest : public ::testing::Test {
 protected:
  Slot BuildTestSlot() {
    Slot test_slot;
    test_slot[EventKey(EV_ABS, ABS_MT_TRACKING_ID)] = kTestTID;
    test_slot[EventKey(EV_ABS, ABS_MT_POSITION_X)] = kTestX;
    test_slot[EventKey(EV_ABS, ABS_MT_POSITION_Y)] = kTestY;
    test_slot[EventKey(EV_ABS, ABS_MT_PRESSURE)] = kTestPressure;
    return test_slot;
  }
};

TEST_F(SlotTest, ValueLookupTest) {
  // Check to make sure that the FindValueByEvent function works by looking
  // up the values in a test Slot object.
  Slot test_slot = BuildTestSlot();
  EXPECT_EQ(test_slot.FindValueByEvent(EV_ABS, ABS_MT_TRACKING_ID), kTestTID);
  EXPECT_EQ(test_slot.FindValueByEvent(EV_ABS, ABS_MT_POSITION_X), kTestX);
  EXPECT_EQ(test_slot.FindValueByEvent(EV_ABS, ABS_MT_POSITION_Y), kTestY);
  EXPECT_EQ(test_slot.FindValueByEvent(EV_ABS, ABS_MT_PRESSURE), kTestPressure);
}

TEST_F(SlotTest, MissingValueTest) {
  // Confirm that looking up a nonexistent value behaves smoothly.  They
  // should return kSlotMissingValue if the slot is missing the requested
  // value.
  Slot test_slot = BuildTestSlot();
  EXPECT_EQ(test_slot.FindValueByEvent(EV_ABS, ABS_MT_TOUCH_MAJOR),
              kSlotMissingValue);
}

}  // namespace mtstatemachine
