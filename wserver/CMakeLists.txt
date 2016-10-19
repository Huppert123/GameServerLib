project(wserver)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
MESSAGE(STATUS "#########################")

file(GLOB ALL_CXX_SRCS
./*.cpp
)

add_executable(${PROJECT_NAME} ${ALL_CXX_SRCS} )
add_dependencies(${PROJECT_NAME} wlib)
add_dependencies(${PROJECT_NAME} wshare)

set(ALL_LIB  pthread
	libwlib.a
	libwshare.a
	libboost_system.a
	)

target_link_libraries(${PROJECT_NAME} ${ALL_LIB})


