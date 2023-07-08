/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <app_version.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <virtqueue.h>
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"
#include "rpmsg_config.h"
#include <assert.h>
#include "board.h"
#include "rsc_table.h"


#define SHM_DEVICE_NAME	"shm"

#if !DT_HAS_CHOSEN(zephyr_ipc_shm)
#error "Sample requires definition of shared memory for rpmsg"
#endif

#define VDEV0_VRING_DEVICE_NAME "vring0"
#if !DT_HAS_CHOSEN(zephyr_vring0_shm)
#error "Sample requires definition of vring0 for rpmsg"
#endif

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);
extern void my_debug_out(const char *fmt,...);
/* #ifdef printk
#undef printk
#endif
#define printk my_debug_out */


/* Constants derived from device tree */
#define SHM_NODE		DT_CHOSEN(zephyr_ipc_shm)
#define SHM_START_ADDR	DT_REG_ADDR(SHM_NODE)
#define SHM_SIZE		DT_REG_SIZE(SHM_NODE)

#define VRING0_NODE		DT_CHOSEN(zephyr_vring0_shm)
#define VRING0_START_ADDR	DT_REG_ADDR(VRING0_NODE)
#define VRING0_SIZE		DT_REG_SIZE(VRING0_NODE)

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// #define RPMSG_LITE_SHMEM_BASE         (VDEV0_VRING_BASE)
#define RPMSG_LITE_SHMEM_BASE         (VRING0_START_ADDR)
#define RPMSG_LITE_LINK_ID            (RL_PLATFORM_IMX8MM_M4_USER_LINK_ID)
#define RPMSG_LITE_NS_ANNOUNCE_STRING "rpmsg-virtual-tty-channel-1"
#define APP_TASK_STACK_SIZE (256)
#ifndef LOCAL_EPT_ADDR
#define LOCAL_EPT_ADDR (30)
#endif

/* Globals */
static char app_buf[512]; /* Each RPMSG buffer can carry less than 512 payload */
static struct rpmsg_lite_instance *volatile my_rpmsg = NULL;
static struct rpmsg_lite_endpoint *volatile my_ept = NULL;
static volatile rpmsg_queue_handle my_queue        = NULL;
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
static uint8_t queue_storage[256*sizeof(rpmsg_queue_rx_cb_data_t)];
#endif

static void dumpVirtqueue(const char *name, struct virtqueue *vq)
{
#if 0
	// dump rvq to the console
	printk("dump %s to the console.\n", name);
	printk("vq name: %s\n", vq->vq_name);
	printk("vq flags: %d\n", vq->vq_flags);
	printk("vq align: %d\n", vq->vq_alignment);
	printk("vq ring size: %d\n", vq->vq_ring_size);
	printk("vq ring mem: %p\n", vq->vq_ring_mem);
	printk("vq callback_fc: %p\n", vq->callback_fc);
	printk("vq notify_fc: %p\n", vq->notify_fc);
	printk("vq max indirect size: %d\n", vq->vq_max_indirect_size);
	printk("vq indirect mem size: %d\n", vq->vq_indirect_mem_size);

	printk("vq->vq_ring.num: %d\n", vq->vq_ring.num);
	printk("vq->vq_ring.desc: %p\n", vq->vq_ring.desc);
	printk("vq->vq_ring.avail: %p\n", vq->vq_ring.avail);
	printk("vq->vq_ring.used: %p\n", vq->vq_ring.used);
	printk("vq->vq_ring.used->ring[0].id: %d\n", vq->vq_ring.used->ring[0].id);
	printk("vq->vq_ring.used->ring[0].len: %d\n", vq->vq_ring.used->ring[0].len);
#else
	((void)name);
	((void)vq);
#endif
}

static void dump_rpmsg_lite_instance(struct rpmsg_lite_instance *rpmsg)
{
#if 1
	printk("rpmsg addr: %p\n", rpmsg);
	printk("rpmsg_lite_dev->link_state addr: %p\n", &(rpmsg->link_state));
	printk("offset: %d\n", (uint32_t)(&(rpmsg->link_state)) - (uint32_t)rpmsg);

	virtqueue_dump(rpmsg->rvq);
	virtqueue_dump(rpmsg->tvq);
	printk("rl_endpoints: %p\n", rpmsg->rl_endpoints);
	printk("LOCK: %p\n", rpmsg->lock);
	printk("link_state: %u\n", rpmsg->link_state);
	printk("sh_mem_base: %p\n", rpmsg->sh_mem_base);
	printk("sh_mem_remaining: %u\n", rpmsg->sh_mem_remaining);
	printk("sh_mem_total: %u\n", rpmsg->sh_mem_total);
	printk("link_id: %u\n", rpmsg->link_id);
#else
	((void)rpmsg);
#endif
}

void app_task(void *arg1, void *arg2, void *arg3)
{
    volatile uint32_t remote_addr = 0;
    struct rpmsg_lite_instance *my_rpmsg = NULL;	
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
    struct rpmsg_lite_ept_static_context my_ept_context = {0};
    struct rpmsg_lite_instance rpmsg_ctxt = {0};
	rpmsg_static_queue_ctxt queue_ctxt = {0};
#endif	

    void *rx_buf = NULL;
    uint32_t len = 0;
    int32_t result = 0;
    void *tx_buf = NULL;
    uint32_t size = 0;

	((void)arg1);
	((void)arg2);
	((void)arg3);

    /* Print the initial banner */
    printk("\r\nRPMSG String Echo Zephyr RTOS API Demo...\r\nsizeof(struct rpmsg_lite_instance): %d\r\nRPMSG_LITE_SHMEM_BASE: %p\r\n",
	sizeof(struct rpmsg_lite_instance),
	(void *)RPMSG_LITE_SHMEM_BASE);
#if 0
	copyResourceTable();

    /* Print the initial banner */
    printk("\r\nAfter copyResourceTable()\r\nsizeof(struct rpmsg_lite_instance): %d\r\nRPMSG_LITE_SHMEM_BASE: %p\r\n",
	sizeof(struct rpmsg_lite_instance),
	(void *)RPMSG_LITE_SHMEM_BASE);

#endif 

#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
	my_rpmsg = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS, &rpmsg_ctxt);
#else
	my_rpmsg = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);
#endif

	if(my_rpmsg == RL_NULL)
	{
		printk("rpmsg_lite_remote_init failed.\n");
		return;
	}

	dump_rpmsg_lite_instance(my_rpmsg);

	if(my_rpmsg->rvq == RL_NULL)
	{
		printk("rvq is NULL.\n");
		return;
	}

	dumpVirtqueue("rvq", my_rpmsg->rvq);
	
	if(my_rpmsg->tvq == RL_NULL)
	{
		printk("tvq is NULL.\n");
		return;
	}

	dumpVirtqueue("tvq", my_rpmsg->tvq);
/*
	if(device_get_binding(DT_N_S_soc_S_mailbox_30ab0000_FULL_NAME))
	{
		my_rpmsg->link_state = 1;
	}
*/
	printk("rpmsg_lite_wait_for_link_up. linkstate addr: %p\n", &(my_rpmsg->link_state));
	/* uint32_t test = RL_SUCCESS; */ /* rpmsg_lite_wait_for_link_up(my_rpmsg, RL_BLOCK); */
	uint32_t test = rpmsg_lite_wait_for_link_up(my_rpmsg, RL_BLOCK);
	if(test != RL_SUCCESS)
	{
		printk("rpmsg_lite_wait_for_link_up failed.\n");
		return;
	}



#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
    my_queue = rpmsg_queue_create(my_rpmsg, queue_storage, &queue_ctxt);
#else
	my_queue = rpmsg_queue_create(my_rpmsg);
#endif

	if(my_queue == RL_NULL)
	{
		printk("rpmsg_queue_create failed.\n");
		return;
	}
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
    my_ept   = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, my_queue, &my_ept_context);
#else
	my_ept   = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, my_queue);
#endif	

	if(my_ept == RL_NULL)
	{
		printk("rpmsg_lite_create_ept failed.\n");
		return;
	}


    int32_t success = rpmsg_ns_announce(my_rpmsg, my_ept, RPMSG_LITE_NS_ANNOUNCE_STRING, RL_NS_CREATE);
	if(RL_SUCCESS != success)
	{
		printk("rpmsg_ns_announce failed.\n");
		return;
	}

    printk("\r\nNameservice sent, ready for incoming messages...\r\n");

    for (;;)
    {
        /* Get RPMsg rx buffer with message */
        result =
            rpmsg_queue_recv_nocopy(my_rpmsg, my_queue, (uint32_t *)&remote_addr, (char **)&rx_buf, &len, RL_BLOCK);
        if (result != 0)
        {
			printk("rpmsg_queue_recv_nocopy failed.\n");
            assert(false);
        }

        /* Copy string from RPMsg rx buffer */
        assert(len < sizeof(app_buf));
        memcpy(app_buf, rx_buf, len);
        app_buf[len] = 0; /* End string by '\0' */
		printk("%s", app_buf);
/*
        if ((len == 2) && (app_buf[0] == 0xd) && (app_buf[1] == 0xa))
            printk("Get New Line From Master Side\r\n");
        else
            printk("Get Message From Master Side : \"%s\" [len : %d]\r\n", app_buf, len);
*/
        /* Get tx buffer from RPMsg */
        tx_buf = rpmsg_lite_alloc_tx_buffer(my_rpmsg, &size, RL_BLOCK);
        assert(tx_buf);
        /* Copy string to RPMsg tx buffer */
        memcpy(tx_buf, app_buf, len);
        /* Echo back received message with nocopy send */
        result = rpmsg_lite_send_nocopy(my_rpmsg, my_ept, remote_addr, tx_buf, len);
        if (result != 0)
        {
			printk("rpmsg_lite_send_nocopy failed.\n");
            assert(false);
        }
        /* Release held RPMsg rx buffer */
        result = rpmsg_queue_nocopy_free(my_rpmsg, rx_buf);
        if (result != 0)
        {
			printk("rpmsg_queue_nocopy_free failed.\n");
            assert(false);
        }
    }

}


static struct k_thread thread_rp__client_data;
K_THREAD_STACK_DEFINE(thread_rp__client_stack, APP_TASK_STACK_SIZE);

#define DT_DRV_COMPAT nxp_imx_mu_rev2

static k_tid_t runRpmsgList();

int main(void)
{

/*	my_debug_out("main()\n"); */

	/* k_sleep(K_MSEC(1000));  */

#if 0
	const struct device *mu = DEVICE_DT_GET(DT_DRV_INST(0));
	bool isready = device_is_ready(mu);

	printk("mu is ready: %d\n", isready);
	printk("mu addr: %p\n", mu);
	printk("mu->name: %s\n", mu->name);
	printk("mu->config: %p\n", mu->config);
	printk("mu->api: %p\n", mu->api);
	printk("mu->state: %p\n", mu->state);
	printk("mu->data: %p\n", mu->data);

	if(mu->state)
	{
		printk("mu->state->init_res: %d\n", mu->state->init_res);
		printk("mu->state->initialized: %d\n", mu->state->initialized);
	}
#endif // 0

	/* k_sleep(K_MSEC(5000));
	app_task(NULL,NULL,NULL); */

	k_thread_create(&thread_rp__client_data, thread_rp__client_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, K_NO_WAIT);
			
	return 0;
}

static k_tid_t runRpmsgList()
{

	return k_thread_create(&thread_rp__client_data, thread_rp__client_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, K_NO_WAIT);

}