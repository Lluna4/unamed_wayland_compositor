cmake_minimum_required(VERSION 3.10.0)
project(lunarland VERSION 0.0.1 LANGUAGES C CXX)

link_libraries(wayland-server)
link_libraries(drm)
add_executable(lunarland src/main.cpp src/include/compositor.cpp src/include/backend.cpp)

