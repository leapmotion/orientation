# This script is to be sourced, not executed

_default_build_arch=x64
_default_build_plat=linux
_default_lib_path="/opt/local/Libraries"

export SHARE_ROOT
if [ -z "${SHARE_ROOT}" ]; then
  SHARE_ROOT="/var"
fi


#must source this first. done in 2 steps to be space-safe.
_plat_script_dir=$(dirname "${0}")
source "${_plat_script_dir}"/../common/environ.sh

#find the required build tools
export INTROJUCER=$(which Introjucer)
if [ ! -x "${INTROJUCER}" ]; then
  INTROJUCER="${_default_lib_path}"/juce/bin/Introjucer
fi

