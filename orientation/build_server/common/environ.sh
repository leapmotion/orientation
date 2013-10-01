#Common version of environ.sh environment setup.
#this file is intended to be sourced, not executed.

if [ ! -n "${_default_build_arch}" -o ! -n "${_default_build_plat}" -o ! -n "${_default_lib_path}" -o ! -n "${SHARE_ROOT}" ]; then
  1>&2 echo "Error: _default_build_arch, _default_build_plat, _default_lib_path and SHARE_ROOT must be set before sourcing common/environ.sh"
  exit 1
fi

if [ ! -x "${TAR}" ]; then
  export TAR="$(which tar)"
fi

if [ ! -x "${TAR}" ]; then
  echo "Error: tar required to build and package orientation."
  exit 1
fi

export BUILD_STEP=$(basename "${0}")

#BUILD_ARCH should be set in jenkins execute shell script build step
#before invocation of build step.

export BUILD_ARCH
if [ ! -n "${BUILD_ARCH}" ]; then
  BUILD_ARCH=${_default_build_arch}
  echo "Warning: BUILD_ARCH was not set. Defaulting to ${BUILD_ARCH}"
fi

if [ ! -n "${BUILD_PLAT}" ]; then
  export BUILD_PLAT=${_default_build_plat}
  echo "Warning: BUILD_PLAT was not set. Defaulting to ${BUILD_PLAT}"
fi

export BUILD_BRANCH=${GIT_BRANCH}
if [ ! -n "${BUILD_BRANCH}" ]; then
  export BUILD_BRANCH="origin/develop"
  echo "Warning: GIT_BRANCH and BUILD_BRANCH were not set. Defaulting to ${BUILD_BRANCH}"
fi

export BRANCH_NAME=$(basename ${BUILD_BRANCH})

export BRANCH_BASE_NAME

case ${BRANCH_NAME} in
release-*) BRANCH_BASE_NAME=release;;
hotfix-*) BRANCH_BASE_NAME=hotfix;;
*) BRANCH_BASE_NAME=${BRANCH_NAME};;
esac

export ORIENTATION_REPO_DIR
if [ -d "${WORKSPACE}" ]; then
  ORIENTATION_REPO_DIR="${WORKSPACE}"
else
  echo "Warning: ${0} invoked outside of Jenkins environment. YMMV."
  ORIENTATION_REPO_DIR=$(dirname ${0})/../..
fi

ORIENTATION_REPO_DIR=$(cd "${ORIENTATION_REPO_DIR}" && pwd)

#libraries location
export LIBRARIES_PATH
if [ ! -n "${LIBRARIES_PATH}" ]; then
	LIBRARIES_PATH=${_default_lib_path}
	echo "No Libraries path found, setting to provided default"
fi

export BUILD_PRODUCTS_DIR
if [ -z "${BUILD_PRODUCTS_DIR}" ]; then
  BUILD_PRODUCTS_DIR="${SHARE_ROOT}/Builds/BuildProducts"
fi

_sdk_dir="${BUILD_PRODUCTS_DIR}/Public/${BUILD_PLAT}"

if [ -z "${PLATFORM_BRANCH}" ]; then
  PLATFORM_BRANCH="develop"
  echo "Platform branch not specified, defaulting to ${PLATFORM_BRANCH}"
fi

if [ -z "${PLATFORM_BUILD}" ]; then
  echo "Platform build not specified, defaulting to latest"
  _most_recent_archive=`ls -t "${_sdk_dir}/${PLATFORM_BRANCH}" | grep 'LeapSDK_.*\.tgz' | head -1`
  PLATFORM_BUILD=`echo ${_most_recent_archive} | cut -d'+' -f 2 | cut -d'.' -f 1`
  PLATFORM_SDK_ARCHIVE="${_sdk_dir}/${PLATFORM_BRANCH}/${_most_recent_archive}"
  unset _most_recent_archive
else
  PLATFORM_SDK_ARCHIVE=`find "${_sdk_dir}/${PLATFORM_BRANCH}" -name "LeapSDK_${PLATFORM_BRANCH}_Public_${BUILD_PLAT}_${BUILD_ARCH}_*+${PLATFORM_BUILD}.tgz"`
fi

if [ ! -f "${PLATFORM_SDK_ARCHIVE}" ]; then
  echo "Error: no valid platform sdk archive to build orientation from."
  exit 1
fi

export PLATFORM_VERSION=`basename ${PLATFORM_SDK_ARCHIVE%.tgz} | awk -F'_' '{ print $6}'`
export PLATFORM_VERSION_MAJOR=`echo ${PLATFORM_VERSION} | awk -F'[\.\+]' '{ print $1}'`
export PLATFORM_VERSION_MINOR=`echo ${PLATFORM_VERSION} | awk -F'[\.\+]' '{ print $2}'`
export PLATFORM_VERSION_PATCH=`echo ${PLATFORM_VERSION} | awk -F'[\.\+]' '{ print $3}'`
export PLATFORM_VERSION_BUILD=`echo ${PLATFORM_VERSION} | awk -F'[\.\+]' '{ print $4}'`

#export location
export BUILD_SHARE="${SHARE_ROOT}/Common/SDK"
export BUILD_PRODUCTS_SHARE="${BUILD_PRODUCTS_DIR}/Public/${BUILD_PLAT}/${BRANCH_BASE_NAME}"

#copy the nessecary files to the LEAP_SDK directory
export LEAP_SDK_LOCAL="${ORIENTATION_REPO_DIR}"/../LeapSDK

if [ ! -n "${LEAP_BUILD_NUMBER}" ]; then
  export LEAP_BUILD_NUMBER="${PLATFORM_VERSION}"
fi

#location of build step scripts
export BUILD_SCRIPT_DIR="${ORIENTATION_REPO_DIR}"/build_server/${BUILD_PLAT}
export BUILD_SCRIPT_COMMON_DIR="${ORIENTATION_REPO_DIR}/build_server/common"

export ORIENTATION_BUILD_NUMBER=$(cd "${BUILD_SCRIPT_COMMON_DIR}" && ./get_build_number.sh)
export ORIENTATION_VERSION=$(echo ${LEAP_BUILD_NUMBER} | sed "s/_.*/_${ORIENTATION_BUILD_NUMBER}/")
export BUILD_IDENTIFIER="${BRANCH_NAME}_public_${BUILD_PLAT}_${BUILD_ARCH}_${LEAP_BUILD_NUMBER}_ex${ORIENTATION_BUILD_NUMBER}"

#verified file copy
function vcp() {
  if cp -v "${@}"; then
    return 0
  fi
  
  1>&2 "Error: File copy failed - Exiting."
  
  exit 1
}

cat << EOF
BUILD_ARCH=${BUILD_ARCH}
BUILD_PLAT=${BUILD_PLAT}
BRANCH_NAME=${BRANCH_NAME}
BRANCH_BASE_NAME=${BRANCH_BASE_NAME}
BUILD_SHARE=${BUILD_SHARE}
BUILD_PRODUCTS_SHARE=${BUILD_PRODUCTS_SHARE}
LEAP_BUILD_NUMBER=${LEAP_BUILD_NUMBER}
ORIENTATION_BUILD_NUMBER=${ORIENTATION_BUILD_NUMBER}
ORIENTATION_VERSION=${ORIENTATION_VERSION}
ORIENTATION_REPO_DIR=${ORIENTATION_REPO_DIR}
BUILD_IDENTIFIER=${BUILD_IDENTIFIER}
BUILD_SCRIPT_DIR=${BUILD_SCRIPT_DIR}
LEAP_SDK_LATEST_DIR=${LEAP_SDK_LATEST_DIR}
EOF
