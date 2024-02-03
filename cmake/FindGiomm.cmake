# - Try to find Pango
# Once done, this will define
#
#  Pango_FOUND - system has Pango
#  Pango_INCLUDE_DIRS - the Pango include directories
#  Pango_LIBRARIES - link these to use Pango
include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Giomm_PKGCONF giomm-2.68)

# Include dir
find_path(Giomm_INCLUDE_DIR
  NAMES giomm.h
  PATHS ${Giomm_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES giomm-2.68
)

find_path(GiommConfig_INCLUDE_DIR
  NAMES giommconfig.h
  PATHS ${Giomm_PKGCONF_INCLUDE_DIRS} /lib
  PATH_SUFFIXES giomm-2.68/include
)

libfind_library(Giomm giomm 2.68)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Giomm_PROCESS_INCLUDES Giomm_INCLUDE_DIR GiommConfig_INCLUDE_DIR)
set(Giomm_PROCESS_LIBS Giomm_LIBRARY)
libfind_process(Giomm)