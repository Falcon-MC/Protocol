cmake_minimum_required(VERSION 3.16)
project(Falcon.Protocol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(NOT TARGET FalconNBT)
    FetchContent_Declare(
        falcon_nbt
        GIT_REPOSITORY https://github.com/Falcon-MC/NBT.git
        GIT_TAG main
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(falcon_nbt)
endif()

file(GLOB_RECURSE FALCON_PROTOCOL_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(FalconProtocol STATIC ${FALCON_PROTOCOL_SOURCES})

target_include_directories(FalconProtocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(FalconProtocol PUBLIC FalconNBT)
