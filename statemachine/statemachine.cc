// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "touch_keyboard/statemachine/statemachine.h"

namespace mtstatemachine {

bool MtStateMachine::AddEvent(
    struct input_event const &ev,
    std::unordered_map<int, struct MtFinger> *out_snapshot) {
  // Here we process an event.  This function returns true at the end of a full
  // snapshot of the data (whenever there is a SYN event) and if you
  // pass it a pointer to an appropriate map, it will fill it with the
  // current state.  If you pass NULL, it will skip that step.
  EventKey key(ev);
  if (key.IsSlot()) {
    slot_ = ev.value;
  } else if (key.IsSyn()) {
    if (out_snapshot) {
      FillSnapshot(out_snapshot);
    }
    return true;
  } else if (ev.type == EV_ABS) {
    slots_[slot_][key] = ev.value;
  }
  return false;
}

void MtStateMachine::FillSnapshot(
    std::unordered_map<int, struct MtFinger> *out_snapshot) {
  out_snapshot->clear();

  for (int slot = 0; slot < kNumSlots; slot++) {
    int tid = slots_[slot].FindValueByEvent(EV_ABS, ABS_MT_TRACKING_ID);
    if (tid == -1) {
      continue;
    }
    struct MtFinger finger;
    finger.x = slots_[slot].FindValueByEvent(EV_ABS, ABS_MT_POSITION_X);
    finger.y = slots_[slot].FindValueByEvent(EV_ABS, ABS_MT_POSITION_Y);
    finger.p = slots_[slot].FindValueByEvent(EV_ABS, ABS_MT_PRESSURE);
    finger.touch_major = slots_[slot].FindValueByEvent(EV_ABS,
                                                       ABS_MT_TOUCH_MAJOR);
    (*out_snapshot)[tid] = finger;
  }
}

}  // namespace mtstatemachine
