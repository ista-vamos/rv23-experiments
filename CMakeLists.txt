cmake_minimum_required(VERSION 3.9)
project(rv23-experiments)

enable_testing()

#FetchContent_Declare(
#        vamos-buffers
#        GIT_REPOSITORY https://github.com/ista-vamos/vamos-buffers.git
#        GIT_TAG rv23-experiments
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS TRUE)
#FetchContent_MakeAvailable(vamos-buffers)

if (NOT EXISTS vamos-buffers/CMakeLists.txt)
    execute_process(COMMAND git submodule update --init --recursive -- vamos-buffers
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY)
endif()

set(vamos-buffers_DIR ${PROJECT_SOURCE_DIR}/vamos-buffers/cmake/vamos-buffers)
add_subdirectory(mpt-monitors)

#add_custom_target(gen-readme
#                  COMMAND pandoc -f markdown -t HTML -o README.html README.md
#                  COMMAND pandoc -f markdown -t pdf -o README.pdf README.md)
