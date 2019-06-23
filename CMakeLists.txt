cmake_minimum_required(VERSION 3.0)

project(chromiumos_touch_keyboard)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

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
	)

find_library(ANDROID_BASE_LIB base
	PATH_SUFFIXES android
	)

find_path(ANDROID_INCDIR
	android-base/macros.h
	PATH_SUFFIXES android
	)

include_directories("${ANDROID_INCDIR}")

message("Android base library headers are located in " ${ANDROID_INCDIR}/android-base)

add_custom_target(make_headers_links
	COMMAND rm -f ./base ./touch_keyboard
	COMMAND ln -sf "${ANDROID_INCDIR}/android-base" ./base
	COMMAND ln -sf "${PROJECT_SOURCE_DIR}" ./touch_keyboard
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
add_dependencies(touch_keyboard_handler make_headers_links)

target_link_libraries(touch_keyboard_handler ${ANDROID_BASE_LIB})
