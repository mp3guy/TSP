cmake_minimum_required(VERSION 2.6.0)
project(TSP)

find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS thread REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIR})

add_definitions(-Wall)

set(CMAKE_CXX_FLAGS "-O3 -msse2 -msse3")

FILE(GLOB srcs *.cpp)
source_group("Source Files" FILES ${srcs} )

add_executable(TSP
               ${srcs}
)

target_link_libraries(TSP
                      ${OpenCV_LIBS}
                      ${Boost_LIBRARIES})
