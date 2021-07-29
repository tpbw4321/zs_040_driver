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
	struct zs_040_data_s *zs_040_data = user_data;
	struct uart_rx_buffer_s *uart_rx_buffer = &(zs_040_data->uart_rx_buffer);
	char data = 0;

	k_timer_stop(&(zs_040_data->uart_rx_timer));
	uart_api->irq_update(dev);

	while(!(uart_api->irq_rx_ready(dev)));

	while (uart_api->fifo_read(dev, &data, 1)) {
		uart_rx_buffer->buffer[(uart_rx_buffer->write_idx)++] = data;
	}

	k_timer_start(&(zs_040_data->uart_rx_timer), K_MSEC(10), K_NO_WAIT);
}

int _uart_poll_tx(const struct device *dev, uint8_t *data, size_t len) {
	const struct uart_driver_api * uart_api = dev->api;
	size_t sent = 0;

	for (sent = 0; sent < len; sent++) {
		while(!uart_api->irq_tx_ready);
		uart_api->poll_out(dev, data[sent]);
	}

	k_sleep(K_MSEC(10));

	return sent;
}

static void zs_040_uart_rx_timer_cb(struct k_timer *timer_id)
{
	struct zs_040_data_s *data = k_timer_user_data_get(timer_id);

	if (data) {
		printk("%s", data->uart_rx_buffer.buffer);
		memset(&(data->uart_rx_buffer), 0, sizeof(struct uart_rx_buffer_s));
	}

	return;
}


static int init(const struct device *dev)
{
	struct zs_040_data_s *data = dev->data;
	struct zs_040_config_s *config = (struct zs_040_config_s *) dev->config;
	const struct uart_driver_api * uart_api;

	config->uart_dev = (const struct device *) BT_UART;

	uart_api = config->uart_dev->api;
	uart_api->irq_rx_enable(config->uart_dev);
	uart_api->irq_callback_set(config->uart_dev, _uart_rx_cb, data);

	// Setup timer for rx timeout
	k_timer_init(&(data->uart_rx_timer), zs_040_uart_rx_timer_cb, NULL);
	k_timer_user_data_set(&(data->uart_rx_timer), data);
	return 0;
}

static int zs_040_send_command_impl(const struct device *dev, uint8_t *data, size_t len)
{
	const struct zs_040_config_s *config = dev->config;
	return _uart_poll_tx(config->uart_dev, data, len);
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
};

struct zs_040_config_s config = {
	.mac_address = {0x98,0xD3,0xA1,0xFD,0x74,0xF2},
	.name = "HC-05",
	.role = 0,
};

struct zs_040_data_s data = {
	.uart_rx_buffer = {
		.buffer = {0}, 
		.write_idx = 0,
		},
};

DEVICE_DEFINE(zs_040, "zs_040",
		    init, NULL, &data, &config,
		    PRE_KERNEL_2, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &zs_040_api);
