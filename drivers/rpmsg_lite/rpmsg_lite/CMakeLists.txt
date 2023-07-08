# Copyright (c) 2023 Teledatics, Inc.
#

message("Building rpmsg_lite")
zephyr_library()
zephyr_library_sources(rpmsg_lite.c)
