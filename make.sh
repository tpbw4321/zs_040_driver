#!/bin/bash
# rm -rf build
# cmake -B build -DBOARD=stm32_min_dev_blue -DDTC_OVERLAY_FILE="overlay.dts"
cd build && make
