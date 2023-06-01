#
# Copyright (c) 2023, Te;edatics
#
# 
#

board_set_debugger_ifnset(jlink)
board_set_flasher_ifnset(jlink)

board_runner_args(jlink "--device=MIMX8MD6_M4")
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
