/*
 * Copyright (c) 2021 YuLong Yao <feilongphone@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

static int rpmsg_lite_init(void)
{
    printk("rpmsg_lite_init\n");
    return 0;
}

SYS_INIT(rpmsg_lite_init, PRE_KERNEL_1, 0);
