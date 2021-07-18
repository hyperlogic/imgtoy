# cmake .. -A x64 -DCMAKE_TOOLCHAIN_FILE=${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(imgtoy LANGUAGES CXX)

set(PROJECT_NAME imgtoy)

find_package(OpenGL REQUIRED)
include_directories(${GL_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)

get_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.cpp src/image.cpp src/log.cpp src/texture.cpp src/program.cpp src/util.cpp)

if(WIN32)
    # set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)

	set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
	find_package(libpng CONFIG REQUIRED)
    set(PNG_LIBRARIES png)
	set(GLEW_LIBRARIES GLEW::GLEW)
else()
    find_package(PNG REQUIRED)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${PNG_LIBRARIES} ${SDL2_LIBRARIES})


