cmake_minimum_required(VERSION 3.21)

project(BetaPlusPlus
    VERSION 1.7.3
    DESCRIPTION "Minecraft Beta 1.7.3 C++ port"
    LANGUAGES C CXX
)

# Keep third-party projects small: BetaPlusPlus only needs their static libraries.
set(SDL_SHARED OFF CACHE BOOL "Build the SDL shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build the SDL static library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Build the SDL test library" FORCE)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Build zlib examples" FORCE)

# Terminal variant (headless kitty-graphics rendering): no window system is
# used at runtime, so SDL is stripped to its dummy backends and GL itself is
# dlopened (OSMesa/EGL) instead of linked.
option(MC_TERMINAL "Build headless terminal-render variant (kitty graphics protocol)" OFF)

# Legacy iOS 5 cross-build (armv7, UIKit + EAGL + GLES11Shim). Replaces
# glad/SDL2/SDL_net with the pc/ios compat layer; see src/pc/ios/.
option(MC_IOS "Build for legacy iOS 5 (armv7, UIKit + EAGL)" OFF)

if(MC_IOS)
    enable_language(OBJCXX)
endif()

if(MC_TERMINAL)
    set(SDL_X11 OFF CACHE BOOL "" FORCE)
    set(SDL_WAYLAND OFF CACHE BOOL "" FORCE)
    set(SDL_KMSDRM OFF CACHE BOOL "" FORCE)
    set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
    set(SDL_OPENGL OFF CACHE BOOL "" FORCE)
    set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
    set(SDL_ALSA OFF CACHE BOOL "" FORCE)
    set(SDL_PULSEAUDIO OFF CACHE BOOL "" FORCE)
    set(SDL_JACK OFF CACHE BOOL "" FORCE)
    set(SDL_PIPEWIRE OFF CACHE BOOL "" FORCE)
    set(SDL_SNDIO OFF CACHE BOOL "" FORCE)
    set(SDL_NAS OFF CACHE BOOL "" FORCE)
    set(SDL_DBUS OFF CACHE BOOL "" FORCE)
    set(SDL_IBUS OFF CACHE BOOL "" FORCE)
    set(SDL_LIBUDEV OFF CACHE BOOL "" FORCE)
    set(SDL_RPI OFF CACHE BOOL "" FORCE)
endif()

if(NOT MC_IOS)
    add_subdirectory(external/glad EXCLUDE_FROM_ALL)
    add_subdirectory(external/SDL2 EXCLUDE_FROM_ALL)
endif()
add_subdirectory(external/zlib EXCLUDE_FROM_ALL)

if(NOT MC_IOS)
    # SDL_net links against the SDL2 static target created above, so it must be
    # added afterwards. Force a static build and skip its samples; CMP0077 lets the
    # normal variable override SDL_net's own option() defaults.
    set(BUILD_SHARED_LIBS OFF)
    set(SDL2NET_SAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/SDL_net EXCLUDE_FROM_ALL)
endif()

if(NOT MC_IOS)
    find_package(Threads REQUIRED)
endif()
if(NOT MC_TERMINAL AND NOT MC_IOS)
    find_package(OpenGL REQUIRED)
endif()

# These are the port's own translation units. Platform implementations are
# selected below so Windows and Linux versions are never linked together.
file(GLOB_RECURSE MCBETA_COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE MCBETA_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

if(WIN32)
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "[/\\\\]pc[/\\\\]linux[/\\\\]")
elseif(UNIX)
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "[/\\\\]pc[/\\\\]win32[/\\\\]")
else()
    message(FATAL_ERROR "BetaPlusPlus currently supports Windows, Linux and legacy iOS only")
endif()

# pc/ios is iOS-only: it shadows SDL/glad through include-path tricks that do
# not exist on desktop targets.
if(NOT MC_IOS)
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "[/\\\\]pc[/\\\\]ios[/\\\\]")
endif()

# pc/terminal is only built for the headless terminal-render variant.
if(NOT MC_TERMINAL)
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "[/\\\\]pc[/\\\\]terminal[/\\\\]")
endif()

# Terminal variant swaps the SDL-window Display/GLContext implementations for
# the offscreen terminal ones (same lwjgl:: namespaces).
if(MC_TERMINAL AND UNIX)
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "pc/lwjgl/(Display|GLContext)\\.cpp$")
endif()

# Legacy iOS: pc/ios replaces the SDL lwjgl layer, POSIX java layer and entry
# point; miniaudio has no legacy-iOS backend (SDL-audio compat is used instead).
if(MC_IOS)
    file(GLOB_RECURSE MCBETA_IOS_OBJCXX CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pc/ios/*.mm"
    )
    list(APPEND MCBETA_COMMON_SOURCES ${MCBETA_IOS_OBJCXX})

    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "[/\\\\]pc[/\\\\](linux|win32)[/\\\\]")
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "[/\\\\]pc[/\\\\]lwjgl[/\\\\]")
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "[/\\\\]pc[/\\\\]terminal[/\\\\]")
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "[/\\\\]pc[/\\\\]Main\\.cpp$")
    list(FILTER MCBETA_COMMON_SOURCES EXCLUDE REGEX "miniaudio\\.cpp$")
endif()

add_executable(BetaPlusPlus
    ${MCBETA_COMMON_SOURCES}
    ${MCBETA_HEADERS}
)

if(MC_TERMINAL)
    set_target_properties(BetaPlusPlus PROPERTIES OUTPUT_NAME "BetaPlusPlus-terminal")
endif()

target_compile_features(BetaPlusPlus PRIVATE cxx_std_17)
set_target_properties(BetaPlusPlus PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
if(MC_IOS)
    set_target_properties(BetaPlusPlus PROPERTIES
        OBJC_STANDARD 11
        OBJCXX_STANDARD 17
        OBJCXX_STANDARD_REQUIRED YES
        OBJCXX_EXTENSIONS NO
    )
endif()

# Link-Time Optimization for Release. The port is built from hundreds of tiny
# cross-translation-unit accessors (getBlockId, getSavedLightValue, Chunk /
# NibbleArray getters) called millions of times per frame in the hot loops;
# without LTO the compiler cannot inline them across .cpp boundaries.
#
# OFF by default: MinGW's bundled ld can fail the final link with
# "plugin needed to handle lto object" because it isn't invoked with GCC's LTO
# linker plugin. The bulk of the codegen win already comes from -O2/-O3 (per-TU
# inlining); LTO only adds the cross-TU part. Turn this on (and make sure the
# toolchain uses gcc-ar / gcc-ranlib and -fuse-linker-plugin) to chase the rest.
option(MCBETA_ENABLE_LTO "Enable Link-Time Optimization for Release builds" OFF)
if(MCBETA_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error LANGUAGES CXX)
    if(ipo_supported)
        set_target_properties(BetaPlusPlus PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    else()
        message(WARNING "IPO/LTO requested but not supported by this toolchain, skipping: ${ipo_error}")
    endif()
endif()

target_include_directories(BetaPlusPlus PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pc"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/stb"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio"
)

if(MC_TERMINAL)
    # Shadow SDL2's SDL_opengl.h with a glad-forwarding shim so GL calls bind
    # to glad's runtime-loaded pointers instead of libGL link symbols.
    target_include_directories(BetaPlusPlus BEFORE PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pc/terminal/include"
    )
endif()

if(MC_IOS)
    # Must come FIRST: SDL/glad includes resolve to the pc/ios compat shims.
    target_include_directories(BetaPlusPlus SYSTEM PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pc/ios"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pc/ios/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/cxxabi-include"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/include"
    )

    # armv7-apple-ios5.0 has no user-mode TLS; stb falls back to statics.
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/external/stb_image.cpp" PROPERTIES
        COMPILE_DEFINITIONS "STBI_NO_THREAD_LOCALS")

    # ── Vendored static libc++ (see external/libcxx-ios/) ────────────────────
    # The iOS 5 system libc++.dylib predates modern headers; a complete
    # static runtime is built from vendored LLVM 19.1.7 sources instead, with
    # an iOS-5 POSIX backport replacing upstream's openat()-based filesystem.
    file(GLOB LIBCXX_IOS_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/upstream/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/upstream/filesystem/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/chrono-ios.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/future_stub.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/cxa_shims.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/filesystem/ios_operations_backport.cpp"
    )
    add_library(libcxx_ios_static STATIC ${LIBCXX_IOS_SOURCES})
    target_compile_features(libcxx_ios_static PRIVATE cxx_std_20)
    # NOTE: no -fno-rtti here — the exception classes' typeinfo/vtables must be
    # emitted into this archive because iOS 5's libc++abi.dylib predates them.
    target_compile_options(libcxx_ios_static PRIVATE
        -D_LIBCPP_BUILDING_LIBRARY
        -D_LIBCPP_PSTL_BACKEND_SERIAL
    )
    target_include_directories(libcxx_ios_static PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/upstream"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx-ios/src/cxxabi-include"
    )
endif()

if(MC_IOS)
    target_link_libraries(BetaPlusPlus PRIVATE
        zlibstatic
        libcxx_ios_static
        "-framework Foundation"
        "-framework UIKit"
        "-framework CoreGraphics"
        "-framework QuartzCore"
        "-framework OpenGLES"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
        "-lc++abi"
    )
    # Link the vendored static libc++ instead of the SDK's ancient one; the
    # matching headers arrive via the SYSTEM include dir above (-nostdinc++
    # removes clang's own C++ header path).
    target_link_options(BetaPlusPlus PRIVATE -nostdlib++)
    target_compile_options(BetaPlusPlus PRIVATE -nostdinc++)
    target_compile_options(BetaPlusPlus PRIVATE
        $<$<COMPILE_LANGUAGE:OBJC>:-fno-objc-arc>
        $<$<COMPILE_LANGUAGE:OBJCXX>:-fno-objc-arc>
        $<$<COMPILE_LANGUAGE:OBJCXX>:-Wno-deprecated-declarations>
    )
else()
    target_link_libraries(BetaPlusPlus PRIVATE
        glad
        SDL2::SDL2-static
        SDL2_net::SDL2_net-static
        zlibstatic
        Threads::Threads
        ${CMAKE_DL_LIBS}
    )

    if(NOT MC_TERMINAL)
        target_link_libraries(BetaPlusPlus PRIVATE OpenGL::GL)
    endif()
endif()

if(WIN32)
    target_compile_definitions(BetaPlusPlus PRIVATE
        MC_WIN32
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    target_link_libraries(BetaPlusPlus PRIVATE shell32 psapi)

    if(MSVC)
        target_compile_options(BetaPlusPlus PRIVATE /MP /bigobj /utf-8)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(BetaPlusPlus PRIVATE
            -Wa,-mbig-obj
            $<$<CONFIG:Release>:-O2>
        )
        target_link_options(BetaPlusPlus PRIVATE
            -static
            -s
            -fno-ident
            -fno-asynchronous-unwind-tables
            -fvisibility=hidden
        )
    endif()
elseif(UNIX)
    if(MC_IOS)
        target_compile_definitions(BetaPlusPlus PRIVATE MC_IOS)
    else()
        target_compile_definitions(BetaPlusPlus PRIVATE MC_LINUX)
        if(MC_TERMINAL)
            target_compile_definitions(BetaPlusPlus PRIVATE MC_TERMINAL)
        endif()
    endif()
endif()

# Put every configuration in a predictable folder instead of beside CMake's
# generated files. Explicit per-config properties avoid an extra Debug/Debug.
# The iOS cross-build uses the SAME build type (Release) as the Linux GUI, so it
# must NOT write into the source tree's bin/Release — that clobbers the host
# binary with an armv7 Mach-O. Keep iOS outputs inside its own build dir.
if(MC_IOS)
    set(MC_OUT_ROOT "${CMAKE_BINARY_DIR}")
else()
    set(MC_OUT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" config_upper)
    set_target_properties(BetaPlusPlus PROPERTIES
        "RUNTIME_OUTPUT_DIRECTORY_${config_upper}"
        "${MC_OUT_ROOT}/bin/${config}"
    )
endforeach()

if(CMAKE_CONFIGURATION_TYPES)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT BetaPlusPlus)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files"
    FILES ${MCBETA_COMMON_SOURCES} ${MCBETA_HEADERS}
)
