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

# ---------------------------------------------------------
# 2. Fetch GLM (With Modern Namespaced Target Alias Fix)
# ---------------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1                                      # ➔ FIXED: Swapped out the raw hash for a rock-solid release tag
)
FetchContent_MakeAvailable(glm)

# INTERCEPT THE DOWNLOAD HERE: If the modern namespaced target doesn't exist, create it!
if(TARGET glm AND NOT TARGET glm::glm)
    add_library(glm::glm ALIAS glm)
endif() 

if(TARGET glm)
    set_target_properties(glm PROPERTIES FOLDER "FetchContent")
endif()

# Catch the hidden dummy testing target GLM sometimes spins up behind the scenes
if(TARGET glm_dummy)
    set_target_properties(glm_dummy PROPERTIES FOLDER "FetchContent")
endif()