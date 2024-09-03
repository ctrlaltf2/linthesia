cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(linthesia
    VERSION 0.4.2.1
    LANGUAGES CXX
    DESCRIPTION "Synthesia-like software for Linux"
)

# TODO: Release/debug configurations

if(WIN32)
    message(SEND_ERROR "Warning! Win32 builds are untested and not guaranteed to work! (yet)")
endif(WIN32)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTKGLEXTMM REQUIRED gtkglextmm-1.2)
pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)
pkg_check_modules(GCONFMM REQUIRED gconfmm-2.6)
pkg_check_modules(ALSA REQUIRED alsa)

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-dynamic")

file(GLOB SOURCES "src/*.cpp")
add_executable(linthesia ${SOURCES})

set_target_properties(linthesia
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY bin
)

target_compile_features(linthesia
    PUBLIC
        cxx_auto_type
        cxx_nullptr
        cxx_std_17
)

# Add definitions for project info
target_compile_definitions(linthesia
    PUBLIC
        -DCMAKE_PROJECT_NAME="${PROJECT_NAME}";
        -DCMAKE_PROJECT_VERSION="${PROJECT_VERSION}";
        -DCMAKE_PROJECT_BUGREPORT="23644849+ctrlaltf2@users.noreply.github.com";
)

# Asset locations
target_compile_definitions(linthesia
    PUBLIC
        -DGRAPHDIR="${PROJECT_SOURCE_DIR}/graphics"
        -DSCRIPTDIR="${PROJECT_SOURCE_DIR}/graphics"
)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    target_compile_definitions(linthesia
        PUBLIC
            -DDEBUG
    )
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

target_include_directories(linthesia
    PUBLIC
        include
        include/libmidi
        ${GTKGLEXTMM_INCLUDE_DIRS}
        ${GTKMM_INCLUDE_DIRS}
        ${GCONFMM_INCLUDE_DIRS}
        ${ALSA_INCLUDE_DIRS}
)

target_compile_options(linthesia
    PUBLIC
        -Wall
        -Wextra
        -Werror=return-type
        -O2
        ${GTKGLEXTMM_CFLAGS}
        ${GTKMM_CFLAGS}
        ${GCONFMM_CFLAGS}
        ${ALSA_CFLAGS}
)

target_link_libraries(linthesia
    pthread
    ${GTKGLEXTMM_LIBRARIES}
    ${GTKMM_LIBRARIES}
    ${GCONFMM_LIBRARIES}
    ${ALSA_LIBRARIES}
)
