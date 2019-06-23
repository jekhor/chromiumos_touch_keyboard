// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOUCH_KEYBOARD_UINPUT_DEFINITIONS_H_
#define TOUCH_KEYBOARD_UINPUT_DEFINITIONS_H_

#include <linux/uinput.h>

/* Define these structs and ioctls so this can compile against an old kernel
 * that doesn't support these features yet.  The kernel this code runs on
 * must support it, but adding these here allows it to build elsewhere.
 * Without these, uinput can't correctly support setting the axis ranges for
 * absolute axes which is essential for simulated touch devices.
 */

#ifndef UI_ABS_SETUP
#define UI_ABS_SETUP _IOW(UINPUT_IOCTL_BASE, 4, struct uinput_abs_setup)
struct uinput_abs_setup {
  __u16  code; /* axis code */
  /* __u16 filler; */
  struct input_absinfo absinfo;
};
#endif

#ifndef UI_DEV_SETUP
#define UI_DEV_SETUP _IOW(UINPUT_IOCTL_BASE, 3, struct uinput_setup)
struct uinput_setup {
  struct input_id id;
  char name[UINPUT_MAX_NAME_SIZE];
  __u32 ff_effects_max;
};
#endif

#endif  // TOUCH_KEYBOARD_UINPUT_DEFINITIONS_H_
