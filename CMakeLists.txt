cmake_minimum_required(VERSION 3.0)

project(chromiumos_touch_keyboard)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -pedantic -Wno-unknown-pragmas")

add_executable(touch_keyboard_handler
	main.cc
	evdevsource.cc
	fakekeyboard.cc
	faketouchpad.cc
	uinputdevice.cc
	haptic/ff_driver.cc
	haptic/touch_ff_manager.cc
	statemachine/eventkey.cc
	statemachine/slot.cc
	statemachine/statemachine.cc
	logging.cc
	)

include(GNUInstallDirs)

set(SYSTEMD_UNIT_DIR "/lib/systemd/system/")
set(UDEV_DIR "/lib/udev/")

install(TARGETS touch_keyboard_handler
	DESTINATION sbin)

install(DIRECTORY layouts
	DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/touch_keyboard)

install(FILES layout-touchpad.csv touch-hw.csv
	DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/touch_keyboard)

install(FILES 60-touch-keyboard.rules
	DESTINATION ${UDEV_DIR}/rules.d)

install(FILES 61-evdev-yogabook.hwdb
	DESTINATION ${UDEV_DIR}/hwdb.d)

install(FILES touch-keyboard-handler.service
	DESTINATION ${SYSTEMD_UNIT_DIR})

