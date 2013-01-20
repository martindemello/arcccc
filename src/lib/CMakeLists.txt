include_directories (
  ${ARCCC_SOURCE_DIR}/src/lib
  ${GLIB_INCLUDEDIR} 
  ${GLIB_INCLUDE_DIRS}
)

link_directories (
  ${ARCCC_BINARY_DIR}/src/lib
)

add_library (
  libarccc arccc.cpp constraint.cpp read_grid.cpp read_words.cpp arccc_wrapper.cpp
)

target_link_libraries ( libarccc ${GLIB_LIBRARIES} )
