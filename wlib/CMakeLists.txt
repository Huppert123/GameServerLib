project(wlib)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

file(GLOB_RECURSE ALL_CXX_SRCS
./*.cpp
)

add_library(${PROJECT_NAME} STATIC ${ALL_CXX_SRCS} )
target_link_libraries(${PROJECT_NAME})

