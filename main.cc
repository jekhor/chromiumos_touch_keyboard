// Copyright 2016 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <logging.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>

#define CSV_IO_NO_THREAD
#include "csv.h"

#include "fakekeyboard.h"
#include "faketouchpad.h"

// This filepath is used as the input evdev device. Whichever touch sensor is
// to be used for touch keyboard input should have a udev rule put in place to
// set up this symlink.
constexpr char kTouchSensorDevicePath[] = "/dev/touch_keyboard";

using touch_keyboard::FakeTouchpad;
using touch_keyboard::FakeKeyboard;

bool LoadHWConfig(std::string const &hw_config_file, struct touch_keyboard::hw_config &hw_config) {
  io::CSVReader<7,
    io::trim_chars<' ', '\t'>,
    io::no_quote_escape<';'>> csv(hw_config_file);

  csv.read_header(io::ignore_no_column,
      "resolution_x", "resolution_y",
      "width_mm", "height_mm",
      "left_margin_mm", "top_margin_mm",
      "rotation_cw");

  int res_x, res_y;
  double w_mm, h_mm;
  double left_margin_mm, top_margin_mm;
  int rotation;

  if (!csv.read_row(res_x, res_y, w_mm, h_mm,
                    left_margin_mm, top_margin_mm, rotation))
    return false;

  LOG(INFO) << "Touchpad HW config: " << res_x << "x" << res_y << " points, " <<
    w_mm << "x" << h_mm << " mm, margins is " <<
    left_margin_mm << "+" << top_margin_mm <<
    ", rotated by " << rotation << " deg. clockwise.\n";

  hw_config.res_x = res_x;
  hw_config.res_y = res_y;
  hw_config.width_mm = w_mm;
  hw_config.height_mm = h_mm;
  hw_config.rotation = rotation;
  hw_config.left_margin_mm = left_margin_mm;
  hw_config.top_margin_mm = top_margin_mm;

  return true;
}

const double tp_x1_mm = 68;
const double tp_x2_mm = 155;
const double tp_y1_mm = 98.5;
const double tp_y2_mm = 133;

int main(int argc, char *argv[]) {
  struct touch_keyboard::hw_config hw_config;
  int debug_level = 0;
  int opt;

  while ((opt = getopt(argc, argv, "hd")) != -1) {
    switch (opt) {
      case 'h':
        std::cerr << "Usage: touch_keyboard_handler [-h] [-d]\n";
        return 0;
      case 'd':
        debug_level++;
        break;
      default:
        std::cerr << "Unknown option " << (char)opt << "\n";
        exit(EXIT_FAILURE);
    }
  }

  if (debug_level)
    SetMinimumLogSeverity(DEBUG);

  LOG(INFO) << "Starting touch_keyboard_handler\n";

  LoadHWConfig("touch-hw.csv", hw_config);

  try {
    // Fork into two processes, one to handle the keyboard functionality
    // and one to handle the touchpad region.
    int pid = fork();
    if (pid < 0) {
      LOG(FATAL) << "ERROR: Unable to fork! (" << pid << ")\n";
    } else if (pid == 0) {
      // TODO(charliemooney): Get these coordinates from somewhere not hard-coded
      LOG(INFO) << "Creating Fake Touchpad.\n";
      FakeTouchpad tp(tp_x1_mm, tp_x2_mm, tp_y1_mm, tp_y2_mm, hw_config);
      tp.Start(kTouchSensorDevicePath, "virtual-touchpad");
    } else {
      FakeKeyboard kbd(hw_config);
      kbd.Start(kTouchSensorDevicePath, "virtual-keyboard");
      wait(NULL);
    }
  } catch (...) {
    exit(EXIT_FAILURE);
  }

  return 0;
}
