#!/bin/sh
set -e

cd "$(dirname "$0")"

export RUSTFLAGS="-C link-arg=-Wl,-z,max-page-size=16384"

build_crate() {
  crate="$1"

  (
    cd "$crate"

    rustup run nightly cargo ndk \
      -t arm64-v8a \
      -t armeabi-v7a \
      -t x86_64-linux-android \
      -o ../../libs \
      build \
      -Z build-std=std,panic_abort \
      --release
  )
}

build_crate oxipng_jni
build_crate ntsc_jni
build_crate imagequant_jni
build_crate vvc_jni