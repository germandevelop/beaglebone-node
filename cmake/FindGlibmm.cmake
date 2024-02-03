
# - Try to find Glib-2.0 (with gobject)
# Once done, this will define
#
#  Glib_FOUND - system has Glib
#  Glib_INCLUDE_DIRS - the Glib include directories
#  Glib_LIBRARIES - link these to use Glib
include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Glibmm_PKGCONF glibmm-2.68)

# Main include dir
find_path(Glibmm_INCLUDE_DIR
  NAMES glibmm.h
  PATHS ${Glibmm_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES glibmm-2.68
)

# Glib-related libraries also use a separate config header, which is in lib dir
find_path(GlibmmConfig_INCLUDE_DIR
  NAMES glibmmconfig.h
  PATHS ${Glibmm_PKGCONF_INCLUDE_DIRS} /lib
  PATH_SUFFIXES glibmm-2.68/include
)

# Finally the library itself
find_library(Glibmm_LIBRARY
  NAMES glibmm-2.68
  PATHS ${Glibmm_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Glibmm_PROCESS_INCLUDES Glibmm_INCLUDE_DIR GlibmmConfig_INCLUDE_DIR)
set(Glibmm_PROCESS_LIBS Glibmm_LIBRARY)
libfind_process(Glibmm)