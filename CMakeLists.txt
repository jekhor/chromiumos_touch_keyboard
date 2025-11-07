cmake_minimum_required(VERSION 3.22)

project(chromiumos_touch_keyboard)

find_package(PkgConfig)

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

pkg_check_modules(SYSTEMD "systemd")
pkg_get_variable(SYSTEMD_SYSTEM_UNIT_DIR systemd systemdsystemunitdir)

pkg_check_modules(UDEV "udev")
pkg_get_variable(UDEV_DIR udev udevdir)

install(TARGETS touch_keyboard_handler
	DESTINATION "${CMAKE_INSTALL_SBINDIR}")

install(DIRECTORY layouts
	DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/touch_keyboard)

install(FILES layout-touchpad.csv touch-hw.csv
	DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/touch_keyboard)

install(FILES 60-touch-keyboard.rules
	DESTINATION ${UDEV_DIR}/rules.d)

install(FILES 61-evdev-yogabook.hwdb
	DESTINATION ${UDEV_DIR}/hwdb.d)

install(FILES touch-keyboard-handler.service
	DESTINATION ${SYSTEMD_SYSTEM_UNIT_DIR})

