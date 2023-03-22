#!/bin/bash
set -e

for PICO_BOARD in adafruit_feather_rp2040 wiznet_w5100s_evb_pico; do
    BUILD_DIR="build-${PICO_BOARD}"
    cmake -B "${BUILD_DIR}" -DPICO_BOARD="${PICO_BOARD}"
    make -C"${BUILD_DIR}/firmware" -j$(getconf _NPROCESSORS_ONLN)
done
