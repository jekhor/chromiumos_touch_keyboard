// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_STATEMACHINE_SLOT_H_
#define TOUCH_KEYBOARD_STATEMACHINE_SLOT_H_

#include <linux/input.h>
#include <unordered_map>
#include <stdio.h>
#include <vector>

#include "statemachine/eventkey.h"

namespace mtstatemachine {

constexpr int kSlotMissingValue = -1;

class Slot : public std::unordered_map<EventKey, int, EventKeyHasher> {
 /* Storage for all the event data for a single multitouch slot
  *
  * Multitouch works by filling "slots" with key-value pairs.  Each slot
  * corresponds with a single finger and is incrementally updated until a SYN
  * event is finally sent, indicating that the slot has been fully updated and
  * the current values are valid.  This class implements a slot as a mapping
  * from event keys to their values.  Slots are used by MtStateMachine to
  * store all the touch information it receives.
  */
 public:
  // Scan through the slot's fields for one that matches this event type and
  // code.  If a match is found, return it. Otherwise return kSlotMissingValue.
  int FindValueByEvent(int ev_type, int ev_code) const;
};

}  // namespace mtstatemachine

#endif  // TOUCH_KEYBOARD_STATEMACHINE_SLOT_H_
