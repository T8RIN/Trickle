#!/bin/sh
set -e

cd "$(dirname "$0")"

export RUSTFLAGS="-C link-arg=-Wl,-z,max-page-size=16384"

build_crate() {
  crate="$1"
  shift

  (
    cd "$crate"

    rustup run nightly cargo ndk \
      -t arm64-v8a \
      -t armeabi-v7a \
      -t x86_64-linux-android \
      -o ../../libs \
      build \
      -Z build-std=std,panic_abort \
      --release "$@"
  )
}

build_patched_crate() (
  crate="$1"
  dependency="$2"
  patched_source="$(mktemp -d)"
  trap 'rm -rf "$patched_source"' EXIT
  source_dir="$(
    cargo metadata --format-version 1 --filter-platform aarch64-linux-android --manifest-path "$crate/Cargo.toml" |
      python3 -c 'import json, pathlib, sys
packages = json.load(sys.stdin)["packages"]
print(pathlib.Path(next(p["manifest_path"] for p in packages if p["name"] == sys.argv[1])).parent)' "$dependency"
  )"
  cp -R "$source_dir/." "$patched_source/"
  chmod -R u+w "$patched_source"
  patch -s -d "$patched_source" -p1 < "$crate/cancellation.patch"
  patch_config="patch.crates-io.$dependency.path='$patched_source'"
  rustup run nightly cargo update --manifest-path "$crate/Cargo.toml" -p "$dependency" --config "$patch_config"
  build_crate "$crate" --config "$patch_config"
)

if [ "$#" -eq 0 ]; then
  set -- oxipng_jni ntsc_jni imagequant_jni vvc_jni jhm17_jni
fi

for crate in "$@"; do
  case "$crate" in
    jhm17_jni) build_patched_crate "$crate" jhm17 ;;
    oxipng_jni) build_patched_crate "$crate" oxipng ;;
    vvc_jni) build_patched_crate "$crate" garnetash ;;
    *) build_crate "$crate" ;;
  esac
done