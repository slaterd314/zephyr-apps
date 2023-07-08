#define DT_DRV_COMPAT zephyr_memory_debug
#include <zephyr/device.h>
#include <zephyr/sys/cbprintf.h>
#include <zephyr/sys/printk.h>
#include <stdarg.h>


static char *const debug_buffer = (char *)(DT_REG_ADDR(DT_DRV_INST(0)));	// 0x80000000;
/* vsprintf_imp will advance the buffer pointer passed in by an
	arbitrary amount. We're assuming it will not advance more than 64k
	at a time, so we'll reset index when we get within 512KB of the end of the buffer.
*/
static const size_t buffer_size = DT_REG_SIZE(DT_DRV_INST(0)) - 0x80000;	// 0x1000000;	// 16MB - 512KB
static size_t index = 0;

/**
 * @brief my_debug_out Output a formatted printf string to the debug buffer.
 * 
 * @param fmt format string
 * @param ... arguments
 * 
 * @note This function is intended to be used for debugging purposes only.
 * 	 The debug buffer is a memory region that is shared between the
 * 	 Cortex-M4 and Cortex-A53 cores. The Cortex-M4 core is responsible
 * 	 for writing to the debug buffer. The Cortex-A53 cores are responsible
 * 	 for reading from the debug buffer from linux. THis buffer can be read
 * 	 from linux using the /dev/mem device.
 *   We avoid using much stack space by using the shared memory buffer directly.
 *   We also avoid using any system services at all. As a result, this function
 *   can be used during the earliest stages of boot, before the kernel is
 *   fully initialized.
 */

void my_debug_out_v(const char *fmt, va_list a)
{
	char *buffer = debug_buffer + index;
	va_list args;
	va_copy(args, a);
	int count = vsnprintk(buffer, buffer_size - index, fmt, args);
	va_end(args);
	index += count;
	if(index >= buffer_size)
	{
		/* if we happen to get exactly to the end of the buffer
		/ vsnprintk won't add the terminating null character */
		debug_buffer[index] = '\0';
		index = 0;
	}	
}

void my_debug_out(const char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	my_debug_out_v(fmt, args);
	va_end(args);
#if 0	
	char *buffer = debug_buffer + index;
	va_list args;
	va_start(args, fmt);
	int count = vsnprintk(buffer, buffer_size - index, fmt, args);
	va_end(args);
	index += count;
	if(index >= buffer_size)
	{
		/* if we happen to get exactly to the end of the buffer
		/ vsnprintk won't add the terminating null character */
		debug_buffer[index] = '\0';
		index = 0;
	}
	#endif
}
