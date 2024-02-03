include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Alsa_PKGCONF alsa)

# Include dir
find_path(Alsa_INCLUDE_DIR
  NAMES asoundlib.h
  PATHS ${Alsa_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES alsa
)

# Finally the library itself
find_library(Alsa_LIBRARY
  NAMES asound
  PATHS ${Alsa_PKGCONF_LIBRARY_DIR}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Alsa_PROCESS_INCLUDES Alsa_INCLUDE_DIR)
set(Alsa_PROCESS_LIBS Alsa_LIBRARY)
libfind_process(Alsa)