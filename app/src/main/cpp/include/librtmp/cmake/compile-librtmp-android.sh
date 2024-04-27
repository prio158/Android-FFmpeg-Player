#! /bin/sh

RTMP_ARCH=$1
if [ -z "$RTMP_ARCH" ]; then
    echo "You must specific an architecture 'armv7a, arm64-v8a...'."
    echo ""
    exit 1
fi

NDK=/Users/chenzirui/Library/Android/sdk/ndk/23.1.7779620

TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64

API=21

# replace this path to your android NDK standalone toolchain ptah.
RTMP_TOOLCHAIN_PATH=${TOOLCHAIN}
RTMP_BUILD_ROOT=`pwd`
RTMP_BIN_ROOT=$RTMP_BUILD_ROOT/bin
RTMP_ARCH_DIR=
cd ..


compile_armv7a() {
	export CC=$RTMP_TOOLCHAIN_PATH/bin/armv7a-linux-androideabi21-clang
	export LD=$RTMP_TOOLCHAIN_PATH/bin/lld
	export AR=$RTMP_TOOLCHAIN_PATH/bin/llvm-ar
	export ARCH_CFLAGS=-march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb
	make clean
	make all
}

compile_armv8a() {
	export CC=$RTMP_TOOLCHAIN_PATH/bin/aarch64-linux-android${API}-clang
	export LD=$RTMP_TOOLCHAIN_PATH/bin/lld
	export AR=$RTMP_TOOLCHAIN_PATH/bin/llvm-ar
	export ARCH_CFLAGS=-march=armv8-a -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb
	make clean
	make all
}


release_librtmp() {
	if [ "$RTMP_ARCH" = "armv7a" ]; then
		RTMP_ARCH_DIR=armeabi-v7a
	elif [ "$RTMP_ARCH" = "arm64-v8a" ]; then
		RTMP_ARCH_DIR=arm64-v8a
	else
		RTMP_ARCH_DIR=$RTMP_ARCH
	fi
	mkdir -p        $RTMP_BIN_ROOT/$RTMP_ARCH_DIR/lib
	cp -f librtmp.a $RTMP_BIN_ROOT/$RTMP_ARCH_DIR/lib
	mkdir -p        $RTMP_BIN_ROOT/$RTMP_ARCH_DIR/include/librtmp
	cp -f amf.h     $RTMP_BIN_ROOT/$RTMP_ARCH_DIR/include/librtmp
	cp -f http.h    $RTMP_BIN_ROOT/$RTMP_ARCH_DIR/include/librtmp
	cp -f log.h     $RTMP_BIN_ROOT/$RTMP_ARCH_DIR/include/librtmp
	cp -f rtmp.h    $RTMP_BIN_ROOT/$RTMP_ARCH_DIR/include/librtmp
}

if [ "$RTMP_ARCH" = "armv7a" ]; then
	compile_armv7a
	release_librtmp

elif [ "$RTMP_ARCH" = "arm64-v8a" ]; then
	compile_armv8a
	release_librtmp

elif [ "$RTMP_ARCH" = "all" ]; then
	cd tools
	sh compile-librtmp-android.sh armv7a
	sh compile-librtmp-android.sh arm64-v8a


else
    echo "unknown architecture $RTMP_ARCH";
    exit 1
fi