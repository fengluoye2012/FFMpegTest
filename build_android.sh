#!/bin/bash
NDK=/Users/mac/Library/Android/sdk/ndk-r17c
ISYSROOT=$NDK/sysroot

# armv7
SYSROOT=$NDK/platforms/android-21/arch-arm
ASM=$ISYSROOT/usr/include/arm-linux-androideabi
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64
PREFIX=$(pwd)/android/armv7-a
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-

CPU=arm64

build_android()
{
    ./configure \
    --prefix=$PREFIX \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-avdevice \
    --disable-symver \
    --cross-prefix=$CROSS_PREFIX \
    --target-os=android \
    --arch=arm \
    --enable-cross-compile \
    --sysroot=$SYSROOT \
    --extra-cflags="-I$ASM -isysroot $ISYSROOT -D__ANDROID_API__=21 -Os -fpic -marm -march=armv7-a"
    
    # make clean
    # make
    # make install
}

build_android

# NDK=/Users/mac/Library/Android/sdk/ndk-r17c
# ISYSROOT=$NDK/sysroot
# # arm64
# SYSROOT=$NDK/platforms/android-21/arch-arm64
# #arm64
# ASM=$ISYSROOT/usr/include/aarch64-linux-android
# # arm64 
# TOOLCHAIN=$NDK/toolchains/aarch64-linux-android-4.9/prebuilt/darwin-x86_64
# PREFIX=$(pwd)/android/arm64-a
# #arm64
# CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-androideabi-

# export TMPDIR="/Users/mac/Desktop/tem"


# build_android()
# {
#     ./configure \
#     --prefix=$PREFIX \
#     --enable-shared \
#     --disable-static \
#     --disable-doc \
#     --disable-ffmpeg \
#     --disable-ffplay \
#     --disable-ffprobe \
#     --disable-avdevice \
#     --disable-symver \
#     --cross-prefix=$CROSS_PREFIX \
#     --target-os=android \
#     --arch=arm64 \
#     --enable-cross-compile \
#     --sysroot=$SYSROOT \
#     --extra-cflags="-I$ASM -isysroot $ISYSROOT -D__ANDROID_API__=21 -Os -fpic -marm -march=arm64-a "  

# #   --arch=aarch64

#     # make clean
#     # make
#     # make install
# }

# build_android


