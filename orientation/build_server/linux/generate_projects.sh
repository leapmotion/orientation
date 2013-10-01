#!/bin/bash -xe

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh

\vncserver :94107 -localhost || echo "Existing VNC, hopefully it belongs to ${USER}"
export DISPLAY=:94107.0

source "${BUILD_SCRIPT_COMMON_DIR}"/generate_projects.sh
