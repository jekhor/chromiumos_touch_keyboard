// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "statemachine/eventkey.h"

namespace mtstatemachine {

bool EventKey::operator==(const EventKey& other) const {
  return (type_ == other.type_ && code_ == other.code_);
}

bool EventKey::IsSyn() const {
  return (type_ == EV_SYN && code_ == SYN_REPORT);
}

bool EventKey::IsSlot() const {
  return (type_ == EV_ABS && code_ == ABS_MT_SLOT);
}

bool EventKey::IsTrackingID() const {
  return (type_ == EV_ABS && code_ == ABS_MT_TRACKING_ID);
}

bool EventKey::IsX() const {
  return (type_ == EV_ABS && code_ == ABS_MT_POSITION_X);
}

bool EventKey::IsY() const {
  return (type_ == EV_ABS && code_ == ABS_MT_POSITION_Y);
}

bool EventKey::IsSupportedForTouchpads() const {
  if (type_ != EV_ABS)
    return false;
  return (code_ == ABS_MT_TRACKING_ID ||
          code_ == ABS_MT_PRESSURE ||
          code_ == ABS_MT_POSITION_X ||
          code_ == ABS_MT_POSITION_Y ||
          code_ == ABS_MT_TOUCH_MAJOR ||
          code_ == ABS_MT_TOUCH_MINOR);
}

}  // namespace mtstatemachine
