#!/bin/sh

CMAKE_OPTS=""
RUN_TESTS=0

while getopts "nrtd:" opt; do
  case $opt in
    n)
      # fresh build
      rm -rf ./build
      mkdir build
      ;;
    r)
      # release build
      RELEASE_STR="-DCMAKE_BUILD_TYPE=Release"
      CMAKE_OPTS="$CMAKE_OPTS $RELEASE_STR"
      ;;
    t)
      # test build
      TEST_STR="-DCMAKE_BUILD_TYPE=Test"
      CMAKE_OPTS="$CMAKE_OPTS $TEST_STR"
      ;;
    d)
      # debug build
      if [ "$OPTARG" = "none" ]; then
        DEBUG_STR="-DCMAKE_BUILD_TYPE=Debug -DSANITIZER="
        CMAKE_OPTS="$CMAKE_OPTS $DEBUG_STR"
        continue
      elif [ "$OPTARG" = "address" ]; then
        DEBUG_STR="-DCMAKE_BUILD_TYPE=Debug -DSANITIZER=address"
        CMAKE_OPTS="$CMAKE_OPTS $DEBUG_STR"
        continue
      elif [ "$OPTARG" = "leak" ]; then
        DEBUG_STR="-DCMAKE_BUILD_TYPE=Debug -DSANITIZER=leak"
        CMAKE_OPTS="$CMAKE_OPTS $DEBUG_STR"
        continue
      elif [ "$OPTARG" = "undefined" ]; then
        DEBUG_STR="-DCMAKE_BUILD_TYPE=Debug -DSANITIZER=undefined"
        CMAKE_OPTS="$CMAKE_OPTS $DEBUG_STR"
        continue
      fi
      ;;
    \?)
      echo "Usage: $0 [-n] [-r] [-t] [-d]"
      echo "  -n: fresh build"
      echo "  -r: release build"
      echo "  -t: test build"
      echo "  -d: debug build"
      exit 1
      ;;
  esac
done

cd build
cmake $CMAKE_OPTS ..
make -j 10
