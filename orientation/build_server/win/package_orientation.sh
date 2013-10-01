#!/bin/bash -xe

#Windows version of packaage_orientation.sh build step.  
#The different platforms have almost nothing in common, so there is no common script

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh

if [ ! -x "${TAR}" ]; then
  "echo tar command needed for packaging up Orientation"
  exit 1
fi

if [ ! -x "${ZIP}" ]; then
  echo "zip command needed for packaging up Developer Kit"
  exit 1
fi

cd "${ORIENTATION_REPO_DIR}"

#package up orientation for use by installer
_zip_target="orientation.tgz"
_zip_share_target="orientation_${BUILD_IDENTIFIER}.tgz"

if [ -f "${_zip_target}" ]; then
  /bin/rm -f "${_zip_target}"
fi

if "${TAR}" czvf "${_zip_target}" Orientation.exe Leap.dll; then
  vcp -f "${_zip_target}" "${BUILD_PRODUCTS_SHARE}/${_zip_share_target}"
else
  1>&2 echo "Error: Failed archiving ${_zip_target} - Exiting."
  exit 1
fi

exit 0
