cmake_minimum_required(VERSION 3.25)
project(TradingPlatform3D)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find System Vulkan SDK dependencies
find_package(Vulkan REQUIRED)

if(WIN32)
    set(WINSOCK_LIB ws2_32)
endif()

# ==============================================================================
# 1. FETCH DEPENDENCIES FIRST (This ensures glm::glm target is created)
# ==============================================================================
include("Dependencies/FetchDependencies.cmake")

# ==============================================================================
# 2. THEN ADD SUBDIRECTORIES (Now Engine can safely find glm::glm)
# ==============================================================================
add_subdirectory(Engine)
add_subdirectory(Application/3DTradingPlatform)