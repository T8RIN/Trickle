#!/bin/sh
set -e

cd "$(dirname "$0")"

export RUSTFLAGS="-C link-arg=-Wl,-z,max-page-size=16384"
output="$PWD/../libs"

build_crate() (
  crate_dir="$1"
  shift
  cd "$crate_dir"

  rustup run nightly cargo ndk \
    -t arm64-v8a \
    -t armeabi-v7a \
    -t x86_64-linux-android \
    -o "$output" \
    build -Z build-std=std,panic_abort --release "$@"
)

build_patched_crate() (
  crate="$1"
  dependency="$2"
  patch_file="$3"
  git_source="${4:-}"
  work="$(mktemp -d)"
  trap 'rm -rf "$work"' EXIT
  source_dir="$(
    cargo metadata --format-version 1 --filter-platform aarch64-linux-android --manifest-path "$crate/Cargo.toml" |
      python3 -c 'import json, pathlib, sys
packages = json.load(sys.stdin)["packages"]
print(pathlib.Path(next(p["manifest_path"] for p in packages if p["name"] == sys.argv[1])).parent)' "$dependency"
  )"

  if [ -n "$git_source" ]; then
    repo="$(git -C "$source_dir" rev-parse --show-toplevel)"
    mkdir "$work/source"
    git -C "$repo" archive HEAD | tar -xf - -C "$work/source"
    patched_source="$work/source/${source_dir#"$repo"/}"
    patch_key="patch.'$git_source'.$dependency.path"
  else
    patched_source="$work/source"
    cp -R "$source_dir" "$patched_source"
    chmod -R u+w "$patched_source"
    patch_key="patch.crates-io.$dependency.path"
  fi

  patch -s -d "$patched_source" -p1 < "patches/$patch_file"
  patch_config="$patch_key='$patched_source'"

  mkdir "$work/$crate"
  cp -R "$crate/Cargo.toml" "$crate/Cargo.lock" "$crate/src" "$work/$crate/"
  cp interruption.rs "$work/"
  export CARGO_TARGET_DIR="${CARGO_TARGET_DIR:-$PWD/$crate/target}"
  rustup run nightly cargo update --offline --manifest-path "$work/$crate/Cargo.toml" -p "$dependency" --config "$patch_config"
  build_crate "$work/$crate" --config "$patch_config" --locked
)

if [ "$#" -eq 0 ]; then
  set -- oxipng_jni ntsc_jni imagequant_jni vvc_jni jhm17_jni
fi

for crate in "$@"; do
  case "$crate" in
    jhm17_jni) build_patched_crate "$crate" jhm17 jhm17-0.1.0.patch ;;
    oxipng_jni) build_patched_crate "$crate" oxipng oxipng-10.1.1.patch ;;
    vvc_jni) build_patched_crate "$crate" garnetash garnetash-0.1.1.patch ;;
    ntsc_jni) build_patched_crate "$crate" ntsc-rs ntsc-rs-add90f5.patch https://github.com/ntsc-rs/ntsc-rs ;;
    *) build_crate "$crate" ;;
  esac
done
