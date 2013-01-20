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

add_executable ( arccc arccc.cpp )
target_link_libraries (arccc libarccc ${GLIB_LIBRARIES} )

add_executable ( test_arccc_wrapper arccc.c )
target_link_libraries (test_arccc_wrapper libarccc ${GLIB_LIBRARIES} )

add_subdirectory ( lib )
