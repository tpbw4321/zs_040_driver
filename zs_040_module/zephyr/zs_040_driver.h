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

/*
 * This 'Hello World' driver has a 'print' syscall that prints the
 * famous 'Hello World!' string.
 *
 * The string is formatted with some internal driver data to
 * demonstrate that drivers are initialized during the boot process.
 *
 * The driver exists to demonstrate (and test) custom drivers that are
 * maintained outside of Zephyr.
 */
__subsystem struct zs_040_driver_api {
	void (*send)(const struct device *dev);
};

__syscall     void zs_040_send_command(const struct device *dev);
static inline void z_impl_zs_040_send_command(const struct device *dev)
{
	const struct zs_040_driver_api *api = dev->api;

	__ASSERT(api->send, "Callback pointer should not be NULL");

	api->send(dev);
}

#ifdef __cplusplus
}
#endif

#include <syscalls/zs_040_driver.h>

#endif /* __ZS_040_DRIVER_H__ */
