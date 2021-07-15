cmake_minimum_required(VERSION 3.17)
project(TP1)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS -DGLEW_STATIC )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
find_package(assimp REQUIRED)

find_package(GLM REQUIRED)
add_executable(TP1
        main.cpp
        matrix4.cpp
        Vector.cpp
        program.cpp
        Mesh.cpp
        Model.cpp
        Camera.cpp
        examples/gouraud.cpp
        examples/texture.cpp
        examples/phong.cpp
        examples/coordinates.cpp
        examples/camera.cpp
        examples/colors.cpp
        examples/light_map.cpp
        examples/multiples_lights.cpp
        examples/backpack.cpp
        examples/link.cpp
        examples/nordic_village.cpp
        examples/cavern.cpp
        examples/isometric_world.cpp
        examples/isometric_world.cpp
        examples/windfall.cpp
        examples/skybox.cpp
        imgui-1.83/imgui.cpp
        imgui-1.83/imgui_demo.cpp
        imgui-1.83/imgui_draw.cpp
        imgui-1.83/imgui_tables.cpp
        imgui-1.83/imgui_widgets.cpp
        imgui-1.83/backends/imgui_impl_glfw.cpp
        imgui-1.83/backends/imgui_impl_opengl3.cpp
        examples/shadow.cpp
        Shadow.cpp
        examples/house_of_wealth.cpp
        callback.cpp
        Helper.cpp
        Skybox.cpp
        Display_template.cpp
        misc.cpp
        windfall_objects.cpp)

add_executable(imgui_demo
        imgui_example.cpp
        imgui-1.83/imgui.cpp
        imgui-1.83/imgui_demo.cpp
        imgui-1.83/imgui_draw.cpp
        imgui-1.83/imgui_tables.cpp
        imgui-1.83/imgui_widgets.cpp
        imgui-1.83/backends/imgui_impl_glfw.cpp
        imgui-1.83/backends/imgui_impl_opengl3.cpp)

target_link_libraries(TP1 -lGLEW -lGL glfw -lX11 -lpthread -lXrandr -ldl -lassimp)
target_link_libraries(imgui_demo -lGLEW -lGL glfw -lX11 -lpthread -lXrandr -ldl -lassimp)

include_directories(imgui-1.83 imgui-1.83/backends)

target_compile_options(TP1 PRIVATE -Wextra -Wall)
