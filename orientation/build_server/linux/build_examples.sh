#!/bin/bash -xe

#linux version of build_examples.sh build step

set -e

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh

export PROJECT_FIND_ARGS='-name "Linux"'
export MAKE=$(which make)
build_cpp_example() {
  if [ ! -n "${_num_threads}" ]; then
    _num_threads=1
  fi
  echo Building example project $project
  ( 
    pushd "${project}" &&
    "${MAKE}" --jobs=${_num_threads}  CONFIG="Release" TARGET_ARCH="-m32" &&
    cp -f build/$(basename $(dirname $(dirname $(pwd)))) ../../
  )
}

# Cannot create the Linux UnitySandbox from within Linux, so copy a cached build
export UNITY_SANDBOX_BINARY_SHARE="/var/Builds/UnitySandbox"

_num_cpus=$(nproc)

source "${BUILD_SCRIPT_COMMON_DIR}"/build_examples.sh
exit 0
