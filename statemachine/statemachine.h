// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_STATEMACHINE_STATEMACHINE_H_
#define TOUCH_KEYBOARD_STATEMACHINE_STATEMACHINE_H_

#include <linux/input.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>

#include "statemachine/slot.h"

namespace mtstatemachine {

constexpr int kNumSlots = 10;

// The information reported by a state machine for a single finger
struct MtFinger {
  int x, y;
  int p;
  int touch_major;
};

class MtStateMachine {
 /* Multi-touch State Machine
  *
  * This class implements a multitouch state machine and  interprets the
  * individual touch kernel events in a manageable way.  To use a state machine
  * instantiate a new MtStateMachine object at the beginning of your program
  * and then continually call AddEvent() to pass the new touch kernel events
  * for processing.  AddEvent() will return 'true' at the end of an entire
  * frame (SYN event) and then the snapshot map you passed will be populated
  * and ready for use.  The snapshot is a mapping from tracking IDs to finger
  * data at the time of the SYN event.
  */
 public:
  MtStateMachine(): slot_(0) {}

  // Consume an input event and update the internal state.  If this was a SYN
  // (which means it's the end of a full update) populate out_snapshot with
  // the current state and return true.  Otherwise leave out_snapshot unchanged
  // and return false to indicate there is still more events on the way.
  bool AddEvent(struct input_event const &ev,
                std::unordered_map<int, struct MtFinger> *out_snapshot);

  int slot_;
  Slot slots_[kNumSlots];

 private:
  // Populate out_snapshot with the current state of the MtStateMachine.
  void FillSnapshot(std::unordered_map<int, struct MtFinger> *out_snapshot);
};

}  // namespace mtstatemachine

#endif  // TOUCH_KEYBOARD_STATEMACHINE_STATEMACHINE_H_
