#target_compile_options(test2 PRIVATE -fsanitize=address,undefined)

add_subdirectory(src)
add_subdirectory(src-gps)
add_subdirectory(tests)
add_subdirectory(bin)
add_subdirectory(rvhyper)

OPTION(ENABLE_MULTIPLE_MOVES "Let MPEs move as far as they can in each `step`" OFF)
