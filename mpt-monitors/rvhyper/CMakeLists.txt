add_executable(monitor-rvhyper ../src/main_argv.cpp ../monitor_inputs_rvhyper.cpp)
target_include_directories(monitor-rvhyper PUBLIC ${vamos-buffers_INCLUDE_DIRS})
target_link_libraries(monitor-rvhyper PRIVATE monitor-od)

add_executable(monitor-rvhyper-sync ../src/main_argv.cpp ../monitor_inputs_rvhyper_sync.cpp)
target_include_directories(monitor-rvhyper-sync PUBLIC ${vamos-buffers_INCLUDE_DIRS})
target_link_libraries(monitor-rvhyper-sync PRIVATE monitor-od)
target_compile_definitions(monitor-rvhyper-sync PRIVATE ASSUME_SYNC_TRACES=1)

