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

int process_command(const struct device *dev, uint8_t * data, size_t len) {
	const struct zs_040_driver_api * zs_040_api = dev->api;
	struct zs_040_data_s *zs_040_data = dev->data;
	char *reply = "CMD_OK\r\n";

	if (ZS_040_CONNECTED_STATE == zs_040_data->state) {
		zs_040_api->send_at_command(dev, reply, strlen(reply));
	}

	return 0;
}

void main(void)
{
	const struct device *dev = device_get_binding("zs_040");
	const struct zs_040_driver_api * zs_040_api;
	
	zs_040_api = dev->api;

	if (!dev) {
		printk("Could not find zs_040 device\r\n");
	}

	zs_040_api->set_data_recv_cb(dev, process_command);

	while (1) {
		k_sleep(K_MSEC(1));
	}
}
