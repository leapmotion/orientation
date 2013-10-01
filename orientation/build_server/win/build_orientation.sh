#!/bin/bash -xe

#windows version of build_examples.sh build step

set -e

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh

#Build the cpp files

#run msvc & build the cpp projects
_msb_plat="Win32"
_msb_config="Release"
if [ "${BUILD_ARCH}" = "x64" ]; then
  _msb_plat="x64"
fi

export PROJECT_FIND_ARGS='-name "*.sln" -path "*VisualStudio2010*"'

build_cpp_orientation() {
  local project="$1"
  echo Building orientation project $project
  "${MAKE}" "${project}" -Build "${_msb_config}|${_msb_plat}"
  projdir=$(dirname $project)
  projname=$(basename $project)
  projname="${projname%.*}"
  cp -f "${projdir}"/Release/{*.exe,*.dll} "${projdir}"/../../
} 

source "${BUILD_SCRIPT_DIR}"/../common/build_orientation.sh

exit 0
