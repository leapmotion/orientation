#!/bin/bash

#
# local_test.sh is for running local tests off of the build server.
# it requires that your machine be set up appropriately.
# this is not for general use.
# this script should not be run on the build server.
# the build step scripts should be used in separate build steps.
#


_script_name=$(basename "${0}")
_script_dir=$(dirname "${0}")
cd "${_script_dir}"

_steps="generate_sdk build_orientation package_orientation"

export _num_cpus=$(sysctl hw.ncpu | awk '{print $2}')
_num_cpus=$((_num_cpus*2 - 2))

function runSteps() {
  for step in ${_steps}; do
    for crt in "libstdc++" "libc++"; do

      export BUILD_CRT="${crt}"
      if [ "${step}" == "generate_installer" -a "${BUILD_CRT}" == "libc++" ]; then
        echo "Ignoring build step ${step}[${BUILD_CRT}]"
      else
        echo "Running build step ${step}[${BUILD_CRT}]"
        if ./${step}.sh; then
          echo "build step ${step}[${BUILD_CRT}] succeeded."
        else
          1>&2 echo "buid step ${step}[${BUILD_CRT}] failed."
          exit 1
        fi
      fi
    done
  done
}

2>&1 runSteps | tee "${_script_name}".log
