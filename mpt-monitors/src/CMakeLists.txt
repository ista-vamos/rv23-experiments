#target_compile_options(test2 PRIVATE -fsanitize=address,undefined)

# do not read more than this number of events at once
# if they are present on the input stream. If we have no
# limit, we could get stalled in reading events only on one trace until
# it is processed completely (which might, in theory, happen never).
# The algorithm described in the paper has this set to 1.
set(ONETIME_READ_EVENTS_LIMIT 128)

add_library(monitor-od STATIC monitor.cpp private.cpp cfgs.cpp)
target_include_directories(monitor-od PUBLIC ${vamos-buffers_INCLUDE_DIRS})
target_link_libraries(monitor-od PUBLIC vamos-hyper vamos-buffers-event)
target_compile_definitions(monitor-od
			   #PRIVATE EXIT_ON_ERROR=1
			   PRIVATE REDUCT_SYMMETRY=1
			   PRIVATE REDUCT_REFLEXIVITY=1
			   PRIVATE ONETIME_READ_EVENTS_LIMIT=${ONETIME_READ_EVENTS_LIMIT}
			   )


add_library(monitor-od-dbg STATIC monitor.cpp private.cpp cfgs.cpp)
target_include_directories(monitor-od-dbg PUBLIC ${vamos-buffers_INCLUDE_DIRS})
target_link_libraries(monitor-od-dbg PUBLIC vamos-hyper vamos-buffers-event)
target_compile_definitions(monitor-od-dbg
		           #PRIVATE EXIT_ON_ERROR=1
			   PRIVATE OUTPUT=1
			   PRIVATE DEBUG=1
			   PRIVATE DEBUG_EVENTS=1
			   PRIVATE DEBUG_CFGS=1
			   PRIVATE DEBUG_WORKBAG=1
			   PRIVATE REDUCT_SYMMETRY=1
			   PRIVATE REDUCT_REFLEXIVITY=1
			   PRIVATE ONETIME_READ_EVENTS_LIMIT=${ONETIME_READ_EVENTS_LIMIT}
			   )

if (ENABLE_MULTIPLE_MOVES)
	target_compile_definitions(monitor-od PRIVATE MULTIPLE_MOVES)
	target_compile_definitions(monitor-od-dbg PRIVATE MULTIPLE_MOVES)
endif()
