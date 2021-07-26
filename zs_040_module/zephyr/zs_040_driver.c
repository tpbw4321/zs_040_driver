/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define CONFIG_UART_INTERRUPT_DRIVEN 1

#include <stdio.h>
#include <string.h>
#include "zs_040_driver.h"
#include <zephyr/types.h>
#include <syscall_handler.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>

#define BT_SERIAL DT_NODELABEL(bt_serial)
#if DT_NODE_HAS_PROP(BT_SERIAL, uart)
#define BT_UART DEVICE_DT_GET(DT_PHANDLE(BT_SERIAL, uart))
#define BT_EN_PIN DEVICE_DT_GET(DT_PHANDLE(BT_SERIAL, en_pin))
#define BT_STATE_PIN DEVICE_DT_GET(DT_PHANDLE(BT_SERIAL, state_pin))
#else
#error No uart property in bt_serial
#endif

void _uart_rx_cb(const struct device *dev, void *user_data)
{
	const struct uart_driver_api * uart_api = dev->api;
	struct zs_040_data *data = dev->data;
	char buffer[256] = {0};
	int len = 0;
	uart_api->irq_update(dev);

	while(!(uart_api->irq_rx_ready(dev)));
	len = uart_api->fifo_read(dev, buffer, 256);
	if (len) {
		strncat((char *) user_data, buffer, len);
		data->data_read = true;
	}
}

int _uart_poll_tx(const struct device *dev, uint8_t *data, size_t len) {
	const struct uart_driver_api * uart_api = dev->api;
	size_t sent = 0;

	for (sent = 0; sent < len; sent++) {
		uart_api->poll_out(dev, data[sent]);
	}

	return sent;
}

static int init(const struct device *dev)
{
	struct zs_040_data *data = dev->data;
	struct zs_040_config *config = (struct zs_040_config *) dev->config;
	const struct uart_driver_api * uart_api;

	config->uart_dev = (const struct device *) BT_UART;

	uart_api = config->uart_dev->api;
	uart_api->irq_rx_enable(config->uart_dev);
	uart_api->irq_callback_set(config->uart_dev, _uart_rx_cb, data->rx_buffer);

	return 0;
}

static void zs_040_send_command_impl(const struct device *dev, uint8_t *data, size_t len)
{
	const struct zs_040_config *config = dev->config;
	int sent = 0;
	sent = _uart_poll_tx(config->uart_dev, data, len);
}

static void zs_040_recv_command_impl(const struct device *dev)
{
	struct zs_040_data *data = dev->data;
	while(!data->data_read);

	data->data_read = false;
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_hello_world_print(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_HELLO_WORLD(dev, print));

	z_impl_hello_world_print(dev);
}
#include <syscalls/hello_world_print_mrsh.c>
#endif /* CONFIG_USERSPACE */

struct zs_040_driver_api zs_040_api = { 
	.send_at_command = zs_040_send_command_impl,
	.recv_at_response = zs_040_recv_command_impl,
};

struct zs_040_config config = {
	.mac_address = {0x98,0xD3,0xA1,0xFD,0x74,0xF2},
	.name = "HC-05",
	.role = 0,
};

struct zs_040_data data = {
	.data_read = false,
	.rx_buffer = {0},
};

DEVICE_DEFINE(zs_040, "zs_040",
		    init, NULL, &data, &config,
		    PRE_KERNEL_2, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &zs_040_api);
