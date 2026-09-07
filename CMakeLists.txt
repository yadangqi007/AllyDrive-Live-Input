cmake_minimum_required(VERSION 3.20)
project(AllyDriveLiveInput VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(allydrive_xinput SHARED xinput_proxy.cpp xinput9_1_0.def)
set_target_properties(allydrive_xinput PROPERTIES
    OUTPUT_NAME "XINPUT9_1_0"
    PREFIX ""
)

add_executable(filter_tests filter_tests.cpp)
add_executable(proxy_smoke_test proxy_smoke_test.cpp)

enable_testing()
add_test(NAME filter_core COMMAND filter_tests)
add_test(NAME proxy_smoke COMMAND proxy_smoke_test $<TARGET_FILE:allydrive_xinput>)

if(DEFINED ENV{AMS2_ROOT})
    add_executable(offset_tests offset_tests.cpp)
    target_compile_definitions(offset_tests PRIVATE
        AMS2_SHARED_MEMORY_HEADER="$ENV{AMS2_ROOT}/Support/SharedMemory/AMS2_SharedMemoryExampleApp/SharedMemory.h"
    )
    add_test(NAME shared_memory_offsets COMMAND offset_tests)
endif()
