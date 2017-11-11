#This is the cmake file and this part is project info part
cmake_minimum_required (VERSION 3.0)
project (LPM_LIB)
include (CTest)

#set version control
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)

#set DATE_SWITCH control
option (DATE_SWITCH "Use to log date and time" ON)

find_package(Boost 1.45.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(LPM_LIB ${Boost_LIBRARIES})
endif()

#generate library
target_link_libraries(LPM.lib.so dd)
add_library(LPM.lib.so SHARED)
