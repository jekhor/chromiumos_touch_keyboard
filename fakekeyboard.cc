// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "touch_keyboard/fakekeyboard.h"

namespace touch_keyboard {

// This sets how long (in ms) to delay a key down event before sending it.  More
// delay causes a visible lag when typing and a shorter delay means the system
// has less time to confirm keypresses and may increase the error rate.
constexpr int kEventDelayMS = 50;

// These dummy values are used to track events.
constexpr bool kKeyDownEvent = true;
constexpr bool kKeyUpEvent = false;
constexpr int kNoKey = -1;
constexpr int kOldTID = -999;

// Below are the dimensions of the Pbody touch keyboard and the key locations.
// TODO(charliemooney): These are hardcoded constants used in SetUpLayout and
// should be removed once that function is upgraded to load a keymap from disk.
constexpr int kInputWidth = 2702;
constexpr int kInputHeight = 1715;
constexpr int kWidthMM = 266;
constexpr int kHeightMM = 168;
constexpr float kWidthPitch = kInputWidth / static_cast<float>(kWidthMM);
constexpr float kHeightPitch = kInputHeight / static_cast<float>(kHeightMM);
constexpr float kKeyWidthMM = 18.5;
constexpr float kKeyHeightMM = 19.0;
constexpr float kKeyWidth = kKeyWidthMM * kWidthPitch;
constexpr float kKeyHeight = kKeyHeightMM * kHeightPitch;
constexpr int kBottomRowYmin = 630;
constexpr float kFnKeyWidth = 22.0 * kWidthPitch;
constexpr float kFnKeyHeight = 12.0 * kHeightPitch;
constexpr float kArrowKeyHeight = kKeyHeight / 2.0;
constexpr float kWideArrowKeyWidth = 23.0 * kWidthPitch;
constexpr float kLeftCtrlWidth = 42.0 * kWidthPitch;
constexpr float kLeftAltWidth = 37.0 * kWidthPitch;
constexpr float kSpaceWidth = 93.0 * kWidthPitch;
constexpr float kShiftWidth = 41.0 * kWidthPitch;
constexpr float kSearchWidth = 32.0 * kWidthPitch;
constexpr float kEnterWidth = 32.0 * kWidthPitch;
constexpr float kTabWidth = 27.0 * kWidthPitch;
constexpr float kBackspaceWidth = 27.0 * kWidthPitch;

constexpr int kMinTapPressure = 50;
constexpr int kMaxTapPressure = 110;

FakeKeyboard::FakeKeyboard() : ff_manager_(kInputWidth) {
  SetUpLayout();
}

void FakeKeyboard::SetUpLayout() {
  // This function sets up the key code and their locations.
  // TODO(charliemooney): Currently this has a layout hard-coded into it.
  //     Obviously, that's a bad idea.  This needs to be updated to load
  //     a layout from disk somewhere once things have stabilized a little.
  unsigned int i;
  int row1[] = {KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA,
                KEY_DOT, KEY_SLASH};
  int row2[] = {KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L,
                KEY_SEMICOLON, KEY_APOSTROPHE};
  int row3[] = {KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I,
                KEY_O, KEY_P, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_BACKSLASH};
  int row4[] = {KEY_GRAVE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7,
                KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL};
  int rowfn[] = {KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
                 KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F13};


  int bottom_row_ymin = kBottomRowYmin;
  layout_.push_back(Key(KEY_LEFTCTRL,
                        0,
                        kLeftCtrlWidth,
                        bottom_row_ymin,
                        bottom_row_ymin + kKeyHeight));
  layout_.push_back(Key(KEY_LEFTALT,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kLeftAltWidth,
                        bottom_row_ymin,
                        bottom_row_ymin + kKeyHeight));
  layout_.push_back(Key(KEY_SPACE,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kSpaceWidth,
                        bottom_row_ymin,
                        bottom_row_ymin + kKeyHeight));
  layout_.push_back(Key(KEY_RIGHTALT,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kKeyWidth,
                        bottom_row_ymin,
                        bottom_row_ymin + kKeyHeight));
  layout_.push_back(Key(KEY_RIGHTCTRL,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kKeyWidth,
                        bottom_row_ymin,
                        bottom_row_ymin + kKeyHeight));
  layout_.push_back(Key(KEY_LEFT,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kKeyWidth,
                        bottom_row_ymin,
                        bottom_row_ymin + kArrowKeyHeight));
  layout_.push_back(Key(KEY_DOWN,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kWideArrowKeyWidth,
                        bottom_row_ymin,
                        bottom_row_ymin + kArrowKeyHeight));
  layout_.push_back(Key(KEY_UP,
                        layout_.back().xmin_,
                        layout_.back().xmax_,
                        bottom_row_ymin + kArrowKeyHeight,
                        bottom_row_ymin + kKeyWidth));
  layout_.push_back(Key(KEY_RIGHT,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kKeyWidth,
                        bottom_row_ymin,
                        bottom_row_ymin + kArrowKeyHeight));


  int row1_ymin = bottom_row_ymin + kKeyHeight;
  layout_.push_back(Key(KEY_LEFTSHIFT,
                        0,
                        kShiftWidth,
                        row1_ymin,
                        row1_ymin + kKeyHeight));
  for (i = 0; i < arraysize(row1); i++) {
    layout_.push_back(Key(row1[i],
                          kShiftWidth + i * kKeyWidth,
                          kShiftWidth + i * kKeyWidth + kKeyWidth,
                          row1_ymin,
                          row1_ymin + kKeyHeight));
  }
  layout_.push_back(Key(KEY_RIGHTSHIFT,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kShiftWidth,
                        row1_ymin,
                        row1_ymin + kKeyHeight));

  int row2_ymin = row1_ymin + kKeyHeight;
  layout_.push_back(Key(KEY_LEFTMETA,
                        0,
                        kSearchWidth,
                        row2_ymin,
                        row2_ymin + kKeyHeight));
  for (i = 0; i < arraysize(row2); i++) {
    layout_.push_back(Key(row2[i],
                          kSearchWidth + i * kKeyWidth,
                          kSearchWidth + i * kKeyWidth + kKeyWidth,
                          row2_ymin,
                          row2_ymin + kKeyHeight));
  }
  layout_.push_back(Key(KEY_ENTER,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kEnterWidth,
                        row2_ymin,
                        row2_ymin + kKeyHeight));

  int row3_ymin = row2_ymin + kKeyHeight;
  layout_.push_back(Key(KEY_TAB,
                        0,
                        kTabWidth,
                        row3_ymin,
                        row3_ymin + kKeyHeight));
  for (i = 0; i < arraysize(row3); i++) {
    layout_.push_back(Key(row3[i],
                          kTabWidth + i * kKeyWidth,
                          kTabWidth + i * kKeyWidth + kKeyWidth,
                          row3_ymin,
                          row3_ymin + kKeyHeight));
  }

  int row4_ymin = row3_ymin + kKeyHeight;
  for (i = 0; i < arraysize(row4); i++) {
    layout_.push_back(Key(row4[i],
                          i * kKeyWidth,
                          i * kKeyWidth + kKeyWidth,
                          row4_ymin,
                          row4_ymin + kKeyHeight));
  }
  layout_.push_back(Key(KEY_BACKSPACE,
                        layout_.back().xmax_,
                        layout_.back().xmax_ + kBackspaceWidth,
                        row4_ymin,
                        row4_ymin + kKeyHeight));

  int rowfn_ymin = row4_ymin + kKeyHeight;
  for (i = 0; i < arraysize(rowfn); i++) {
    layout_.push_back(Key(rowfn[i],
                          i * kFnKeyWidth,
                          i * kFnKeyWidth + kFnKeyWidth,
                          rowfn_ymin,
                          rowfn_ymin + kFnKeyHeight));
  }
}


void FakeKeyboard::EnableKeyboardEvents() const {
  // Enable key events in general for output.
  EnableEventType(EV_KEY);
  // Enable each specific key code found in the layout.
  for (unsigned int i = 0; i < layout_.size(); i++) {
    EnableKeyEvent(layout_[i].event_code_);
  }
}

struct timespec FakeKeyboard::AddMsToTimespec(struct timespec const& orig,
                                              int additional_ms) {
  struct timespec extended = orig;
  extended.tv_nsec += additional_ms * 1e6;
  while (extended.tv_nsec >= 1e9) {
    extended.tv_nsec -= 1e9;
    extended.tv_sec += 1;
  }
  return extended;
}

bool FakeKeyboard::TimespecIsLater(struct timespec const& t1,
                                    struct timespec const& t2) {
  return ((t1.tv_sec > t2.tv_sec) ||
          (t1.tv_sec == t2.tv_sec && t1.tv_nsec > t2.tv_nsec));
}

int FakeKeyboard::GenerateEventForArrivingFinger(
    struct timespec now,
    struct mtstatemachine::MtFinger const &finger, int tid) {
  for (unsigned int key_num = 0; key_num < layout_.size(); key_num++) {
    if (layout_[key_num].Contains(finger.x, finger.y)) {
      Event ev(layout_[key_num].event_code_, kKeyDownEvent,
               AddMsToTimespec(now, kEventDelayMS), tid);
      EnqueueEvent(ev);
      return key_num;
    }
  }
  return kNoKey;
}

void FakeKeyboard::HandleLeavingFinger(int tid, FingerData finger,
                                       timespec now) {
  bool up_event_guaranteed = false, down_event_guaranteed = false;

  // If the finger has already been marked dead for some reason, ignore it.
  if (finger.rejection_status_ != RejectionStatus::kNotRejectedYet) {
    return;
  }

  // If there is an outstanding down event for this finger and mark it
  // guaranteed.
  if (!finger.down_sent_) {
    std::list<Event>::iterator it = pending_events_.begin();
    while (it != pending_events_.end()) {
      if (it->tid_ == tid) {
        it->is_guaranteed_ = true;
        down_event_guaranteed |= it->is_down_;
        up_event_guaranteed |= !it->is_down_;
        break;
      }
      it++;
    }
  }

  // If we're here we either have already sent the key_down event for this
  // finger, or have just marked in guaranteed.  Either way we have to enqueue
  // a guaranteed up event now if there isn't one already.
  if (!up_event_guaranteed) {
    EnqueueKeyUpEvent(layout_[finger.starting_key_number_].event_code_, now);
  }
}

void FakeKeyboard::EnqueueKeyUpEvent(int ev_code, timespec now) {
  Event up_event(ev_code, kKeyUpEvent,
                 AddMsToTimespec(now, kEventDelayMS), kOldTID);
  up_event.is_guaranteed_ = true;
  EnqueueEvent(up_event);
}

bool FakeKeyboard::StillOnFirstKey(
    struct mtstatemachine::MtFinger const & finger,
    FingerData const & data) const {
  // If this finger didn't start on a key, then automatically return false.
  if (data.starting_key_number_ == kNoKey) {
    return false;
  }

  // Otherwise, see if it's still contained in that starting key.
  return layout_.at(data.starting_key_number_).Contains(finger.x, finger.y);
}

void FakeKeyboard::RejectFinger(int tid, RejectionStatus reason) {
  // First, mark the finger's FingerData as rejected.
  finger_data_[tid].rejection_status_ = reason;

  // Next, scan through the pending events and delete any for that finger.
  std::list<Event>::iterator it = pending_events_.begin();
  while (it != pending_events_.end()) {
    auto current = it++;
    if (current->tid_ == tid) {
      pending_events_.erase(current);
    }
  }
}

void FakeKeyboard::ProcessIncomingSnapshot(
    struct timespec now,
    std::unordered_map<int, struct mtstatemachine::MtFinger> const &snapshot) {
  // First we go through all the touches reported by the touchscreen in the most
  // recent snapshot.
  for (auto map_entry : snapshot) {
    int tid = map_entry.first;
    struct mtstatemachine::MtFinger finger = map_entry.second;

    std::unordered_map<int, FingerData>::iterator data_for_tid_it;
    data_for_tid_it = finger_data_.find(tid);
    if (data_for_tid_it == finger_data_.end()) {
      int ev_code_ = GenerateEventForArrivingFinger(now, finger, tid);

      // If this is a newly arriving finger, make a new entry for it and fill
      // out all the starting data we have.  In some cases, this may invalidate
      // a finger immediately.
      FingerData data;
      data.arrival_time_ = now;
      data.max_pressure_ = finger.p;
      data.starting_key_number_ = ev_code_;
      data.down_sent_ = false;
      data.rejection_status_ = RejectionStatus::kNotRejectedYet;

      if (ev_code_ == kNoKey) {
        data.rejection_status_ = RejectionStatus::kRejectTouchdownOffKey;
      } else {
        ff_manager_.EventTriggered(TouchKeyboardEvent::FingerDown, finger.x);
      }

      // TODO(charliemooney): Add more data here that can be used for
      // tracking fingers.
      finger_data_[tid] = data;
    } else if (data_for_tid_it->second.rejection_status_ ==
               RejectionStatus::kNotRejectedYet) {
      // If we've seen this finger before, update the data on it.
      // First, Check if the maxium pressure has changed.
      data_for_tid_it->second.max_pressure_ =
          std::max(data_for_tid_it->second.max_pressure_, finger.p);

      // Check if the finger has left the key it started on
      if (!StillOnFirstKey(finger, data_for_tid_it->second)) {
        RejectFinger(data_for_tid_it->first,
                     RejectionStatus::kRejectMovedOffKey);
        if (data_for_tid_it->second.down_sent_) {
          // Send a KeyUp event to cancel any held-down buttons.
          EnqueueKeyUpEvent(
              layout_[data_for_tid_it->second.starting_key_number_].event_code_,
              now);
        }
      }

      // TODO(charliemooney): Update the additional data here, once it's added.
    }
  }

  // Next we need to check if there are any fingers missing that we saw before
  // which would indicate a finger leaving the touchscreen.
  std::unordered_map<int, FingerData>::iterator data_it = finger_data_.begin();
  while (data_it != finger_data_.end()) {
    int tid = data_it->first;
    FingerData this_finger_data = data_it->second;
    auto input_it = snapshot.find(tid);
    if (tid != kOldTID && input_it == snapshot.end()) {
      HandleLeavingFinger(tid, this_finger_data, now);
      auto next = data_it;
      next++;
      finger_data_.erase(data_it);
      data_it = next;
    } else {
      data_it++;
    }
  }
}

void FakeKeyboard::EnqueueEvent(Event ev) {
  // Find the right place in the queue, maintaining the order.
  std::list<Event>::iterator it = pending_events_.begin();
  while (it != pending_events_.end()) {
    if (TimespecIsLater(it->deadline_, ev.deadline_)) {
      pending_events_.insert(it, ev);
      return;
    }
    it++;
  }

  // If we didn't find a place for it, then it must belong at the end.
  pending_events_.push_back(ev);
}

void FakeKeyboard::Consume() {
  while (1) {
    bool needs_syn = false;

    // Compute how long to wait for a timeout. (At most until the next pending
    // event is set to fire)
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    int timeout_ms = -1;  // No timeout if there aren't any pending events.
    if (!pending_events_.empty()) {
      timespec deadline = pending_events_.front().deadline_;
      timeout_ms = ((deadline.tv_sec - now.tv_sec) * 1000 +
                    (deadline.tv_nsec - now.tv_nsec) / 1000000);
      timeout_ms++;  // Always add 1 more ms so as to not undershoot.
      if (timeout_ms < 0) {
        LOG(WARNING) << "Negative timeout (" << timeout_ms <<
                        ").  We missed an event somewhere!\n";
        timeout_ms = 1;
      }
    }

    // Wait for a touchscreen event or a timeout.  If there's an event from
    // the touchscreen add it to the statemachine.  Otherwise, that means it's
    // time to fire the next pending event.
    struct input_event ev;
    bool event_ready = GetNextEvent(timeout_ms, &ev);
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (event_ready) {
      std::unordered_map<int, struct mtstatemachine::MtFinger> snapshot;
      if (sm_.AddEvent(ev, &snapshot)) {
        // Here we process the new snapshot, enqueing events as needed.
        ProcessIncomingSnapshot(now, snapshot);
      } else {
        // If it was a touchscreen event but *not* the end of a snapshot, then
        // just continue the loop.  This allows us to treat touchscreen events
        // as atomic events where the entire snapshot arrives at one time.
        continue;
      }
    }

    // Loop over pending events and process any that are ready to fire.
    while (!pending_events_.empty()) {
      // If the next event's deadline is still in the future, stop looking.
      Event next_event = pending_events_.front();
      if (TimespecIsLater(next_event.deadline_, now)) {
        break;
      }

      // Pop off the next pending event and process it now.
      pending_events_.pop_front();

      // Look up the FingerData associated with this event and make sure the
      // event is still valid.
      std::unordered_map<int, FingerData>::iterator it;
      it = finger_data_.find(next_event.tid_);
      if (it != finger_data_.end()) {
        // Here we check to see if this event is still valid before firing it
        // off to the OS.  Currently there is only a pressure check here, but
        // more could easily be added later.

        // This checks if the maximum pressure a finger reported is within
        // range.  An exception is made for the spacebar since it is often
        // pressed by a user's thumb, which may have unusually high pressure.
        if (it->second.max_pressure_ < kMinTapPressure ||
            (layout_[it->second.starting_key_number_].event_code_ !=
             KEY_SPACE && it->second.max_pressure_ > kMaxTapPressure)) {
          LOG(INFO) << "Tap rejected!  Pressure of " <<
                    it->second.max_pressure_ << " is out of range " <<
                    kMinTapPressure << "->" << kMaxTapPressure;
          continue;
        }
      } else {
        // The finger has already left -- that's OK as long as it is
        // "guaranteed" to fire.
        if (!next_event.is_guaranteed_) {
          LOG(ERROR) << "No finger data for event that should have some! " <<
                       "(guaranteed: " << next_event.is_guaranteed_ << ", " <<
                       "is_down: " << next_event.is_down_ << ", " <<
                       "tid: " << next_event.tid_ << ")";
        }
      }

      // Actually send the event and update the fingerdata if applicable.
      SendEvent(EV_KEY, next_event.ev_code_, next_event.is_down_ ? 1 : 0);
      needs_syn = true;
      if (next_event.is_down_) {
        std::unordered_map<int, FingerData>::iterator it;
        it = finger_data_.find(next_event.tid_);
        if (it != finger_data_.end()) {
          finger_data_[next_event.tid_].down_sent_ = true;
        }
      }
    }
    if (needs_syn) {
      // Finally, send out a SYN after all applicable events are sent.
      SendEvent(EV_SYN, SYN_REPORT, 0);
    }
  }
}

void FakeKeyboard::Start(std::string const &source_device_path,
                         std::string const &keyboard_device_name) {
  // Do all the set up steps.
  OpenSourceDevice(source_device_path);
  CreateUinputFD();
  EnableKeyboardEvents();
  FinalizeUinputCreation(keyboard_device_name);

  // Loop forever, comsuming the events coming in from the source device and
  // generating keystroke events when appropriate.
  Consume();
}

}  // namespace touch_keyboard
