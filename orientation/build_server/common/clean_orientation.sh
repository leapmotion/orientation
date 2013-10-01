#!/bin/bash -xe

if [ "${BUILD_STEP}" != $(basename "${0}") ]; then
  1>&2 echo "Don't run this script directly. use one of the platform-specific clean_orientation.sh wrappers."
  exit 1
fi

if [ -d "${LEAP_SDK_LOCAL}" ]; then
  /bin/rm -rf "${LEAP_SDK_LOCAL}"/
fi

if [ -d "${ORIENTATION_REPO_DIR}"/Builds/MacOSX/build ]; then
  /bin/rm -rf "${ORIENTATION_REPO_DIR}"/Builds/MacOSX/build/
fi

if [ -d "${ORIENTATION_REPO_DIR}"/Builds/VisualStudio2010/Release ]; then
  /bin/rm -rf "${ORIENTATION_REPO_DIR}"/Builds/VisualStudio2010/Release
  /bin/rm -rf "${ORIENTATION_REPO_DIR}"/Builds/VisualStudio2010/Orientation.vcxproj.user/
  /bin/rm -rf "${ORIENTATION_REPO_DIR}"/Orientation.exe
  /bin/rm -rf "${ORIENTATION_REPO_DIR}"/Leap.dll
fi

exit 0
