/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 * Copyright 2020 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This file populates resource table for BM remote
 * for use by the Linux Master */

#include <zephyr/kernel.h>
#include "board.h"
#include "rsc_table.h"
#include "rpmsg_lite.h"
#include "platform/imx8mm_m4/rpmsg_platform.h"
#include <string.h>

#define RSC_TABLE_NAME "rsc_table"
#if !DT_HAS_CHOSEN(zephyr_rsc_table)
#error "Sample requires definition of rsc_table for rpmsg"
#endif

#define VDEV0_VRING_DEVICE_NAME "vring0"
#if !DT_HAS_CHOSEN(zephyr_vring0_shm)
#error "Sample requires definition of vring0 for rpmsg"
#endif

/* Constants derived from device tree */
#define RSC_TABLE_NODE		DT_CHOSEN(zephyr_rsc_table)
#define RSC_TABLE_START_ADDR	DT_REG_ADDR(RSC_TABLE_NODE)
#define RSC_TABLE_SIZE		DT_REG_SIZE(RSC_TABLE_NODE)

#define VRING0_NODE		DT_CHOSEN(zephyr_vring0_shm)
#define VRING0_START_ADDR	DT_REG_ADDR(VRING0_NODE)
#define VRING0_SIZE		DT_REG_SIZE(VRING0_NODE)


#define NUM_VRINGS 0x02

/* Place resource table in special ELF section */
#if defined(__ARMCC_VERSION) || defined(__GNUC__)
__attribute__((section(".resource_table")))
#elif defined(__ICCARM__)
#pragma location = ".resource_table"
#else
#error Compiler not supported!
#endif

const struct remote_resource_table resources = {
    /* Version */
    1,

    /* NUmber of table entries */
    NO_RESOURCE_ENTRIES,
    /* reserved fields */
    {
        0,
        0,
    },

    /* Offsets of rsc entries */
    {
        offsetof(struct remote_resource_table, user_vdev),
    },

    /* SRTM virtio device entry */
    {
        RSC_VDEV,
        7,
        0,
        RSC_VDEV_FEATURE_NS,
        0,
        0,
        0,
        NUM_VRINGS,
        {0, 0},
    },

    /* Vring rsc entry - part of vdev rsc entry */
    {VRING0_START_ADDR, VRING_ALIGN, RL_BUFFER_COUNT, 0, 0},
    {VRING0_START_ADDR + VRING0_SIZE, VRING_ALIGN, RL_BUFFER_COUNT, 1, 0},
/*    {VDEV0_VRING_BASE, VRING_ALIGN, RL_BUFFER_COUNT, 0, 0},
    {VDEV0_VRING_BASE + VRING_SIZE, VRING_ALIGN, RL_BUFFER_COUNT, 1, 0}, */
};

void copyResourceTable(void)
{
    /*
     * Resource table should be copied to VDEV0_VRING_BASE + RESOURCE_TABLE_OFFSET.
     * VDEV0_VRING_BASE is temperorily kept for backward compatibility, will be
     * removed in future release
     */
    memcpy((void *)RSC_TABLE_START_ADDR, &resources, sizeof(resources));
/*    memcpy((void *)VDEV0_VRING_BASE, &resources, sizeof(resources));
    memcpy((void *)(VDEV0_VRING_BASE + RESOURCE_TABLE_OFFSET), &resources, sizeof(resources)); */
}
