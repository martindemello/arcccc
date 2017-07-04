include_directories (
  ${ARCCC_SOURCE_DIR}/src/lib
  ${GLIB_INCLUDEDIR} 
  ${GLIB_INCLUDE_DIRS}
)

link_directories (
  ${ARCCC_BINARY_DIR}/src/lib
)

add_library (
  libarccc backtracking.c constraint.c init_vars.c read_grid.c
)

target_link_libraries (libarccc ${GLIB_LIBRARIES})

# Enable ExternalProject CMake module
include(ExternalProject)

# Set default ExternalProject root directory
set_directory_properties(PROPERTIES EP_PREFIX ${ARCCC_BINARY_DIR}/src/rust)

# Add rust lib as a CMake target
ExternalProject_Add(
  dict_reader
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND cargo build COMMAND cargo build --release
  BINARY_DIR "${ARCCC_SOURCE_DIR}/src/rust"
  INSTALL_COMMAND ""
  LOG_BUILD ON)

# Create dependency of libarccc on rust lib
add_dependencies(libarccc dict_reader)

# Specify rust libraries to link into libarccc
target_link_libraries(libarccc
  debug "${ARCCC_SOURCE_DIR}/src/rust/target/debug/libdict_reader.a"
  optimized "${ARCCC_SOURCE_DIR}/src/rust/target/release/libdict_reader.a"
  gobject-2.0 glib-2.0 glib-2.0 util dl rt pthread gcc_s c m rt pthread util)
