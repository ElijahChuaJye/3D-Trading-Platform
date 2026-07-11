# Discover application level sources (like your Main entry file)
file(GLOB_RECURSE APP_SOURCES "source/*.cpp")

# Establish the master executable application target binary 
add_executable(3DTradingPlatform ${APP_SOURCES})

# ==============================================================================
# LINK ENGINE MODULE - This brings in all engine functions and header loops automatically!
# ==============================================================================
target_link_libraries(3DTradingPlatform PRIVATE Engine)

# ==============================================================================
# AUTOMATED GLSL -> SPIR-V COMPILATION PASS (Moved directly to App space)
# ==============================================================================
find_program(GLSLC_EXECUTABLE glslc HINTS "ENV VULKAN_SDK" PATH_SUFFIXES "bin")

if(GLSLC_EXECUTABLE)
    file(GLOB SHADER_SOURCES 
        "shaders/*.vert"
        "shaders/*.frag"
    )
    
    foreach(SHADER ${SHADER_SOURCES})
        get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
        
        # Direct compilation output right to the application's runtime folder context
        set(SPV_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/SPV/${SHADER_NAME}.spv")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SPV")
        
        add_custom_command(
            OUTPUT ${SPV_OUTPUT}
            COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPV_OUTPUT}
            DEPENDS ${SHADER}
            COMMENT "Automated Build: Compiling ${SHADER_NAME} -> ${SHADER_NAME}.spv"
        )
        list(APPEND SPV_SHADERS ${SPV_OUTPUT})
    endforeach()
    
    add_custom_target(CompileShaders DEPENDS ${SPV_SHADERS})
    set_target_properties(CompileShaders PROPERTIES FOLDER "CMakeTargets")
    add_dependencies(3DTradingPlatform CompileShaders)
else()
    message(FATAL_ERROR "Vulkan SDK 'glslc' compiler not found!")
endif()

# Force IDE structure to match the raw shaders directory layout path
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" FILES ${SHADER_SOURCES})