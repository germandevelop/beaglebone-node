# - Try to find Pango
# Once done, this will define
#
#  Pango_FOUND - system has Pango
#  Pango_INCLUDE_DIRS - the Pango include directories
#  Pango_LIBRARIES - link these to use Pango
include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Pangomm_PKGCONF pangomm-2.48)

# Include dir
find_path(Pangomm_INCLUDE_DIR
  NAMES pangomm.h
  PATHS ${Pangomm_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES pangomm-2.48
)

find_path(PangommConfig_INCLUDE_DIR
  NAMES pangommconfig.h
  PATHS ${Pangomm_PKGCONF_INCLUDE_DIRS} /lib
  PATH_SUFFIXES pangomm-2.48/include
)


libfind_library(Pangomm pangomm 2.48)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Pangomm_PROCESS_INCLUDES Pangomm_INCLUDE_DIR PangommConfig_INCLUDE_DIR)
set(Pangomm_PROCESS_LIBS Pangomm_LIBRARY)
libfind_process(Pangomm)