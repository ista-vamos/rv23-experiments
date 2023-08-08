#target_compile_options(test2 PRIVATE -fsanitize=address,undefined)

set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(vamos-buffers REQUIRED)
include_directories(${vamos_buffers_INCLUDE_DIRS})

add_subdirectory(src)
#add_subdirectory(src-gps)
add_subdirectory(bin)
add_subdirectory(rvhyper)

enable_testing()
add_subdirectory(tests)

OPTION(ENABLE_MULTIPLE_MOVES "Let MPEs move as far as they can in each `step`" OFF)
