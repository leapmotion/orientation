#!/bin/bash

set -e
usage()
{
cat << EOF
usage: $0 options

OPTIONS:
  -s <path>          Optional path to source folder
EOF
}

SOURCE_PATH=.
while getopts "s:" OPTION
do
  case $OPTION in
    h)
      usage
      exit 1
      ;;
    s)
      SOURCE_PATH=$OPTARG
      ;;
  esac
done
cd $SOURCE_PATH
BUILD_NUMBER=$(git log --pretty=format:'x' | sed -e '$a\' | wc -l | tr -d ' ')
echo $BUILD_NUMBER
exit 0
