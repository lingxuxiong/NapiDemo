# the minimum version of CMake.
cmake_minimum_required(VERSION 3.4.1)
project(NapiDemo)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(AKI_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../oh_modules/@ohos/aki)
set(CMAKE_MODULE_PATH ${AKI_ROOT_PATH})
message("AKI_ROOT_PATH:" + ${AKI_ROOT_PATH})
find_package(Aki REQUIRED)

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)

add_library(entry SHARED hello.cpp async.cpp promise.cpp)
target_link_libraries(entry PUBLIC libace_napi.z.so libhilog_ndk.z.so Aki::libjsbind)