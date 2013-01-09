include_directories (
  ${ARCCC_SOURCE_DIR}/src/lib
  ${GLIB_INCLUDEDIR} 
  ${GLIB_INCLUDE_DIRS}
)

link_directories (
  ${ARCCC_BINARY_DIR}/src/lib
)

add_library (
  libarccc backtracking.cpp constraint.cpp init_vars.cpp read_grid.cpp read_words.cpp
)
target_link_libraries ( libarccc ${GLIB_LIBRARIES} )
