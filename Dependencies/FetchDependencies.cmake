# Include the built-in FetchContent module
include(FetchContent)

# ---------------------------------------------------------
# 1. Fetch GLFW
# ---------------------------------------------------------
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# If you ever want to add ImGui, GLM, or other libraries later, 
# you just add their FetchContent blocks right below this