cmake_minimum_required(VERSION 3.24)
project(VulkanTest)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
#Matrix Math
find_package(glm REQUIRED)
#Window System
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
#3D Renderer
find_package(Vulkan REQUIRED)
#Shader Compiler
find_program(glslc_executable NAMES glslc PATHS /Scratch/Vulkan/install/bin)

#==============================================================================
# COMPILE SHADERS
#

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

file(GLOB SHADERS
        ${SHADER_SOURCE_DIR}/*.vert
        ${SHADER_SOURCE_DIR}/*.frag
        ${SHADER_SOURCE_DIR}/*.comp
        ${SHADER_SOURCE_DIR}/*.geom
        ${SHADER_SOURCE_DIR}/*.tesc
        ${SHADER_SOURCE_DIR}/*.tese
        ${SHADER_SOURCE_DIR}/*.mesh
        ${SHADER_SOURCE_DIR}/*.task
        ${SHADER_SOURCE_DIR}/*.rgen
        ${SHADER_SOURCE_DIR}/*.rchit
        ${SHADER_SOURCE_DIR}/*.rmiss)

foreach(source IN LISTS SHADERS)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
            COMMAND
            ${glslc_executable}
            -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
            ${source}
            OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
            DEPENDS ${source} ${SHADER_BINARY_DIR}
            COMMENT "Compiling ${FILENAME}"
    )
    list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
set(MY_SHADERS ${SPV_SHADERS} lve_game_object.hpp simple_render_system.cpp simple_render_system.cpp)

### END SHADER COMPILATION
file(GLOB CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
#Note we don’t need to bother with any of the .h or .hpp files.
#set(LVE_INCLUDES main.cpp lve_window.hpp lve_window.cpp lve_model.cpp lve_swap_chain.cpp first_app.hpp first_app.cpp lve_pipeline.hpp lve_pipeline.cpp lve_device.hpp lve_device.cpp lve_swap_chain.cpp)
#At some point he will add a systems directory to break up
#the organization of the files, that will add new complexity to this
#file. Luckily not much complexity.
add_executable(VulkanTest ${CPP_FILES} ${MY_SHADERS})
target_sources(VulkanTest PRIVATE main.cpp)
#We don't use the glm libraries till after video 8.
target_link_libraries(VulkanTest PRIVATE Vulkan::Vulkan glm::glm ${GLFW_LIBRARIES})