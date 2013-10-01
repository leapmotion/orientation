#!/bin/bash -xe

if [ "${BUILD_STEP}" != $(basename "${0}") ]; then
  1>&2 echo "Don't run this script directly. use one of the platform-specific generate_sdk.sh wrappers."
  exit 1
fi

if [ ! -x "${TAR}" ]; then
  1>&2 echo "Error: tar ( ${TAR} ) is not an executable."
  exit 1
fi

if [ -d "${LEAP_SDK_LOCAL}" ]; then
  /bin/rm -rf "${LEAP_SDK_LOCAL}"
fi

mkdir -p "${LEAP_SDK_LOCAL}"

(cd "${LEAP_SDK_LOCAL}/.."; "${TAR}" xf "${PLATFORM_SDK_ARCHIVE}")

if [ ! -d "${LEAP_SDK_LOCAL}" ]; then
  echo "Failed extracting LeapSDK"
  exit 1
fi
