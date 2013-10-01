#!/bin/bash -xe

#common version of build_orientation.sh build step

set -e

if [ "${BUILD_STEP}" != $(basename "${0}") ]; then
  1>&2 echo "Don't run this script directly. use one of the platform-specific generate_sdk.sh wrappers."
  exit 1
fi

if [ ! -d "${LEAP_SDK_LOCAL}" ]; then
  1>&2 echo "Error: Leap SDK Directory ${LEAP_SDK_LOCAL} does not exit."
  exit 1
fi

# Build Orientation
if [ "${BUILD_PLAT}" == "mac" ]; then 
  cd "${ORIENTATION_REPO_DIR}"/Builds/MacOSX; xcodebuild -project Orientation.xcodeproj
fi

if [ "${BUILD_PLAT}" == "win" ]; then
  export _num_threads=1
  if [ -n "${_num_cpus}" ]; then
    if [ "${_num_cpus}" -gt 2 ]; then
        _num_threads=$((_num_cpus/2))
    fi
  fi
  
  # Build the cpp project
  eval find . ${PROJECT_FIND_ARGS} ! -path Builds -print0 | while read -d $'\0' project
    do
          build_cpp_orientation "${project}"
    done
  fi

  if [ "$?" != "0" ]; then
      echo "CPP project build failed, abort"
      exit -1
  fi

exit 0
