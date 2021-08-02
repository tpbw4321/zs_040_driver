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
#define BT_EN_DEV DEVICE_DT_GET(DT_PHANDLE_BY_IDX(BT_SERIAL, gpios, 0))
#define BT_EN_PIN DT_PHA_BY_IDX(BT_SERIAL, gpios, 0, pin)
#define BT_EN_FLAGS DT_PHA_BY_IDX(BT_SERIAL, gpios, 0, flags)
#define BT_STATE_DEV DEVICE_DT_GET(DT_PHANDLE_BY_IDX(BT_SERIAL, gpios, 1))
#define BT_STATE_PIN DT_PHA_BY_IDX(BT_SERIAL, gpios, 1, pin)
#define BT_STATE_FLAGS DT_PHA_BY_IDX(BT_SERIAL, gpios, 1, flags)
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
	
	return sent;
}

static void zs_040_uart_rx_timer_cb(struct k_timer *timer_id)
{
	struct zs_040_data_s *data = k_timer_user_data_get(timer_id);

	if (data) {
		printk("%s", data->uart_rx_buffer.buffer);
		if (data->recv_cb_handler) {
			data->recv_cb_handler(data->dev, data->uart_rx_buffer.buffer, strlen(data->uart_rx_buffer.buffer));
		}
		memset(&(data->uart_rx_buffer), 0, sizeof(struct uart_rx_buffer_s));
	}

	return;
}

static void zs_040_state_int_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	const struct device *zs_040 = device_get_binding("zs_040");
	struct zs_040_data_s *data = zs_040->data;
	int pin_state = 0;

	if (cb->pin_mask == BIT(BT_STATE_PIN)) {
		pin_state = gpio_pin_get(dev, BT_STATE_PIN);
		if (pin_state) {
			data->state = ZS_040_CONNECTED_STATE;
		} else {
			data->state = ZS_040_CONFIG_STATE;
		}
	}
}


static int init(const struct device *dev)
{
	struct zs_040_data_s *data = dev->data;
	struct zs_040_config_s *config = (struct zs_040_config_s *) dev->config;
	const struct uart_driver_api * uart_api;

	data->dev = dev;
	data->state = ZS_040_CONFIG_STATE;
	config->uart_dev = (const struct device *) BT_UART;
	uart_api = config->uart_dev->api;
	uart_api->irq_rx_enable(config->uart_dev);
	uart_api->irq_callback_set(config->uart_dev, _uart_rx_cb, data);

	// Setup timer for rx timeout
	k_timer_init(&(data->uart_rx_timer), zs_040_uart_rx_timer_cb, NULL);
	k_timer_user_data_set(&(data->uart_rx_timer), data);

	// Set enable pin
	gpio_pin_configure(BT_EN_DEV, BT_EN_PIN, GPIO_OUTPUT);
	k_sleep(K_MSEC(1000));
	gpio_pin_set(BT_EN_DEV, BT_EN_PIN, 1);

	//Set state pin
	gpio_pin_configure(BT_STATE_DEV, BT_STATE_PIN, GPIO_INPUT);
	gpio_pin_interrupt_configure(BT_STATE_DEV, BT_STATE_PIN, GPIO_INT_EDGE_BOTH);
	gpio_init_callback(&(data->state_gpio_cb_data), zs_040_state_int_cb, BIT(BT_STATE_PIN));
	gpio_add_callback(BT_STATE_DEV, &(data->state_gpio_cb_data));

	return 0;
}

static int zs_040_send_command_impl(const struct device *dev, uint8_t *data, size_t len)
{
	const struct zs_040_config_s *config = dev->config;
	return _uart_poll_tx(config->uart_dev, data, len);
}

static void set_data_recv_cb_impl(const struct device *dev, zs_040_data_recv_cb_t handler)
{
	struct zs_040_data_s *data = dev->data;
	data->recv_cb_handler = handler;
	return;
}

struct zs_040_driver_api zs_040_api = { 
	.send_at_command = zs_040_send_command_impl,
	.set_data_recv_cb = set_data_recv_cb_impl,
};

struct zs_040_config_s config = {
	.mac_address = {0x98,0xD3,0xA1,0xFD,0x74,0xF2},
	.name = "HC-05",
	.role = 0,
};

struct zs_040_data_s data = {
	.state = ZS_040_CONFIG_STATE,
	.uart_rx_buffer = {
		.buffer = {0}, 
		.write_idx = 0,
		},
};

DEVICE_DEFINE(zs_040, "zs_040",
		    init, NULL, &data, &config,
		    APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &zs_040_api);
