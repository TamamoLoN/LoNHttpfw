cmake_minimum_required(VERSION 3.1)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME})
project(lonhttpfw)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_BUILD_TYPE Debug)

if(WIN32)
    message(STATUS "Building on Windows (MSVC)")
    add_definitions(/DWIN32 /D_WINDOWS)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/Zi)
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LoNetfw/thirdparty/win32/pthreads-w32/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LoNetfw/thirdparty/win32/wepoll/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LoNetfw/thirdparty/minhook/include
    )
    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LoNetfw/thirdparty/win32/pthreads-w32/lib/x64
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LoNetfw/thirdparty/win32/wepoll/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LoNetfw/thirdparty/win32/dl/lib
    )
    set(LIB_TYPE STATIC)
else()
    message(STATUS "Building on Linux")
    add_definitions(-g -LINUX_X86)
    set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wno-builtin-macro-redefined -Wno-terminate -Wno-deprecated-declarations")
    set(LIB_TYPE SHARED)
endif()

set(MODULARIZATION OFF CACHE BOOL "" FORCE)
# set(BUILT_TESTS OFF CACHE BOOL "" FORCE)

set(MONGREL2_HTTP_PARSER ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/mongrel2-http11/)

add_subdirectory(thirdparty/LoNetfw)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LoNetfw
${CMAKE_CURRENT_SOURCE_DIR}
${MONGREL2_HTTP_PARSER}/
${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LoNetfw/thirdparty/yaml-cpp/include
)

file(GLOB HTTP_SOURCES 
http/*.cpp
httpservice/*.cpp
${MONGREL2_HTTP_PARSER}/*.cpp
${MONGREL2_HTTP_PARSER}/*.c
)

add_library(lonhttpfw ${LIB_TYPE} ${HTTP_SOURCES}
)
target_link_libraries(lonhttpfw PUBLIC lonetfw)
if(USE_ABSOLUTE_PATH)
else()
    force_redefine_file_macro_for_sources(lonhttpfw)
endif()

function(add_lonhttpfw_build_target targetname)
    add_executable(${targetname}
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/${targetname}.cpp
                )
    target_link_libraries(${targetname} PRIVATE 
                            lonhttpfw
                        ) 
    if(USE_ABSOLUTE_PATH)
    else()
        force_redefine_file_macro_for_sources(${targetname})
    endif()
endfunction()

if(BUILT_TESTS)
    add_lonhttpfw_build_target(test_http)
    add_lonhttpfw_build_target(test_http_server)
    add_lonhttpfw_build_target(test_http_client)
    add_lonhttpfw_build_target(test_http_uri)
    add_lonhttpfw_build_target(test_http_connectionpool)
endif()

install(TARGETS lonhttpfw
            ARCHIVE DESTINATION lib 
            LIBRARY DESTINATION lib 
            RUNTIME DESTINATION bin)

install(DIRECTORY 
        http
        httpservice
        lonhttpfw
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)