#!/bin/bash -xe

#mac version of build_orientation.sh build step

set -e

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh

XCODEBUILD=$(which xcodebuild)

if [ ! -x "${XCODEBUILD}" ]; then
  echo Could not find xcodebuild, cannot compile cpp projects
  exit 1
fi

source "${BUILD_SCRIPT_COMMON_DIR}"/build_orientation.sh
exit 0
