include_directories (
  ${ARCCC_SOURCE_DIR}/src/lib
  ${GLIB_INCLUDEDIR} 
  ${GLIB_INCLUDE_DIRS}
)

link_directories (
  ${ARCCC_BINARY_DIR}/src/lib
)

add_library (
  libarccc backtracking.c constraint.c init_vars.c read_grid.c read_words.c
)
target_link_libraries ( libarccc ${GLIB_LIBRARIES} )
