#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)


set(LOCAL_RPMSG_LITE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/rpmsg_lite)
set(RPMSG_LITE ${CMAKE_CURRENT_SOURCE_DIR}/../../rpmsg-lite)

include_directories(BEFORE ${LOCAL_RPMSG_LITE})

set(APP_SOURCES src/main.c src/rsc_table.c src/vsprintf.c 
${RPMSG_LITE}/lib/common/llist.c 
${LOCAL_RPMSG_LITE}/rpmsg_lite.c 
${RPMSG_LITE}/lib/rpmsg_lite/rpmsg_ns.c 
${LOCAL_RPMSG_LITE}/rpmsg_queue.c 
${LOCAL_RPMSG_LITE}/rpmsg_env_zephyr.c 
${LOCAL_RPMSG_LITE}/rpmsg_platform.c 
${LOCAL_RPMSG_LITE}/virtqueue.c 
)

set(APP_SOURCES ${APP_SOURCES} )

target_include_directories(app 
PUBLIC ${LOCAL_RPMSG_LITE} 
PUBLIC ${RPMSG_LITE}/lib/include 
PUBLIC ${RPMSG_LITE}/lib/include/environment/zephyr 
PUBLIC ${RPMSG_LITE}/lib/include/platform/imx8mm_m4 
PUBLIC src)
target_sources(app PRIVATE ${APP_SOURCES})
