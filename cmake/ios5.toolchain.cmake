# ─────────────────────────────────────────────────────────────────────────────
# ios5.toolchain.cmake — cross-compile BetaPlusPlus for iOS 5 (armv7) with the
# ~/ios-toolchain cctools-port/ld64 toolchain (see iOS5-Guide-Debian.md).
#
# Usage:
#   cmake -B build-ios -G Ninja -DMC_IOS=ON \
#         -DCMAKE_TOOLCHAIN_FILE=cmake/ios5.toolchain.cmake
#
# The ios5-clang wrapper already pins the target triple, sysroot and legacy
# linker, so this file only wires compilers/binutils into CMake. CMAKE_SYSTEM_NAME
# is plain Darwin: CMake stays out of the way while clang's Apple target logic
# does the real work.
# ─────────────────────────────────────────────────────────────────────────────

set(IOS_TOOLCHAIN_ROOT "$ENV{HOME}/ios-toolchain" CACHE PATH "Legacy iOS cross-toolchain root")
set(IOS5_SDK "${IOS_TOOLCHAIN_ROOT}/SDKs/iPhoneOS5.0.sdk" CACHE PATH "iPhoneOS 5.0 SDK")

if(NOT EXISTS "${IOS5_SDK}")
	message(FATAL_ERROR "iOS 5.0 SDK not found at ${IOS5_SDK}")
endif()

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR armv7)

set(CMAKE_C_COMPILER   "${IOS_TOOLCHAIN_ROOT}/bin/ios5-clang" CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "${IOS_TOOLCHAIN_ROOT}/bin/ios5-clang" CACHE FILEPATH "")
set(CMAKE_OBJC_COMPILER   "${IOS_TOOLCHAIN_ROOT}/bin/ios5-clang" CACHE FILEPATH "")
set(CMAKE_OBJCXX_COMPILER "${IOS_TOOLCHAIN_ROOT}/bin/ios5-clang" CACHE FILEPATH "")

set(CMAKE_AR     "${IOS_TOOLCHAIN_ROOT}/bin/arm-apple-darwin11-ar" CACHE FILEPATH "")
set(CMAKE_RANLIB "${IOS_TOOLCHAIN_ROOT}/bin/arm-apple-darwin11-ranlib" CACHE FILEPATH "")
set(CMAKE_STRIP  "${IOS_TOOLCHAIN_ROOT}/bin/arm-apple-darwin11-strip" CACHE FILEPATH "")
set(CMAKE_INSTALL_NAME_TOOL "${IOS_TOOLCHAIN_ROOT}/bin/arm-apple-darwin11-install_name_tool" CACHE FILEPATH "")
set(CMAKE_OTOOL  "${IOS_TOOLCHAIN_ROOT}/bin/arm-apple-darwin11-otool" CACHE FILEPATH "")

# search only inside the SDK for headers/libraries; host programs stay native
set(CMAKE_FIND_ROOT_PATH "${IOS5_SDK}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# bare Mach-O executable, no bundle magic from CMake
set(MACOSX_BUNDLE FALSE)
