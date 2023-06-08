#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

set(RPMSG_LITE ${CMAKE_CURRENT_SOURCE_DIR}/../../rpmsg-lite)

set(APP_SOURCES src/main.c src/rsc_table.c 
${RPMSG_LITE}/lib/common/llist.c 
${RPMSG_LITE}/lib/rpmsg_lite/rpmsg_lite.c 
${RPMSG_LITE}/lib/rpmsg_lite/rpmsg_ns.c 
${RPMSG_LITE}/lib/rpmsg_lite/rpmsg_queue.c 
${RPMSG_LITE}/lib/rpmsg_lite/porting/environment/rpmsg_env_zephyr.c 
${RPMSG_LITE}/lib/rpmsg_lite/porting/platform/imx8mm_m4/rpmsg_platform.c 
${RPMSG_LITE}/lib/virtio/virtqueue.c 
)

set(APP_SOURCES ${APP_SOURCES} )

target_include_directories(app 
PUBLIC ${RPMSG_LITE}/lib/include 
PUBLIC ${RPMSG_LITE}/lib/include/environment/zephyr 
PUBLIC ${RPMSG_LITE}/lib/include/platform/imx8mm_m4 
PUBLIC src)
target_sources(app PRIVATE ${APP_SOURCES})
