//
//  uart_irq.c
//
//
//  Created by Barron Wong on 7/24/21.
//

#include "uart_irq.h"
#include <device.h>
#include <misc/printk.h>
#include <uart.h>
#include <zephyr.h>
#define SYS_LOG_DOMAIN "UART_DRIVER"
#include <logging/sys_log.h>

static void (*__uart_callback)(u8_t) = 0;
void ay_uart_driver_set_callback(void (*callback)(u8_t))
{
    __uart_callback = callback;
}
static void uart_fifo_callback(struct device *dev)
{
    u8_t recvData;
    /* Verify uart_irq_update() */
    if (!uart_irq_update(dev)) {
        SYS_LOG_ERR("retval should always be 1");
        return;
    }
    /* Verify uart_irq_rx_ready() */
    if (uart_irq_rx_ready(dev)) {
        /* Verify uart_fifo_read() */
        uart_fifo_read(dev, &recvData, 1);
        if (__uart_callback) {
            __uart_callback(recvData);
        }
    }
}

u8_t ay_uart_driver_open()
{
    struct device *uart_dev = device_get_binding("UART_0");
    if (!uart_dev) {
        SYS_LOG_ERR("Problem to load uart device");
        return 1;
    }
    
    /* Verify uart_irq_callback_set() */
    uart_irq_callback_set(uart_dev, uart_fifo_callback);
    /* Enable Tx/Rx interrupt before using fifo */
    /* Verify uart_irq_rx_enable() */
    uart_irq_rx_enable(uart_dev);
    SYS_LOG_INF("UART device loaded...[OK]");
    return 0;
}

void ay_uart_driver_write(u8_t data)
{
    
}
