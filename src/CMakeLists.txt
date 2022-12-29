# if(UNIX)
#     set(OpenGL_GL_PREFERENCE "GLVND")
# endif(UNIX)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS} PRIVATE)

# Include GLFW and GLM
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Add parser dependency
add_subdirectory("${EXTERNAL_FOLDER}/parser" parser)

include_directories("${EXTERNAL_FOLDER}/parser" PRIVATE)

include_directories("${MAIN_FOLDER}/private" PRIVATE)
include_directories("${MAIN_FOLDER}/public" PUBLIC)

file(GLOB_RECURSE SRC_FILES
    "${MAIN_FOLDER}/private/*.cpp"
)

add_executable(VulkanRenderer ${SRC_FILES})
target_link_libraries(VulkanRenderer ${Vulkan_LIBRARIES} glfw glm parser)
set_target_properties(VulkanRenderer PROPERTIES FOLDER ${MAIN_FOLDER})
