cmake_minimum_required(VERSION 3.0)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add executable
add_executable(wiimtebridged 
	wiimtebridged.c
	osc.c
)

# Link libraries
target_link_libraries(wiimtebridged
	wiiuse
	m  # For math functions
)

if(INSTALL_EXAMPLES)
	install(TARGETS wiimtebridged
		RUNTIME DESTINATION bin COMPONENT examples)
endif()