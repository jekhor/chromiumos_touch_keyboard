// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "touch_keyboard/statemachine/eventkey.h"
#include <gtest/gtest.h>

namespace mtstatemachine {

class EventKeyTest : public ::testing::Test {};

TEST_F(EventKeyTest, ConstructorTest) {
  // This test performs a basic sanity check to confirm that the two
  // constructors work as expected.
  EventKey key1(EV_ABS, ABS_MT_POSITION_X);
  EXPECT_EQ(key1.type_, EV_ABS);
  EXPECT_EQ(key1.code_, ABS_MT_POSITION_X);

  struct input_event event;
  event.type = EV_KEY;
  event.code = KEY_BACKSPACE;
  EventKey key2(event);

  // Confirm the event passed to the constructor remains unchanged.
  EXPECT_EQ(event.type, EV_KEY);
  EXPECT_EQ(event.code, KEY_BACKSPACE);

  // Confirm that the EventKey has the right values.
  EXPECT_EQ(key2.type_, EV_KEY);
  EXPECT_EQ(key2.code_, KEY_BACKSPACE);
}

TEST_F(EventKeyTest, XPositionEventTest) {
  // This test performs a basic sanity check to confirm that the classifiers
  // work for x position events.
  EventKey abs_mt_pos_x(EV_ABS, ABS_MT_POSITION_X);
  EXPECT_TRUE(abs_mt_pos_x.IsX());

  EXPECT_FALSE(abs_mt_pos_x.IsSlot());
  EXPECT_FALSE(abs_mt_pos_x.IsSyn());
  EXPECT_FALSE(abs_mt_pos_x.IsTrackingID());
  EXPECT_FALSE(abs_mt_pos_x.IsY());
}

TEST_F(EventKeyTest, YPositionEventTest) {
  // This test performs a basic sanity check to confirm that the classifiers
  // work for y position events.
  EventKey abs_mt_pos_y(EV_ABS, ABS_MT_POSITION_Y);
  EXPECT_TRUE(abs_mt_pos_y.IsY());

  EXPECT_FALSE(abs_mt_pos_y.IsSlot());
  EXPECT_FALSE(abs_mt_pos_y.IsSyn());
  EXPECT_FALSE(abs_mt_pos_y.IsX());
  EXPECT_FALSE(abs_mt_pos_y.IsTrackingID());
}

TEST_F(EventKeyTest, TrackingIDEventTest) {
  // This test performs a basic sanity check to confirm that the classifiers
  // work for tracking ID events.
  EventKey abs_mt_tracking_id(EV_ABS, ABS_MT_TRACKING_ID);
  EXPECT_TRUE(abs_mt_tracking_id.IsTrackingID());

  EXPECT_FALSE(abs_mt_tracking_id.IsSlot());
  EXPECT_FALSE(abs_mt_tracking_id.IsSyn());
  EXPECT_FALSE(abs_mt_tracking_id.IsX());
  EXPECT_FALSE(abs_mt_tracking_id.IsY());
}

TEST_F(EventKeyTest, SlotEventTest) {
  // This test performs a basic sanity check to confirm that the classifiers
  // work for slot events.
  EventKey abs_mt_slot(EV_ABS, ABS_MT_SLOT);
  EXPECT_TRUE(abs_mt_slot.IsSlot());

  EXPECT_FALSE(abs_mt_slot.IsSyn());
  EXPECT_FALSE(abs_mt_slot.IsX());
  EXPECT_FALSE(abs_mt_slot.IsY());
  EXPECT_FALSE(abs_mt_slot.IsTrackingID());
}

TEST_F(EventKeyTest, SynEventTest) {
  // This test performs a basic sanity check to confirm that the classifiers
  // work for syn events.
  EventKey syn_report(EV_SYN, SYN_REPORT);
  EXPECT_TRUE(syn_report.IsSyn());

  EXPECT_FALSE(syn_report.IsSlot());
  EXPECT_FALSE(syn_report.IsX());
  EXPECT_FALSE(syn_report.IsY());
  EXPECT_FALSE(syn_report.IsTrackingID());
}

}  // namespace mtstatemachine
