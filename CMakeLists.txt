cmake_minimum_required ( VERSION 2.6 )
project ( ARCCC )

find_package ( PkgConfig REQUIRED )

pkg_check_modules ( GLIB REQUIRED glib-2.0 ) 

set ( CMAKE_CXX_FLAGS "-g -Wall" )
add_subdirectory ( src )
