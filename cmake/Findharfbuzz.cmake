# - Try to find harfbuzz
# Once done, this will define
#
#  Harfbuzz_FOUND - system has Harfbuzz
#  Harfbuzz_INCLUDE_DIRS - the Harfbuzz include directories
#  Harfbuzz_LIBRARIES - link these to use Harfbuzz

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Harfbuzz_PKGCONF harfbuzz)

# Include dir
find_path(Harfbuzz_INCLUDE_DIR
  NAMES hb.h
  PATHS ${Harfbuzz_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES harfbuzz
)

# Finally the library itself
find_library(Harfbuzz_LIBRARY
  NAMES harfbuzz
  PATHS ${Harfbuzz_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Harfbuzz_PROCESS_INCLUDES Harfbuzz_INCLUDE_DIR)
set(Harfbuzz_PROCESS_LIBS Harfbuzz_LIBRARY)
libfind_process(Harfbuzz)

