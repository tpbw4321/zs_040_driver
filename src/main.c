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

	char *command = "AT+VERSION\r\n";
	char *command2 = "AT+NAME\r\n";

	dev = device_get_binding("zs_040");
	
	zs_040_api = dev->api;
	data = dev->data;

	if (!dev) {
		printk("Could not find zs_040 device\r\n");
	}

	zs_040_api->send_at_command(dev, command, strlen(command));
	zs_040_api->send_at_command(dev, command2, strlen(command2));

	while (1) {
		k_sleep(K_MSEC(1));
	}
}
