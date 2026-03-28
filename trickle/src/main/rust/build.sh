cargo ndk \
  -t arm64-v8a \
  -t armeabi-v7a \
  -t x86_64-linux-android \
  -o ../libs \
  build --release