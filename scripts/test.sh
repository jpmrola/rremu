#!/bin/sh

buildArgs=""
coverageOn=0
export GTEST_COLOR=1
cd build

while getopts "vc" opt; do
  case $opt in
    v)
      buildArgs="$buildArgs --verbose"
      ;;
    c)
      coverageOn=1
      ;;
    \?)
      exit 1
      ;;
  esac
done

ctest --output-on-failure $buildArgs

if [ $coverageOn -eq 1 ]
then
  lcov --directory . --capture --output-file coverage.info
  lcov --remove coverage.info '*/usr/*' --output-file coverage.info
  lcov --remove coverage.info '*/test/*' --output-file coverage.info
  lcov --remove coverage.info '*/googletest/*' --output-file coverage.info
  genhtml coverage.info --output-directory coverage
  open coverage/index.html
  echo "Coverage report generated in build/coverage/index.html"
fi
