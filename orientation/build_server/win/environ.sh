#windows version of environ.sh environment setup.
#this file is intended to be sourced, not executed.

#when bash is not executed with --login on windows the /bin directory is at the end of the path.
#this is a particular headache when using "find" or other commands with bash/win name collisions.
export PATH=".:/bin:${PATH}"

_default_build_arch=x86
_default_build_plat=win
_default_lib_path="C:/Libraries-x86"
export SHARE_ROOT
if [ -z "${SHARE_ROOT}" ]; then
  SHARE_ROOT="//ocuserv2"
fi

#must source this first. done in 2 steps to be space-safe.
_plat_script_dir=$(dirname "${0}")
source "${_plat_script_dir}/../common/environ.sh"

#BUILD_ARCH should be set in jenkins execute shell script build step
#before invocation of build step.

#find the required build tools
#export INTROJUCER=$(which introjucer)
#if [ ! -x "${INTROJUCER}" ]; then
#  INTROJUCER="${EXAMPLES_REPO_DIR}"/ThirdParty/JUCE/bin/"${BUILD_PLAT}"/Introjucer.exe
#fi

#if [ ! -x "${INTROJUCER}" ]; then
#  echo "Warning: introjucer executable not in path and not installed in default location."
#  echo "		 Can't generate project files."
#  echo INTROJUCER=$INTROJUCER
#  unset INTROJUCER
#fi

#export UNITY=$(which unity)
#if [ ! -x "${UNITY}" ]; then
#  UNITY="${PROGRAMFILES}"/Unity/Editor/Unity.exe
#fi
#if [ ! -x "${UNITY}" ]; then
#  echo "Warning: Unity executable not in path and not installed in default location."
#  echo "         Can't generate project files."
#fi

export ZIP=$(which 7z)
if [ ! -x "${ZIP}" ]; then
  ZIP="${PROGRAMFILES}/7-Zip/7z.exe"
fi
if [ ! -x "${ZIP}" ]; then
  ZIP="${PROGRAMW6432}/7-Zip/7z.exe"
fi
if [ ! -x "${ZIP}" ]; then
  echo "Warning: 7-zip executable not in path and not installed in default location."
  echo "         Can't generate zip files."
fi

export MAKE=$(which devenv.com) || true

if [ ! -x "${MAKE}" ]; then
  MAKE="${PROGRAMFILES//\\//}/Microsoft Visual Studio 11.0/Common7/IDE/devenv.com"
fi

if [ ! -x "${MAKE}" ]; then
  MAKE="${PROGRAMFILES//\\//}/Microsoft Visual Studio 10.0/Common7/IDE/devenv.com"
fi

if [ ! -x "${MAKE}" ]; then
  echo "Error: can't build MSVS projects without devenv.com"
  exit 1
fi
