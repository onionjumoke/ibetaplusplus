# Cross-compilation for Raspberry Pi Zero W / Pi 1 (ARM1176JZF-S, armv6zk + VFPv2)
# using a Bootlin armv6-eabihf glibc toolchain.
#
# Usage:
#   BPP_TOOLCHAIN=/path/to/armv6-eabihf--glibc--stable-XXXX \
#   cmake -B build-armv6 -DCMAKE_TOOLCHAIN_FILE=$(pwd)/cmake/armv6-rpi.cmake \
#         -DMC_TERMINAL=ON -DCMAKE_BUILD_TYPE=Release
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(NOT DEFINED ENV{BPP_TOOLCHAIN})
	message(FATAL_ERROR "Set BPP_TOOLCHAIN to the extracted Bootlin toolchain root")
endif()
set(TOOLCHAIN_ROOT "$ENV{BPP_TOOLCHAIN}" CACHE PATH "Bootlin toolchain root")

file(GLOB _cross_gcc "${TOOLCHAIN_ROOT}/bin/*-gcc")
if(NOT _cross_gcc)
	message(FATAL_ERROR "No *-gcc found under ${TOOLCHAIN_ROOT}/bin")
endif()
list(GET _cross_gcc 0 _gcc_path)
string(REGEX REPLACE "-gcc$" "" _prefix "${_gcc_path}")
message(STATUS "Cross prefix: ${_prefix}")

set(CMAKE_C_COMPILER   "${_prefix}-gcc")
set(CMAKE_CXX_COMPILER "${_prefix}-g++")

file(GLOB _sysroot_candidates "${TOOLCHAIN_ROOT}/*/sysroot" "${TOOLCHAIN_ROOT}/sysroot")
if(_sysroot_candidates)
	list(GET _sysroot_candidates 0 CMAKE_SYSROOT)
endif()

# ARM1176JZF-S on the Pi Zero W: ARMv6ZK with VFPv2, hard-float ABI.
set(ARM_FLAGS "-march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT   "${ARM_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${ARM_FLAGS}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Don't run test programs during compiler checks.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
