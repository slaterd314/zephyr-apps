# Copyright (c) 2023 Teledatics, Inc.
#
message("rpmsg_lite dir")
add_subdirectory_ifdef(CONFIG_RPMSG_LITE_RSC_TABLE rpmsg_lite)