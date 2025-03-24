cmake_minimum_required(VERSION 3.0)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add executable
add_executable(wiimotebridged 
	wiimotebridged.c
	osc.c
)

# Link libraries
target_link_libraries(wiimotebridged
	wiiuse
	m  # For math functions
)

if(INSTALL_EXAMPLES)
	install(TARGETS wiimotebridged
		RUNTIME DESTINATION bin COMPONENT examples)
endif()