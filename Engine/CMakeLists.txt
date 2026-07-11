# ==============================================================================
# DISCOVER ALL ENGINE GRAPHICS MODULES
# ==============================================================================
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "source/*.cpp"
    "source/**/*.cpp"
    "extern/*.c" # Volk initialization code
)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    "include/*.h"
    "include/**/*.h"
)

# Compile into a STATIC Library asset instead of an executable target
add_library(Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Organize folder structures cleanly inside the IDE sidebar view
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Export internal and external lookup include paths for anyone linking to Engine
target_include_directories(Engine PUBLIC
    include
    extern
    extern/vulkan
    ${Vulkan_INCLUDE_DIRS}
)

# Connect low-level API dependencies directly to the Engine framework binary
target_link_libraries(Engine PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
    ${WINSOCK_LIB}
)

if(MSVC)
    target_compile_options(Engine PRIVATE /W4 /WX)
else()
    target_compile_options(Engine PRIVATE -Wall -Wextra -Werror)
endif()