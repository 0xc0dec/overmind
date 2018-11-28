cmake_minimum_required(VERSION 2.8.4)
project(Overmind)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Config types" FORCE)

add_compile_options(-std=c++14)

if (MSVC)
    add_compile_options("$<$<CONFIG:DEBUG>:/MTd>")
    add_compile_options("$<$<CONFIG:RELEASE>:/MT>")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

file(GLOB SOURCES "src/*.cpp" "src/*.h")

add_executable(Overmind ${SOURCES})

target_include_directories(Overmind PRIVATE
    "vendor/httplib"
)

target_link_libraries(Overmind
    ws2_32.lib
    winmm.lib
    imm32.lib
    version.lib
)

set_target_properties(Overmind PROPERTIES OUTPUT_NAME "ovm")