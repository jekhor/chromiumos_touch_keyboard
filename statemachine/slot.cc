// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "statemachine/slot.h"

namespace mtstatemachine {

int Slot::FindValueByEvent(int ev_type, int ev_code) const {
  EventKey x_key(ev_type, ev_code);
  auto it = find(x_key);
  if (it != end()) {
    return it->second;
  }
  return kSlotMissingValue;
}

}  // namespace mtstatemachine
