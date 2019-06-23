// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_STATEMACHINE_EVENTKEY_H_
#define TOUCH_KEYBOARD_STATEMACHINE_EVENTKEY_H_

#include <functional>
#include <linux/input.h>
#include <stdio.h>

namespace mtstatemachine {

class EventKey {
 /* An Event Key to correspond to the various values a multitouch device
  * can report.  (x position, y position, pressure, etc...)
  *
  * In the kernel, each event is specified as a pair of integers: one
  * for the event type (key, absolute position, relative position, etc..) and
  * one for the specific event code (which key it is, which axis movement is
  * on, etc..).  This class represents both of those as a cohesive groups and
  * offers several convenience functions for classifying these events.
  */
 public:
  EventKey(int type, int code): type_(type), code_(code) {}
  explicit EventKey(struct input_event const& ev):
      type_(ev.type), code_(ev.code) {}

  // Define equality so we can use EventKeys as an std::unordered_map key.
  bool operator==(const EventKey& other) const;

  // These Is*() functions identify various types of Events.
  bool IsSyn() const;
  bool IsSlot() const;
  bool IsTrackingID() const;
  bool IsX() const;
  bool IsY() const;

  // Use this function to check to see if this Event is supported for touchpads.
  bool IsSupportedForTouchpads() const;

  int type_, code_;
};

struct EventKeyHasher {
  // This defines a functor used to Hash EventKeys so they can be an
  // std::unordered_map key.  Unordered Maps require a hash function for
  // whatever datatypes they use as keys.
  size_t operator()(const EventKey& key) const {
    return (std::hash<int>()(key.type_) ^
            (std::hash<int>()(key.code_) << 1) >> 1);
  }
};

}  // namespace mtstatemachine

#endif  // TOUCH_KEYBOARD_STATEMACHINE_EVENTKEY_H_
