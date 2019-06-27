// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "touch_keyboard/faketouchpad.h"

namespace touch_keyboard {

FakeTouchpad::FakeTouchpad(double xmin_mm, double xmax_mm,
                           double ymin_mm, double ymax_mm,
                           struct hw_config &hw_config) :
  hw_config_(hw_config) {

  double hw_pitch_x = hw_config_.res_x / hw_config_.width_mm;
  double hw_pitch_y = hw_config_.res_y / hw_config_.height_mm;
  double left_margin, top_margin;

  left_margin = hw_config_.left_margin_mm;
  top_margin = hw_config_.top_margin_mm;
  
  switch (hw_config_.rotation) {
    case 0:
      xmin_ = (xmin_mm + left_margin) * hw_pitch_x;
      xmax_ = (xmax_mm + left_margin) * hw_pitch_x;
      ymin_ = (ymin_mm + top_margin) * hw_pitch_y;
      ymax_ = (ymax_mm + top_margin) * hw_pitch_y;
      break;
    case 90:
      xmin_ = (top_margin + ymin_mm) * hw_pitch_x;
      xmax_ = (top_margin + ymax_mm) * hw_pitch_x;
      ymin_ = (hw_config_.height_mm - (left_margin + xmax_mm)) * hw_pitch_y;
      ymax_ = (hw_config_.height_mm - (left_margin + xmin_mm)) * hw_pitch_y;
      break;
    case 180:
      xmin_ = (hw_config_.width_mm - (xmax_mm + left_margin)) * hw_pitch_x;
      xmax_ = (hw_config_.width_mm - (xmin_mm + left_margin)) * hw_pitch_x;
      ymin_ = (hw_config_.height_mm - (ymax_mm + top_margin)) * hw_pitch_y;
      ymax_ = (hw_config_.height_mm - (ymin_mm + top_margin)) * hw_pitch_y;
      break;
    case 270:
      xmin_ = (hw_config_.width_mm - (ymax_mm + top_margin)) * hw_pitch_x;
      xmax_ = (hw_config_.width_mm - (ymin_mm + top_margin)) * hw_pitch_x;
      ymin_ = (left_margin + xmin_mm) * hw_pitch_y;
      ymax_ = (left_margin + xmax_mm) * hw_pitch_y;
      break;
    default:
      LOG(ERROR) << "Invalid rotation value: " << hw_config_.rotation;
      throw;
  }

  LOG(INFO) << "FakeTouchpad geometry: (" << xmin_ << ", " << xmax_ <<
                                    "), (" << ymin_ << ", " << ymax_ << ")";
  for (int i = 0; i < mtstatemachine::kNumSlots; i++) {
    slot_memberships_.push_back(false);
  }
}

void FakeTouchpad::Start(std::string const &source_device_path,
                         std::string const &touchpad_device_name) {
  if (!OpenSourceDevice(source_device_path))
    return;
  CreateUinputFD();

  // Enable the few button events that touchpads need.
  EnableEventType(EV_KEY);
  EnableKeyEvent(BTN_TOUCH);
  EnableKeyEvent(BTN_TOOL_FINGER);
  EnableKeyEvent(BTN_TOOL_DOUBLETAP);
  EnableKeyEvent(BTN_TOOL_TRIPLETAP);
  EnableKeyEvent(BTN_TOOL_QUADTAP);

  int w, h;

  switch (hw_config_.rotation) {
    case 0:
    case 180:
      w = xmax_ - xmin_;
      h = ymax_ - ymin_;
      break;
    case 90:
    case 270:
      w = ymax_ - ymin_;
      h = xmax_ - xmin_;
      break;
    default:
      break;
  }

  // Duplicate the ABS events from the source device.
  CopyABSOutputEvents(source_fd_, w, h);

  // Finally, tell kernel to create the new fake touchpad's uinput device.
  FinalizeUinputCreation(touchpad_device_name);

  // Loop forever consuming the events coming in from the source device.
  Consume();
}

void FakeTouchpad::Consume() {
  while (1) {
    struct input_event ev;
    bool event_ready = GetNextEvent(kNoTimeout, &ev);
    // If we timed out waiting, then there is no event yet.
    if (!event_ready) {
      continue;
    }

    if (sm_.AddEvent(ev, NULL)) {
      // Sync over all the touch events from the source state machine.
      int touch_count = SyncTouchEvents();
      // Make sure the BTN events are correct since this is a fake touchpad.
      SendTouchpadBtnEvents(touch_count);
      // Finally send a SYN after all applicable events are sent.
      SendEvent(EV_SYN, SYN_REPORT, 0);
    }
  }
}

void FakeTouchpad::SendTouchpadBtnEvents(int touch_count) const {
  // Since this is a fake touchpad, we need to send BTN_TOUCH and BTN_TOOL_*
  // events whenever a finger arrives and leaves for the gesture library to
  // interpret the motions correctly.  This function generates those events
  // based on the number of fingers currently being reported by the fake
  // touchpad.
  SendEvent(EV_KEY, BTN_TOUCH, (touch_count > 0) ? 1 : 0);
  SendEvent(EV_KEY, BTN_TOOL_FINGER, (touch_count == 1) ? 1 : 0);
  SendEvent(EV_KEY, BTN_TOOL_DOUBLETAP, (touch_count == 2) ? 1 : 0);
  SendEvent(EV_KEY, BTN_TOOL_TRIPLETAP, (touch_count == 3) ? 1 : 0);
  SendEvent(EV_KEY, BTN_TOOL_QUADTAP, (touch_count == 4) ? 1 : 0);
}

bool FakeTouchpad::Contains(mtstatemachine::Slot const &slot) const {
  // Check and see if the contact in the slot is currently contained within
  // the boundaries of this region.
  int x = slot.FindValueByEvent(EV_ABS, ABS_MT_POSITION_X);
  int y = slot.FindValueByEvent(EV_ABS, ABS_MT_POSITION_Y);
  if (x < xmin_ || x > xmax_)
    return false;
  if (y < ymin_ || y > ymax_)
    return false;
  return true;
}

bool FakeTouchpad::PassEventsThrough(mtstatemachine::Slot const &slot) const {
  // Go through the slot in question and send events setting each of the set
  // values into this region.  Essentially this updates all of the values for
  // this slot in the kernel to match our internal version.
  // This function returns True if the updated slot represented a contact that
  // is current on the touchpad.
  mtstatemachine::Slot::const_iterator it;
  bool is_valid = false;

  // Iterate over each value in the slot and send the corresponding event.
  for (it = slot.begin(); it != slot.end(); it++) {
    mtstatemachine::EventKey slot_event_key = it->first;
    int value = it->second;
    int code = slot_event_key.code_;

    // If the value is for an unsupported event_key, skip it.
    if (!slot_event_key.IsSupportedForTouchpads())
      continue;

    // Check the tracking ID. (-1 indicates this contact is not valid anymore)
    if (slot_event_key.IsTrackingID()) {
      is_valid |= (value != -1);
    }

    // Transform X and Y values to keep the corner of the region 0,0 and
    // invert any axes that were set to be inverted at creation.
    if (slot_event_key.IsX()) {
      value -= xmin_;

      switch (hw_config_.rotation) {
        case 0:
          break; // no transform
        case 90:
          code = ABS_MT_POSITION_Y;
          break;
        case 180:
          value = (xmax_ - xmin_) - value;
          break;
        case 270:
          code = ABS_MT_POSITION_Y;
          value = (xmax_ - xmin_) - value;
          break;
      }
    } else if (slot_event_key.IsY()) {
      value -= ymin_;
      switch (hw_config_.rotation) {
        case 0:
          break; // no transform
        case 90:
          code = ABS_MT_POSITION_X;
          value = (ymax_ - ymin_) - value;
          break;
        case 180:
          value = (ymax_ - ymin_) - value;
          break;
        case 270:
          code = ABS_MT_POSITION_X;
          break;
      }
    }

    // Push an event that sets this value into the region.
    SendEvent(slot_event_key.type_, code, value);
  }

  return is_valid;
}

int FakeTouchpad::SyncTouchEvents() {
  // Scan through all the slots of the state machine and sync the
  // uinput device with it by copying over the touch events for any contacts
  // that are currently contained within the region, returning the number
  // of such contacts.  This only passes on events that are contained within
  // the region and performs transformations on the coordinates to maintain
  // the illusion of a different device (shifting x/y, adding fake finger
  // arriving events, etc)
  int touch_count = 0;

  for (int slot = 0; slot < mtstatemachine::kNumSlots; slot++) {
    // Send a SLOT message to make sure these events go to the right slot
    SendEvent(EV_ABS, ABS_MT_SLOT, slot);

    // Don't pass on events from contacts outside of the region.
    if (!Contains(sm_.slots_[slot])) {
      // If this slot just left the region, send a finger-leaving event.
      if (slot_memberships_[slot]) {
        SendEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);
      }
      slot_memberships_[slot] = false;
      continue;
    } else {
      // If this slot just entered the region, send a finger-arrive event.
      if (!slot_memberships_[slot]) {
        int tid = sm_.slots_[slot].FindValueByEvent(EV_ABS,
                                                    ABS_MT_TRACKING_ID);
        SendEvent(EV_ABS, ABS_MT_TRACKING_ID, tid);
      }
      slot_memberships_[slot] = true;
    }

    // Scan through the slot and update all the properties.
    bool valid_finger = PassEventsThrough(sm_.slots_[slot]);
    if (valid_finger && slot_memberships_[slot]) {
      touch_count++;
    }
  }

  return touch_count;
}

}  // namespace touch_keyboard
