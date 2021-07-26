/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zs_040_driver.h"
#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <string.h>

const struct device *dev;

void main(void)
{
	const struct zs_040_driver_api * zs_040_api;
	struct zs_040_data *data;

	char *command = "AT\r\n";

	dev = device_get_binding("zs_040");
	
	zs_040_api = dev->api;
	data = dev->data;

	if (!dev) {
		printk("Could not find zs_040 device\r\n");
	}

	k_sleep(K_SECONDS(5));

	zs_040_api->send_at_command(dev, command, sizeof(command));
	
	while (1) {
		printk("%s len: %u\r\n", data->rx_buffer, strlen(data->rx_buffer));
		k_sleep(K_MSEC(1));
	}
}
