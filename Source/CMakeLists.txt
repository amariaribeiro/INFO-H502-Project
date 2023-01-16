project("SOURCE")

#These commands are there to specify the path to the folder containing the object and textures files as macro
#With these you can just use PATH_TO_OBJECTS and PATH_TO_TEXTURE in your c++ code and the compiler will replace it by the correct expression
add_compile_definitions(PATH_TO_OBJECTS="${CMAKE_CURRENT_SOURCE_DIR}/objects")
add_compile_definitions(PATH_TO_TEXTURE="${CMAKE_CURRENT_SOURCE_DIR}/textures")

if(COMPILE_SOLUTION)

set(MAIN "main.cpp")

add_executable(${PROJECT_NAME}_main ${MAIN} "main.cpp" "object.h" "shader.h" "camera.h")
target_link_libraries(${PROJECT_NAME}_main PUBLIC OpenGL::GL glfw glad)

endif()