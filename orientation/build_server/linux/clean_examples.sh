#!/bin/bash -xe

#linux version of clean_examples.sh build step

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh


source "${BUILD_SCRIPT_DIR}"/../common/clean_examples.sh

