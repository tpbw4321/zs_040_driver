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

struct zs_040_config {
	uint8_t mac_address[6];
	uint8_t role;
	char * name;
	const struct device *uart_dev;
};

struct zs_040_data {
	volatile bool data_read;
	char rx_buffer[256];
};
__subsystem struct zs_040_driver_api {
	void (*send_at_command)(const struct device *dev, uint8_t *data, size_t len);
	void (*recv_at_response)(const struct device *dev);
};

__syscall     void zs_040_send_command(const struct device *dev, uint8_t *data, size_t len);
static inline void z_impl_zs_040_send_command(const struct device *dev, uint8_t *data, size_t len)
{
	const struct zs_040_driver_api *api = dev->api;

	__ASSERT(api->send, "Callback pointer should not be NULL");
	__ASSERT(data, "Data pointer should not be NULL");

	api->send_at_command(dev, data, len);
}

__syscall     void zs_040_recv_command(const struct device *dev);
static inline void z_impl_zs_040_recv_command(const struct device *dev)
{
	const struct zs_040_driver_api *api = dev->api;

	__ASSERT(api->send, "Callback pointer should not be NULL");
	__ASSERT(data, "Data pointer should not be NULL");

	api->recv_at_response(dev);
}

#ifdef __cplusplus
}
#endif

#include <syscalls/zs_040_driver.h>

#endif /* __ZS_040_DRIVER_H__ */
