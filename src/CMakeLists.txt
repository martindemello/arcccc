include_directories (
  ${ARCCC_SOURCE_DIR}/src
  ${ARCCC_SOURCE_DIR}/src/lib
  ${GLIB_INCLUDEDIR} 
  ${GLIB_INCLUDE_DIRS}
)

link_directories ( 
  ${ARCCC_BINARY_DIR}/src
  ${ARCCC_BINARY_DIR}/src/lib
)

add_executable ( arccc arccc.c )
target_link_libraries (arccc libarccc ${GLIB_LIBRARIES} )

add_subdirectory ( lib )
