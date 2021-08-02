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
#include <drivers/gpio.h>

typedef int (*zs_040_data_recv_cb_t)(const struct device *dev, uint8_t *data, size_t len);

struct zs_040_config_s {
	uint8_t mac_address[6];
	uint8_t role;
	char * name;
	const struct device *uart_dev;
	const struct device *en_pin;
	const struct device *state_pin;
};

typedef enum {
	ZS_040_CONFIG_STATE,
	ZS_040_CONNECTED_STATE,
	ZS_040_UNKNOWN_STATE,
} zs_040_state_e;

struct uart_rx_buffer_s {
	char buffer[256];
	uint8_t write_idx;
};

struct zs_040_data_s {
	const struct device 	*dev;
	volatile zs_040_state_e state;
	struct uart_rx_buffer_s uart_rx_buffer;
	struct k_timer 			uart_rx_timer;
	struct gpio_callback  	state_gpio_cb_data;
	zs_040_data_recv_cb_t   recv_cb_handler;
};


__subsystem struct zs_040_driver_api {
	int (*send_at_command)(const struct device *dev, uint8_t *data, size_t len);
	void (*set_data_recv_cb)(const struct device *dev, zs_040_data_recv_cb_t handler);
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
