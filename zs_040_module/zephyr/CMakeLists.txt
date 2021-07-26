# SPDX-License-Identifier: Apache-2.0
if(CONFIG_ZS_040_DRIVER)
  # Add zs_040_driver.h to the set of global include paths.
  zephyr_include_directories(.)

  zephyr_library()
  zephyr_library_sources(
    zs_040_driver.c
    )
endif()
