#target_compile_options(test2 PRIVATE -fsanitize=address,undefined)

add_library(monitor-gps-od STATIC monitor.cpp private.cpp cfgs.cpp)
target_include_directories(monitor-gps-od PUBLIC ${vamos-buffers_INCLUDE_DIRS})
target_link_libraries(monitor-gps-od PUBLIC vamos-hyper vamos-buffers-event)

add_library(monitor-gps-od-dbg STATIC monitor.cpp private.cpp cfgs.cpp)
target_include_directories(monitor-gps-od-dbg PUBLIC ${vamos-buffers_INCLUDE_DIRS})
target_link_libraries(monitor-gps-od-dbg PUBLIC vamos-hyper vamos-buffers-event)
target_compile_definitions(monitor-gps-od-dbg
			   PRIVATE EXIT_ON_ERROR=1
			   PRIVATE OUTPUT=1)

add_executable(monitor-gps monitor_msr_gps_main.cpp monitor_inputs_msr_gps.cpp)
target_include_directories(monitor-gps PUBLIC ${vamos-buffers_INCLUDE_DIRS})
target_link_libraries(monitor-gps PRIVATE monitor-gps-od)
target_compile_definitions(monitor-gps
                           #PRIVATE RAND_SEED=0xdeed
                           PRIVATE NUM_STREAMS=2
                           PRIVATE STREAM_LEN=10
                           PRIVATE REDUCT_SYMMETRY=1
                           PRIVATE REDUCT_REFLEXIVITY=1
                           )


