# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0
#
# This CMake file is picked by the Zephyr build system because it is defined
# as the module CMake entry point (see zephyr/module.yml).

zephyr_include_directories(include)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rpmsg-lite/lib/include)

add_subdirectory(drivers)
add_subdirectory(lib)
