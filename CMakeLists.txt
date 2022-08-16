cmake_minimum_required(VERSION 3.10.0)
project(WsServer)


#SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -O0 -g -ggdb -Wl,-rpath=./libs -lz")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -O0 -g -ggdb ")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -O3 -Wl,-rpath=./libs -lz")

add_definitions(-D__LINUX_OS__)
add_definitions(-DRAPIDJSON_HAS_STDSTRING)
#add_definitions(-D__WINDOWS_OS__)
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message("this is debug")
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
    message("this is release")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(WsServer_NAME WsServerd)
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
    set(WsServer_NAME WsServer)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

file(GLOB SRC_FILE 
	src/*.cpp
	src/util/*.cpp
	src/services/*.cpp
	src/http/*.cpp
	src/http/*.c
)

include_directories(
	src/
	src/util/
	src/services/
	rapidjson/
	src/http/
	src/util/
	spdlog/
	paho.mqtt.c/src/
	spdlog/include/
) 

# if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
#     set(STREAMSDK_FILE streamsdkd)
# elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
#     set(STREAMSDK_FILE streamsdk)
# endif()

set(SYSTEM_FILE 
	#dl 
	pthread
)

link_directories(${CMAKE_SOURCE_DIR}/libs
#/usr/local/cuda/lib64
)

message(${CMAKE_SOURCE_DIR})

add_executable(WsServer ${SRC_FILE})

target_link_libraries(
	WsServer
	spdlog
	paho-mqtt3c
	stdc++fs
	pthread
	dl
)

#set(INSTALL_PATH ${CMAKE_SOURCE_DIR}/install/linux)

#set(INSTALL_BIN_PATH ${INSTALL_PATH}/bin)
#set(INSTALL_LIB_PATH ${INSTALL_PATH}/lib)

