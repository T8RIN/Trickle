#!/bin/sh
set -e

cd "$(dirname "$0")"

cargo ndk \
  -t arm64-v8a \
  -t armeabi-v7a \
  -t x86_64-linux-android \
  -o ../libs \
  build --release

(
  cd ntsc_jni
  cargo ndk \
    -t arm64-v8a \
    -t armeabi-v7a \
    -t x86_64-linux-android \
    -o ../../libs \
    build --release
)
