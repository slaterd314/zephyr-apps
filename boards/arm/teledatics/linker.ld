/*
 * Copyright (c) 2020, Manivannan Sadhasivam <mani@kernel.org>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_OPENAMP_RSC_TABLE
#undef CONFIG_OPENAMP_RSC_TABLE
#define CONFIG_RPMSG_LITE_RSC_TABLE 1
#endif

#include <zephyr/arch/arm/aarch32/cortex_m/scripts/linker.ld>

SECTIONS
     {
/* #ifdef CONFIG_RPMSG_LITE_RSC_TABLE */
        SECTION_PROLOGUE(.resource_table,, SUBALIGN(8))
        {
            KEEP(*(.resource_table*))
        } GROUP_LINK_IN(RAMABLE_REGION)
/* #endif */
     }
