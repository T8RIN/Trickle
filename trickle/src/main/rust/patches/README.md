# Rust dependency patches

These patches add cooperative cancellation inside the pinned upstream codecs and NTSC effect. The JNI wrappers poll the calling thread, and the patches let the algorithms stop while work is in progress.

Run `sh build.sh` from the parent directory to rebuild the Android libraries. The script applies each patch to a temporary copy of the dependency. Include the patches and rebuilt JNI binaries when committing an update.
