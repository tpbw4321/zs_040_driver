/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define CONFIG_UART_INTERRUPT_DRIVEN

#include "zs_040_driver.h"
#include <stdio.h>
#include <zephyr.h>
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

const struct device *dev;
volatile bool data_read = false;
/* The devicetree node identifier for the "led0" alias. */

void uart_rx_cb(const struct device *dev, void *user_data)
{
	const struct uart_driver_api * uart_api = dev->api;
	uart_api->irq_update(dev);

	while(!(uart_api->irq_rx_ready(dev)));

	if (uart_api->fifo_read(dev, user_data, 256)) {
		data_read = true;
	}
}

int uart_poll_tx(const struct device *dev, uint8_t *data, size_t len) {
	const struct uart_driver_api * uart_api;
	size_t sent = 0;

	if (!dev) {
		return -ENODEV;
	}

	if (!data) {
		return -EFAULT;
	}

	uart_api = dev->api;

	for (sent = 0; sent < len; sent++) {
		uart_api->poll_out(dev, data[sent]);
	}

	return sent;
}

int zs_040_sent_at_cmd()
{
	
}

void main(void)
{
	int ret = 0;
	char rx_buffer[256] = {0};
	uint8_t tx[4] = "AT\r\n";
	int len = 0;

	const struct uart_driver_api * uart_api;

	dev = BT_UART;

	if(!dev) {
		printk("Uart is broke\r\n");
	}

	uart_api = dev->api;
	uart_api->irq_rx_enable(dev);
	uart_api->irq_callback_set(dev, uart_rx_cb, rx_buffer);

	len = uart_poll_tx(dev, tx, sizeof(tx));

	printk("Sent %u\r\n", len);

	while (1) {
		if (data_read) {
			printk("%s", rx_buffer);
			data_read = false;
		} 

		// k_sleep(K_MSEC(1));
	}
}

