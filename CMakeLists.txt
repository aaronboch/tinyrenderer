cmake_minimum_required(VERSION 3.12...3.26)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Release default")
endif()

project(tinyrenderer LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(iwyu "Run include-what-you-use")
if(iwyu)
  find_program(IWYU_EXE NAMES include-what-you-use REQUIRED)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_EXE})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|Intel")
  add_compile_options(-Wall)
endif()

find_package(OpenMP COMPONENTS CXX)
find_package(raylib REQUIRED)

set(SOURCES main.cpp model.cpp our_gl.cpp camera.cpp)

set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
)

set(RLIMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/rlImGui/rlImGui.cpp
)

set(TINYFD_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/tinyfd/tinyfiledialogs.cpp
)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
list(TRANSFORM SOURCES PREPEND "${SRC_DIR}/")

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${RLIMGUI_SOURCES} ${TINYFD_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/third_party/rlImGui
    ${CMAKE_SOURCE_DIR}/third_party/tinyfd
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib $<$<BOOL:${OpenMP_CXX_FOUND}>:OpenMP::OpenMP_CXX>)
file(GENERATE OUTPUT .gitignore CONTENT "*")
