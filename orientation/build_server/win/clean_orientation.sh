#!/bin/bash -xe

#windows version of clean_orientation.sh build step

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh


source "${BUILD_SCRIPT_DIR}"/../common/clean_orientation.sh
