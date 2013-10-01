#!/bin/bash -xe

#Linux version of package_examples.sh build step.  
#The different platforms have almost nothing in common, so there is no common script

export BUILD_SCRIPT_DIR=$(dirname "${0}")
source "${BUILD_SCRIPT_DIR}"/environ.sh

PACKAGE_TARGET="LeapDeveloperKit_${PLATFORM_BRANCH}_linux_${EXAMPLES_VERSION}.tgz"

cd $EXAMPLES_REPO_DIR

if [ -a $PACKAGE_TARGET ]; then
  rm -f $PACKAGE_TARGET
fi

TEMP_DIR="LeapDeveloperKit"
if [ -d $TEMP_DIR ]; then
  rm -rf "${TEMP_DIR}*"
fi

mkdir -p $TEMP_DIR/Examples

COPY_DIRS="FingerVisualizer MotionVisualizer UnitySandbox ThirdParty README.txt"
REMOVE_PATTERN='-name "*.sh" -or -name "README_INTERNAL.txt" -or -name ".gitignore" \
-or -path "*VisualStudio2005*" \
-or -path "*VisualStudio2008*" \
-or -path "*VisualStudio2010*" \
-or -path "*MacOSX*" \
-or -path "*Linux/build*" \
-or -path "*UnitySandbox/*" -and ! -path "*UnitySandbox/UnitySandbox*" -and ! -path "*UnitySandbox/README.txt*" \
-or -name "win" -or -name "mac" -or -path "*/Windows/*"'

cp -R $COPY_DIRS $TEMP_DIR/Examples

eval find $TEMP_DIR/Examples/ "${REMOVE_PATTERN}" | while read file
do
  echo removing "$file"
  rm -rf "$file"
done

mkdir -p "${TEMP_DIR}"/Examples/ThirdParty/JUCE/bin/linux
cp -f "${LIBRARIES_PATH}"/juce/bin/Introjucer "${TEMP_DIR}"/Examples/ThirdParty/JUCE/bin/linux/

cp -rf "${LEAP_SDK_LOCAL}" "${TEMP_DIR}"

tar -czvf "${PACKAGE_TARGET}" "${TEMP_DIR}"

rm -rf $TEMP_DIR

exit 0
