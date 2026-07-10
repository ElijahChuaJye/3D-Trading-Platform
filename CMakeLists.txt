# Sets minimum version of cmake first
# 3.25 is minimum modern baseline for it.
cmake_minimum_required(VERSION 3.25)

# Names my project
project(TradingPlatform3D)

# Tells cmake what standard to use.
set(CMAKE_CXX_STANDARD 20)

# Forces build to fail if compiler does not support C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Search my computer for Vulkan SDK
# ==============================================================================
find_package(Vulkan REQUIRED)

if(WIN32)
    # If true, sets a variable called WINSOCK_LIB
    # Real name for Windows Networking Library
    set(WINSOCK_LIB ws2_32)
endif()

# ==============================================================================
# Dependencies
# ==============================================================================
include("Dependencies/FetchDependencies.cmake")

# ==============================================================================
# Automated File Discovery
# ==============================================================================
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "Project/source/*.cpp"      # Finds all C++ files in source
    "Project/source/**/*.cpp"   # Finds all C++ files in source subfolders
    "Project/extern/*.c"        # Finds all C files in extern (like volk.c)
    "Project/extern/*.cpp"      # In case you add C++ libraries later
)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS
    "Project/include/*.h"       # Finds all Header files in include
    "Project/include/**/*.h"     # Finds all header files in subfolders
    "Project/extern/*.h"        # Finds all headers in extern
)

message(STATUS "---Found Files---")
foreach(FILE IN LISTS SOURCE_FILES)
    message(STATUS "${FILE}")
endforeach()

foreach(HEADER IN LISTS HEADER_FILES)
    message(STATUS "${HEADER}")
endforeach()

# ==============================================================================
# AUTOMATED GLSL -> SPIR-V COMPILATION PASS
# ==============================================================================
# 1. Look up the glslc compiler in the Vulkan SDK installation path
find_program(GLSLC_EXECUTABLE glslc HINTS "ENV VULKAN_SDK" PATH_SUFFIXES "bin")

if(GLSLC_EXECUTABLE)
    message(STATUS "Vulkan GLSL Compiler located: ${GLSLC_EXECUTABLE}")
    
    #Looking for .vert and .glsl files
    file(GLOB SHADER_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/Project/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/Project/shaders/*.frag"
    )
    
    foreach(SHADER ${SHADER_SOURCES})
        # Extract file name without extension (e.g., "vert")
        get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
        
	# 1. CHANGE THIS LINE: Change /shaders/ to /SPV/
        set(SPV_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/SPV/${SHADER_NAME}.spv")
        
        # 2. CHANGE THIS LINE: Update the directory creation path to SPV
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SPV")
        
        # 4. Bind the glslc system command to run during a standard project build
        add_custom_command(
            OUTPUT ${SPV_OUTPUT}
            COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPV_OUTPUT}
            DEPENDS ${SHADER}
            COMMENT "Automated Build: Compiling ${SHADER_NAME} -> ${SHADER_NAME}.spv"
        )
        list(APPEND SPV_SHADERS ${SPV_OUTPUT})
    endforeach()
    
    # 5. Package these operations into a unified target dependency block
    add_custom_target(CompileShaders DEPENDS ${SPV_SHADERS})
else()
    message(FATAL_ERROR "Vulkan SDK 'glslc' compiler not found! Make sure your Vulkan SDK is fully installed.")
endif()

# ==============================================================================
# Visual Studio Folder Mirroring
# ==============================================================================
# This forces Visual Studio to display the exact physical folder structure
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Project" FILES ${SOURCE_FILES} ${HEADER_FILES})

# ==============================================================================
# Building executable
# ==============================================================================
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# === LINK SHADER ENGINE COMPILATION AS A DIRECT BUILD REQUIREMENT ===
add_dependencies(${PROJECT_NAME} CompileShaders)

if(MSVC)
    #/W4 = Warning level 4
    #/WX = Treat all warnings as errors
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    # If I ever move this engine to Linux
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif()

# For linking
# Tells your executable where to look when you type #include "something.h" in your code.
target_include_directories(${PROJECT_NAME} PRIVATE
    Project/source
    Project/extern
    Project/include
    ${Vulkan_INCLUDE_DIRS}
)

# Links the pre-compiled machine code (the actual libraries) to your executable.
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw
    ${WINSOCK_LIB}
)