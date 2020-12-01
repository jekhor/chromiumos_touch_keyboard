// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "touch_keyboard/fakekeyboard.h"

#define CSV_IO_NO_THREAD
#include "csv.h"

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

constexpr int kMinTapPressure = 50;
constexpr int kMaxTapPressure = 110;

constexpr int kMinTapTouchDiameter = 300;
constexpr int kMaxTapTouchDiameter = 3000;

FakeKeyboard::FakeKeyboard(struct hw_config &hw_config) :
  hw_config_(hw_config) {

  fn_key_pressed_ = false;

  LoadLayout("layout.csv");

  ff_manager_ = new TouchFFManager(hw_config_.res_x, hw_config_.res_y,
      hw_config_.rotation);
}

bool FakeKeyboard::LoadLayout(std::string const &layout_filename) {
  double hw_pitch_x, hw_pitch_y;

  hw_pitch_x = hw_config_.res_x / hw_config_.width_mm;
  hw_pitch_y = hw_config_.res_y / hw_config_.height_mm;

  LOG(DEBUG) << "pitch: " << hw_pitch_x << "x" << hw_pitch_y;

  io::CSVReader<8,
    io::trim_chars<' ', '\t'>,
    io::no_quote_escape<';'>> l_csv(layout_filename);

  l_csv.read_header(io::ignore_missing_column, "x", "y", "width", "height",
      "name", "code", "name_fn", "code_fn");

  double x, y, w, h;
  double left_margin, top_margin;
  std::string keyname;
  std::string keyname_fn = "";
  int keycode = 0;
  int keycode_fn = 0;

  left_margin = hw_config_.left_margin_mm;
  top_margin = hw_config_.top_margin_mm;

  while(l_csv.read_row(x, y, w, h, keyname, keycode, keyname_fn, keycode_fn)) {
    LOG(INFO) << "Key " << keyname << "(" << keycode << ") | " <<
      keyname_fn << " (" << keycode_fn << "): " <<
      w << "x" << h << "@(" << x << "," << y << ") mm";

    int x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    switch (hw_config_.rotation) {
      case 0:
        x1 = (left_margin + x) * hw_pitch_x;
        x2 = (left_margin + x + w) * hw_pitch_x;
        y1 = (top_margin + y) * hw_pitch_y;
        y2 = (top_margin + y + h) * hw_pitch_y;
        break;
      case 90:
        x1 = (top_margin + y) * hw_pitch_x;
        x2 = (top_margin + y + h) * hw_pitch_x;
        y1 = (hw_config_.height_mm - (left_margin + x + w)) * hw_pitch_y;
        y2 = (hw_config_.height_mm - (left_margin + x)) * hw_pitch_y;
        break;
      case 180:
        x1 = (hw_config_.width_mm - (left_margin + x + w)) * hw_pitch_x;
        x1 = (hw_config_.width_mm - (left_margin + x)) * hw_pitch_x;
        y1 = (hw_config_.height_mm - (top_margin + y + h)) * hw_pitch_y;
        y2 = (hw_config_.height_mm - (top_margin + y)) * hw_pitch_y;
        break;
      case 270:
        x1 = (hw_config_.width_mm - (y + h + top_margin)) * hw_pitch_x;
        x2 = (hw_config_.width_mm - (y + top_margin)) * hw_pitch_x;
        y1 = (left_margin + x) * hw_pitch_y;
        y2 = (left_margin + x + w) * hw_pitch_y;
        break;
      default:
        LOG(ERROR) << "Rotation by " << hw_config_.rotation << " degrees is not supported";
        return false;
    }

    LOG(INFO) << "HW coords: (" << x1 << ", " << y1 << "), (" <<
      x2 << ", " << y2 << ")";

    layout_.push_back(Key(keycode, keycode_fn, x1, x2, y1, y2));
  }

  return true;
}

void FakeKeyboard::EnableKeyboardEvents() const {
  // Enable key events in general for output.
  EnableEventType(EV_KEY);
  // Enable each specific key code found in the layout.
  for (unsigned int i = 0; i < layout_.size(); i++) {
    EnableKeyEvent(layout_[i].event_code_);
    if (layout_[i].event_code_fn_)
      EnableKeyEvent(layout_[i].event_code_fn_);
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
    struct mtstatemachine::MtFinger const &finger, int tid, int *event_code) {

  for (unsigned int key_num = 0; key_num < layout_.size(); key_num++) {
    if (layout_[key_num].Contains(finger.x, finger.y)) {

      if (fn_key_pressed_ && layout_[key_num].event_code_fn_)
        *event_code = layout_[key_num].event_code_fn_;
      else
        *event_code = layout_[key_num].event_code_;

      LOG(DEBUG) << "fn_key_pressed_: " << fn_key_pressed_ << ", event_code: " <<
        *event_code;

      Event ev(*event_code, kKeyDownEvent,
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
    EnqueueKeyUpEvent(finger.event_code_, now);
    if (finger.event_code_ == KEY_FN)
      fn_key_pressed_ = false;
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
  LOG(DEBUG) << "Reject finger, reason " << static_cast<int>(reason);
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
      int event_code = 0;
      int key = GenerateEventForArrivingFinger(now, finger, tid, &event_code);

      // If this is a newly arriving finger, make a new entry for it and fill
      // out all the starting data we have.  In some cases, this may invalidate
      // a finger immediately.
      FingerData data;
      data.arrival_time_ = now;
      data.max_pressure_ = finger.p;
      data.max_touch_major_ = finger.touch_major;
      data.starting_key_number_ = key;
      data.event_code_ = event_code;
      data.down_sent_ = false;
      data.rejection_status_ = RejectionStatus::kNotRejectedYet;

      if (event_code == KEY_FN)
        fn_key_pressed_ = true;

      if (key == kNoKey) {
        data.rejection_status_ = RejectionStatus::kRejectTouchdownOffKey;
      } else {
        ff_manager_->EventTriggered(TouchKeyboardEvent::FingerDown, finger.x, finger.y);
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

      // The same for touch contact diameter
      data_for_tid_it->second.max_touch_major_ =
          std::max(data_for_tid_it->second.max_touch_major_, finger.touch_major);

      // Check if the finger has left the key it started on
      if (!StillOnFirstKey(finger, data_for_tid_it->second)) {
        RejectFinger(data_for_tid_it->first,
                     RejectionStatus::kRejectMovedOffKey);
        if (data_for_tid_it->second.down_sent_) {
          // Send a KeyUp event to cancel any held-down buttons.
          EnqueueKeyUpEvent(
              data_for_tid_it->second.event_code_, now);

          if (data_for_tid_it->second.event_code_ == KEY_FN)
            fn_key_pressed_ = false;
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

        if (it->second.max_pressure_ != -1) {
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
          if (it->second.max_touch_major_ < kMinTapTouchDiameter ||
            (layout_[it->second.starting_key_number_].event_code_ !=
             KEY_SPACE && it->second.max_touch_major_ > kMaxTapTouchDiameter)) {
            LOG(INFO) << "Tap rejected!  Diameter of " <<
              it->second.max_touch_major_ << " is out of range " <<
              kMinTapTouchDiameter << "->" << kMaxTapTouchDiameter;
            continue;
          }

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

      LOG(DEBUG) << "Event: EV_KEY, code " << next_event.ev_code_ << " down: " << next_event.is_down_;
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
  if (!OpenSourceDevice(source_device_path))
    return;

  CreateUinputFD();
  EnableKeyboardEvents();
  FinalizeUinputCreation(keyboard_device_name);

  // Loop forever, comsuming the events coming in from the source device and
  // generating keystroke events when appropriate.
  Consume();
}

}  // namespace touch_keyboard
