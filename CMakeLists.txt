cmake_minimum_required( VERSION 3.1 )

project( BitChat )

set( CMAKE_CXX_STANDARD 14 )

find_package( Boost 1.54 COMPONENTS program_options system filesystem log REQUIRED )

aux_source_directory( ${PROJECT_SOURCE_DIR} ${RPOJECT_NAME}_sources )
add_executable( ${PROJECT_NAME} ${${RPOJECT_NAME}_sources} )

target_compile_definitions( ${PROJECT_NAME} PRIVATE BOOST_ALL_DYN_LINK )
target_include_directories( ${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR} )
target_link_libraries( ${PROJECT_NAME} LINK_PRIVATE ${Boost_LIBRARIES} pthread )
