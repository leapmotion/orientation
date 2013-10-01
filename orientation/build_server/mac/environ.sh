#mac version of environ.sh environment setup.
#this file is intended to be sourced, not executed.

#when bash is not executed with --login on windows the /bin directory is at the end of the path.
#this is a particular headache when using "find" or other commands with bash/win name collisions.

export PATH=".:/opt/local/bin:/usr/local/bin:/usr/bin:/bin:${PATH}"

_default_build_arch=x64
_default_build_plat=mac
_default_lib_path="/opt/local/Libraries"

export SHARE_ROOT
if [ -z "${SHARE_ROOT}" ]; then
  SHARE_ROOT="/Users/tombuilder/LeapMotion/OcuShare"
fi

#must source this first. done in 2 steps to be space-safe.
_plat_script_dir=$(dirname "${0}")
source "${_plat_script_dir}"/../common/environ.sh

export ZIP=$(which tar)
if [ ! -x "${ZIP}" ]; then
  echo "Warning: tar executable not in path and not installed in default location."
  echo "   Can't generate archive files."
fi
