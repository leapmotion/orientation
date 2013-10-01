#!/bin/bash -xe

#mac version of generate_sdk.sh build step

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh


source "${BUILD_SCRIPT_COMMON_DIR}"/generate_sdk.sh