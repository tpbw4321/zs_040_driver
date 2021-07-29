/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __ZS_040_DRIVER_H__
#define __ZS_040_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <device.h>

struct zs_040_config_s {
	uint8_t mac_address[6];
	uint8_t role;
	char * name;
	const struct device *uart_dev;
};

struct uart_rx_buffer_s {
	char buffer[256];
	uint8_t write_idx;
};

struct zs_040_data_s {
	struct uart_rx_buffer_s uart_rx_buffer;
	struct k_timer uart_rx_timer;
};

__subsystem struct zs_040_driver_api {
	int (*send_at_command)(const struct device *dev, uint8_t *data, size_t len);
};

__syscall     int zs_040_send_command(const struct device *dev, uint8_t *data, size_t len);
static inline int z_impl_zs_040_send_command(const struct device *dev, uint8_t *data, size_t len)
{
	const struct zs_040_driver_api *api = dev->api;

	__ASSERT(api->send, "Callback pointer should not be NULL");
	__ASSERT(data, "Data pointer should not be NULL");

	return api->send_at_command(dev, data, len);
}

#ifdef __cplusplus
}
#endif

#include <syscalls/zs_040_driver.h>

#endif /* __ZS_040_DRIVER_H__ */
