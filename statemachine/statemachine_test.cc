// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "touch_keyboard/statemachine/statemachine.h"
#include <gtest/gtest.h>

namespace mtstatemachine {

// These values are used to test "finger" number 1.
constexpr int kTestSlot1 = 1;
constexpr int kTestTID1 = 11;
constexpr int kTestX1 = 101;
constexpr int kTestY1 = 201;
constexpr int kTestP1 = 301;

// These values are used to test "finger" number 1.
constexpr int kTestSlot2 = 2;
constexpr int kTestTID2 = 12;
constexpr int kTestX2 = 102;
constexpr int kTestY2 = 202;
constexpr int kTestP2 = 302;

class StateMachineTest : public ::testing::Test {
 protected:
  struct input_event CreateInputEvent(int type, int code, int value) const {
    // Create a dummy input_event struct to pass to an MtStateMachine.
    // Don't bother filling in the "time" field of the struct -- it's unused.
    struct input_event ev;
    ev.type = type;
    ev.code = code;
    ev.value = value;
    return ev;
  }
};

TEST_F(StateMachineTest, BasicOneFingerTest) {
  // Test to confirm that a single, well behaived finger is understood as it
  // arrives, moves, then leaves.
  MtStateMachine sm;
  std::unordered_map<int, struct MtFinger> snapshot;

  // The snapshot should start empty.  Confirm this before starting.
  EXPECT_TRUE(snapshot.empty());

  // Adding events shouldn't return true or alter the snapshot until the
  // whole batch is in and we send a SYN event.
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot1),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID,
                                            kTestTID1), &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_X, kTestX1),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_Y, kTestY1),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE, kTestP1),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());

  // Sending a SYN event should return true and populate the snapshot object.
  // The snapshot should match the values in the events we added to the
  // state machine earlier.  (ie: one finger with the above values)
  EXPECT_TRUE(sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot));
  EXPECT_FALSE(snapshot.empty());
  EXPECT_EQ(snapshot.size(), 1);
  EXPECT_NE(snapshot.find(kTestTID1), snapshot.end());
  EXPECT_EQ(snapshot[kTestTID1].x, kTestX1);
  EXPECT_EQ(snapshot[kTestTID1].y, kTestY1);
  EXPECT_EQ(snapshot[kTestTID1].p, kTestP1);

  // Now, we update the x value to simulate the finger moving 1 pixel right.
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_X,
                                            kTestX1 + 1), &snapshot));

  // A SYN event tells the state machine that all values have updated, and we
  // can check the contents of the snapshot again to make sure that X (and only
  // X) was updated.
  EXPECT_TRUE(sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot));
  EXPECT_FALSE(snapshot.empty());
  EXPECT_EQ(snapshot.size(), 1);
  EXPECT_NE(snapshot.find(kTestTID1), snapshot.end());
  EXPECT_EQ(snapshot[kTestTID1].x, kTestX1 + 1);
  EXPECT_EQ(snapshot[kTestTID1].y, kTestY1);
  EXPECT_EQ(snapshot[kTestTID1].p, kTestP1);

  // Now, make the finger leave by marking it's TID as invalid (-1).
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID, -1),
                           &snapshot));
  // After a SYN, we should see the snapshot emptied out.
  EXPECT_TRUE(sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot));
  EXPECT_TRUE(snapshot.empty());
}

TEST_F(StateMachineTest, BasicTwoFingerTest) {
  // This is a test to confirm that a two well behaived fingers are understood
  // as they arrive, move, then finally leave.
  MtStateMachine sm;
  std::unordered_map<int, struct MtFinger> snapshot;

  // The snapshot should start empty.  Confirm this before starting.
  EXPECT_TRUE(snapshot.empty());

  // Adding events shouldn't return true or alter the snapshot until the
  // whole batch is in and we send a SYN event.
  // Establish values for finger 1.
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot1),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID,
                                            kTestTID1), &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_X, kTestX1),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_Y, kTestY1),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE, kTestP1),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());

  // Establish values for finger 2.
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot2),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID,
                                            kTestTID2), &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_X, kTestX2),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_Y, kTestY2),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE, kTestP2),
                           &snapshot));
  EXPECT_TRUE(snapshot.empty());

  // Sending a SYN event should return true and populate the snapshot object
  // with data from both of the fingers.
  EXPECT_TRUE(sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot));
  EXPECT_FALSE(snapshot.empty());

  // The snapshot should match the values in the events we added to the
  // state machine earlier.  (ie: two fingers with the above values)
  EXPECT_EQ(snapshot.size(), 2);
  EXPECT_NE(snapshot.find(kTestTID1), snapshot.end());
  EXPECT_EQ(snapshot[kTestTID1].x, kTestX1);
  EXPECT_EQ(snapshot[kTestTID1].y, kTestY1);
  EXPECT_EQ(snapshot[kTestTID1].p, kTestP1);
  EXPECT_NE(snapshot.find(kTestTID2), snapshot.end());
  EXPECT_EQ(snapshot[kTestTID2].x, kTestX2);
  EXPECT_EQ(snapshot[kTestTID2].y, kTestY2);
  EXPECT_EQ(snapshot[kTestTID2].p, kTestP2);

  // Next, simulate them both moving a bit by updating a couple of their values.
  // Confirm that the values are updated in the snapshot after a SYN event.
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot1),
                           &snapshot));
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_Y,
                                            kTestY1 + 1), &snapshot));
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot2),
                           &snapshot));
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE,
                                            kTestP2 + 1), &snapshot));
  EXPECT_TRUE(sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot));
  EXPECT_EQ(snapshot.size(), 2);
  EXPECT_NE(snapshot.find(kTestTID1), snapshot.end());
  EXPECT_EQ(snapshot[kTestTID1].x, kTestX1);
  EXPECT_EQ(snapshot[kTestTID1].y, kTestY1 + 1);
  EXPECT_EQ(snapshot[kTestTID1].p, kTestP1);
  EXPECT_NE(snapshot.find(kTestTID2), snapshot.end());
  EXPECT_EQ(snapshot[kTestTID2].x, kTestX2);
  EXPECT_EQ(snapshot[kTestTID2].y, kTestY2);
  EXPECT_EQ(snapshot[kTestTID2].p, kTestP2 + 1);

  // Now mark each finger's tracking ID as -1 in turn -- indicating that the
  // finger has left checking that the state machine understands.
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot1),
                           &snapshot));
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID, -1),
                           &snapshot));
  EXPECT_TRUE(sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot));
  EXPECT_EQ(snapshot.size(), 1);
  EXPECT_EQ(snapshot.find(kTestTID1), snapshot.end());
  EXPECT_NE(snapshot.find(kTestTID2), snapshot.end());
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot2),
                           &snapshot));
  EXPECT_FALSE(sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID, -1),
                           &snapshot));
  EXPECT_TRUE(sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot));
  EXPECT_TRUE(snapshot.empty());
  EXPECT_EQ(snapshot.find(kTestTID1), snapshot.end());
  EXPECT_EQ(snapshot.find(kTestTID2), snapshot.end());
}

TEST_F(StateMachineTest, RepeatedUpdateTest) {
  // This tests how the state machine handles the situation where a single
  // value is updated repeatedly without SYNs between them.  This is an
  // unexpected behavior, but it is observed occasionally in the field with
  // poorly written touch firmware.
  MtStateMachine sm;
  std::unordered_map<int, struct MtFinger> snapshot;

  // Set up a finger with valid values in the state machine, but send two
  // different pressure values before the SYN and confirm that only the
  // last value sent is reported in the snapshot.
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID, kTestTID1),
              &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_X, kTestX1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_Y, kTestY1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE, kTestP1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE, kTestP2), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot);

  // The snapshot should include the finger, plus the latter pressure value
  // kTestP2 should have completely overwritten kTestP1.
  EXPECT_NE(snapshot.find(kTestTID1), snapshot.end());
  EXPECT_EQ(snapshot[kTestTID1].p, kTestP2);
}

TEST_F(StateMachineTest, DeltaCompressionTest) {
  // When a finger leaves, it's values remained stored in its slot -- so if
  // by chance the next finger that is assigned to that slot happens to have
  // the same value for something the kernel will delta-compress that event
  // away entirely.  This state machine needs to handle this case.
  MtStateMachine sm;
  std::unordered_map<int, struct MtFinger> snapshot;

  // Set up a finger with valid values in the state machine and issue a SYN.
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID, kTestTID1),
              &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_X, kTestX1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_Y, kTestY1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE, kTestP1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot);

  // Now show that finger leaving the pad by invalidating it's tracking ID.
  // The other values should remain unchanged in the internal state of its slot.
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID, -1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot);

  // When a new finger arrives, if it's assigned to the same slot, the old
  // values remain until they are overwritten, so we'll update all the values
  // for the new contact, except one and make sure the value that we skipped
  // retains the old value from the previous finger.
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_TRACKING_ID, kTestTID2),
              &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_Y, kTestY2), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE, kTestP2), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot);
  EXPECT_EQ(snapshot.find(kTestTID1), snapshot.end());
  EXPECT_NE(snapshot.find(kTestTID2), snapshot.end());
  EXPECT_EQ(snapshot[kTestTID2].x, kTestX1);
  EXPECT_EQ(snapshot[kTestTID2].y, kTestY2);
  EXPECT_EQ(snapshot[kTestTID2].p, kTestP2);
}

TEST_F(StateMachineTest, BadTrackingIdTest) {
  // Even if a finger has valid data, a bad tracking ID should indicate that
  // it's not a valid finger and should never be included in a snapshot.
  MtStateMachine sm;
  std::unordered_map<int, struct MtFinger> snapshot;

  // Set up a finger with valid values for everything but the tracking ID
  // in the state machine and issue a SYN.
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_SLOT, kTestSlot1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_X, kTestX1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_POSITION_Y, kTestY1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_ABS, ABS_MT_PRESSURE, kTestP1), &snapshot);
  sm.AddEvent(CreateInputEvent(EV_SYN, SYN_REPORT, 0), &snapshot);

  // The snapshot should be empty because we didn't add a tracking ID
  EXPECT_TRUE(snapshot.empty());
}

}  // namespace mtstatemachine
