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

find_package(vamos-buffers REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
        if (vamos_buffers_BUILD_TYPE)
	        message(STATUS "Build type not set, using the same as vamos-buffers.")
	        set(CMAKE_BUILD_TYPE ${vamos_buffers_BUILD_TYPE} CACHE STRING "" FORCE)
        else()
	        message(STATUS "Build type not set. Setting default RelWithDebInfo.")
	        set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
        endif()
endif()

if (NOT ${vamos_buffers_BUILD_TYPE} STREQUAL ${CMAKE_BUILD_TYPE})
        message(WARNING "Build type (${CMAKE_BUILD_TYPE}) is different from "
                        "vamos-buffers' build type (${vamos_buffers_BUILD_TYPE}). "
                        "That can cause troubles with linking if IPO is enabled at "
                        "vamos-buffers and not in this project "
                        "(vamos_buffers_IPO=${vamos_buffers_OPTION_ENABLE_IPO})")
endif()

OPTION(ENABLE_IPO "Enable interprocedural optimizations" ${vamos_buffers_IPO})

if (ENABLE_IPO)
if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	cmake_policy(SET CMP0069 NEW)
	include(CheckIPOSupported)
	check_ipo_supported()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
endif(ENABLE_IPO)





add_subdirectory(mpt-monitors)

#add_custom_target(gen-readme
#                  COMMAND pandoc -f markdown -t HTML -o README.html README.md
#                  COMMAND pandoc -f markdown -t pdf -o README.pdf README.md)
