## Overview
This package includes the software required for enabling a low-level
touch-based keyboard and touchpad.  It includes utilities for reading
touch events from a specified multitouch input device and selecting
a certain region to act as a normal touchpad, then laying out where
various "keys" are for a keyboard.

To generate the actual events, this package uses the uinput kernel module
to create fake input devices at the kernel level.  This allows the
touch keyboard and touchpad to look identical to real, physical devices
and keeps the upper layers consistent.

When you build this package it will generate an executable called
/usr/bin/touch\_keyboard\_handler that gets started automatically on boot by
an upstart script.

This package also includes a command line tool to test the vibrator
for the touch keyboard: /usr/bin/touchkb\_haptic\_test. It operates the
symlink of left vibrator and right vibrator under /dev/left\_vibrator
and /dev/right\_vibrator. The proper symlink should be created by udev rules.

## Class descriptions

### FakeTouchpad
This class is a high-level abstraction that handles everything required to
create a "fake" touchpad from a subregion of a larger touch device by using the
other helper classes defined here.

### FakeKeyboard
This class is a high-level abstraction that handles everything required to
create a "fake" keyboard from a subregion of a larger touch device.  It knows
where each key is on the pad and generates input events accordingly for the
system as if it were a real keyboard.

### EvdevSource
This is a class that can be derived from (as FakeTouchpad does) to easily allow
an object to collect and process Evdev events from an input device.

### UinputDevice
This is a class that can be derived from (as FakeTouchpad does) to easily allow
an object to create a brand new input device for the OS by using the uinput
module, and then generate input event programmatically.

### MtStateMachine
This class processes Evdev events into a more manageable format
by mimicking the kernel's state machine, and allows us to make sense
of the events captured from the source touch sensor.

## Haptic Test

### Usage
| parameter     |  explain   |
|---------------|------------|
help            |    Show help message.
duramtion\_ms   |    Set the duration of vibration in ms. Default is 1000.
magnitude       |    Set the strength of vibration, the value is from 0.0 to 1.0. Default is 1.0.
vibrator        |    Select left or right motor. Default is "left".

### Example
Drive the left vibrator at max strength for 500ms:

    $touchkb\_haptic\_test --vibrator=left --magnitude=1.0 --duration\_ms=500

Drive the right vibrator at half strength for 300ms:

    $touchkb\_haptic\_test --vibrator=right --magnitude=0.5 --duration\_ms=300
