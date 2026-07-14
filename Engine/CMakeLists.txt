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
    ${vma_SOURCE_DIR}/include
)

# Connect low-level API dependencies directly to the Engine framework binary
target_link_libraries(Engine PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
    ${WINSOCK_LIB}
)


if(MSVC)
   # WHY: We add /wd4100 (Disable unused parameter warnings) and 
    # /wd4324 (Disable structure padding warnings) to prevent third-party 
    # headers like VMA from triggering compilation failures under our strict /WX flag.
    target_compile_options(Engine PRIVATE /W4 /WX /wd4100 /wd4324)
else()
    # WHY: On GCC/Clang, we similarly tell the compiler to ignore unused parameter 
    # warnings inside the Engine so that VMA doesn't halt the build.
    target_compile_options(Engine PRIVATE -Wall -Wextra -Werror -Wno-unused-parameter)
endif()