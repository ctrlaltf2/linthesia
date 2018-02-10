cmake_minimum_required(VERSION 3.1)
project(linthesia)

# TODO: Version stuff
# TODO: Custom script and graphics paths
# TODO: CPack with tgz, deb, rpm output (based on OS/tool detection?)
# TODO: Check gettimeofday library function (whatever that means)
# TODO: Check compiler features

set(CMAKE_CXX_FLAGS "-Wall")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-dynamic")
# TEMPORARY
add_definitions(-DHAVE_CONFIG_H)
# TODO: This is broken in some places according to CMake 3.1 docs
add_definitions(-DGRAPHDIR="${PROJECT_SOURCE_DIR}/graphics")
add_definitions(-DSCRIPTDIR="${PROJECT_SOURCE_DIR}/graphics")

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_definitions(-DDEBUG)
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

# TODO: Detect library support and gtkmm, atkmm etc existence
include_directories(include /usr/include/gtkmm-2.4 /usr/include/glibmm-2.4 /usr/lib/glibmm-2.4/include /usr/include/glib-2.0 /usr/lib/glib-2.0/include /usr/include/sigc++-2.0 /usr/lib/sigc++-2.0/include /usr/include/giomm-2.4 /usr/lib/giomm-2.4/include /usr/include/gdkmm-2.4 /usr/include/gtk-2.0 /usr/include/cairo /usr/include/pango-1.0 /usr/lib/gtk-2.0/include /usr/include/gdk-pixbuf-2.0 /usr/lib/gdkmm-2.4/include /usr/lib/pangomm-1.4/include /usr/include/cairomm-1.0 /usr/include/freetype2 /usr/include/pangomm-1.4 /usr/include/atk-1.0 /usr/lib/gtkmm-2.4/include /usr/include/atkmm-1.6 /usr/include/gtkglextmm-1.2 /usr/lib/gtkglextmm-1.2/include /usr/lib/gtkglext-1.0/include /usr/include/gtkglext-1.0 /usr/include/gconfmm-2.6 /usr/include/gconf/2 include/libmidi)
file(GLOB SOURCES "src/*.cpp")

add_executable(linthesia ${SOURCES})
target_link_libraries(linthesia pthread gconfmm-2.6 gconf-2 gtkglextmm-x11-1.2 gdkglextmm-x11-1.2 gtkglext-x11-1.0 gdkglext-x11-1.0 GLU GL Xmu Xt pangox-1.0 X11 gmodule-2.0 gtkmm-2.4 atkmm-1.6 gdkmm-2.4 giomm-2.4 pangomm-1.4 glibmm-2.4 cairomm-1.0 sigc-2.0 gtk-x11-2.0 gdk-x11-2.0 pangocairo-1.0 atk-1.0 cairo gdk_pixbuf-2.0 gio-2.0 pangoft2-1.0 pango-1.0 gobject-2.0 glib-2.0 fontconfig freetype asound)
