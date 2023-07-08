# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

message("Building drivers")
add_subdirectory_ifdef(CONFIG_RPMSG_LITE_RSC_TABLE rpmsg_lite)
