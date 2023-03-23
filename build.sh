#!/bin/bash
set -e


PARALLEL=$(getconf _NPROCESSORS_ONLN)

while [[ -v 1 ]]; do
    case "$1" in

        "--parallel" | "-j")
            shift
            PARALLEL="$1"
            shift
            ;;

        "--verbose"|"-v")
            VERBOSE="VERBOSE=1"
            shift
            ;;

        *)
            echo "unknown argument '$1'"
            exit 1
            ;;

    esac
done


for PICO_BOARD in adafruit_feather_rp2040 wiznet_w5100s_evb_pico; do
    BUILD_DIR="build-${PICO_BOARD}"
    MAKE_ARGS=(
        -C"${BUILD_DIR}/firmware"
        -j"${PARALLEL}"
    )
    if [[ -v VERBOSE ]]; then
        MAKE_ARGS[${#MAKE_ARGS[@]}+1]="${VERBOSE}"
    fi

    cmake -B "${BUILD_DIR}" -DPICO_BOARD="${PICO_BOARD}"
    make "${MAKE_ARGS[@]}"
done
