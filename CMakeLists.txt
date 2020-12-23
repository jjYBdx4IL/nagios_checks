cmake_minimum_required (VERSION 3.18)

project ("nagios_checks")

set(CMAKE_CXX_STANDARD 17)

set(Boost_DEBUG ON)
set(Boost_USE_STATIC_LIBS ON)
find_package (Boost 1.70 COMPONENTS unit_test_framework log system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
#link_directories(${Boost_LIBRARY_DIRS})
#link_libraries(${Boost_LIBRARIES})
#add_compile_definitions("BOOST_TEST_DYN_LINK=1")

if(UNIX)
  link_libraries(stdc++fs)
endif()


enable_testing()

 
add_subdirectory ("checks")
add_subdirectory ("tests")
