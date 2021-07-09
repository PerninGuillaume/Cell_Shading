cmake_minimum_required(VERSION 3.17)
project(TP1)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS -DGLEW_STATIC )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
find_package(assimp REQUIRED)

find_package(GLM REQUIRED)
add_executable(TP1 main.cpp matrix4.cpp matrix4.h main.h Vector.cpp Vector3.h program.cpp program.h examples/gouraud.cpp examples/gouraud.h examples/texture.cpp examples/texture.h examples/phong.cpp examples/phong.h examples/coordinates.cpp examples/camera.cpp Camera.cpp Camera.h examples/colors.cpp examples/colors.h examples/light_map.cpp examples/light_map.h examples/multiples_lights.cpp examples/multiples_lights.h Mesh.cpp Mesh.h Model.cpp Model.h examples/backpack.cpp examples/backpack.h examples/link.cpp examples/link.h examples/nordic_village.cpp examples/nordic_village.h)

target_link_libraries(TP1 -lGLEW -lGL glfw -lX11 -lpthread -lXrandr -ldl -lassimp)

target_compile_options(TP1 PRIVATE -Wextra -Wall)
