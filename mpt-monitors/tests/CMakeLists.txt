#target_compile_options(test2 PRIVATE -fsanitize=address,undefined)

set(MONITOR_TEST_MAIN main.cpp)

file(GLOB inputs "inputs*.cpp")
foreach(file ${inputs})
	string(REGEX MATCH "inputs-((.*)[0-9]+)\.cpp" _ ${file})
	message(STATUS "${file} ${CMAKE_MATCH_1} ${CMAKE_MATCH_2}")

	add_executable(test-${CMAKE_MATCH_1} ${MONITOR_TEST_MAIN} ${file})
	target_include_directories(test-${CMAKE_MATCH_1} PUBLIC ${vamos-buffers_INCLUDE_DIRS})
	target_include_directories(test-${CMAKE_MATCH_1} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
	target_link_libraries(test-${CMAKE_MATCH_1} PRIVATE monitor-od-dbg)

	add_test(NAME test-${CMAKE_MATCH_1}
		 COMMAND test-${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
endforeach()

