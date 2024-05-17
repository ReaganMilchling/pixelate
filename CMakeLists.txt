cmake_minimum_required(VERSION 3.27)
project(pixelate)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE pixelate src/*)
#file(GLOB_RECURSE stb third_party/stb/*)
add_executable(
    pixelate
    ${pixelate}
    #third_party/stb/stb_image_write.h
    #third_party/stb/stb_image.h
    #third_party/stb/stb_image_resize2.h
)

#include_directories(third_party/stb/)

target_link_libraries(pixelate PRIVATE)
