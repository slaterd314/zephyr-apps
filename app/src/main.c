/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <app_version.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

int main(void)
{
	printk("Zephyr Example Application %s\n", APP_VERSION_STRING);
	for(int i=0; i<10; i++)
	{
		k_sleep(K_MSEC(100));
		printk("%d: Zephyr Example Application %s\n", i, APP_VERSION_STRING);
		printk("%d: Hello World! %s\n", i, CONFIG_ARCH);
	}
	return 0;
#if 0	
	int ret;
	const struct device *sensor;

	printk("Zephyr Example Application %s\n", APP_VERSION_STRING);

	sensor = DEVICE_DT_GET(DT_NODELABEL(examplesensor0));
	if (!device_is_ready(sensor)) {
		LOG_ERR("Sensor not ready");
		return 0;
	}

	while (1) {
		struct sensor_value val;

		ret = sensor_sample_fetch(sensor);
		if (ret < 0) {
			LOG_ERR("Could not fetch sample (%d)", ret);
			return 0;
		}

		ret = sensor_channel_get(sensor, SENSOR_CHAN_PROX, &val);
		if (ret < 0) {
			LOG_ERR("Could not get sample (%d)", ret);
			return 0;
		}

		printk("Sensor value: %d\n", val.val1);

		k_sleep(K_MSEC(1000));
	}

	return 0;
#endif
}

