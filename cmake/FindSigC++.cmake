include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(SigC++_PKGCONF sigc++-3.0)

# Main include dir
find_path(SigC++_INCLUDE_DIR
  NAMES sigc++/sigc++.h
  PATHS ${SigC++_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES sigc++-3.0
)

# Glib-related libraries also use a separate config header, which is in lib dir
find_path(SigC++Config_INCLUDE_DIR
  NAMES sigc++config.h
  PATHS ${SigC++_PKGCONF_INCLUDE_DIRS} /lib
  PATH_SUFFIXES sigc++-3.0/include
)

libfind_library(SigC++ sigc 3.0)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(SigC++_PROCESS_INCLUDES SigC++_INCLUDE_DIR SigC++Config_INCLUDE_DIR)
set(SigC++_PROCESS_LIBS SigC++_LIBRARY)
libfind_process(SigC++)

