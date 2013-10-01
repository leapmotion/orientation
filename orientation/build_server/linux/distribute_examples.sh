#!/bin/bash -xe

#linux version of distribute_installer.sh build step

#must source this first. done in 2 steps to be space-safe.
BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh

PACKAGE=$(find "${EXAMPLES_REPO_DIR}" -name "LeapDeveloperKit_${PLATFORM_BRANCH}_linux_*.tgz" -maxdepth 1)
# if more than one file, select the most recent
PACKAGE=$(ls -t ${PACKAGE} | head -1)

source "${BUILD_SCRIPT_COMMON_DIR}"/distribute_examples.sh
