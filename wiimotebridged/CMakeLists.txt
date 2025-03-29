cmake_minimum_required(VERSION 3.0)

# Find Avahi
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVAHI REQUIRED avahi-client)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add executable
add_executable(wiimotebridged 
	wiimotebridged.c
	osc.c
)

target_include_directories(wiimotebridged PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${AVAHI_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(wiimotebridged
	wiiuse
	m  # For math functions
	${AVAHI_LIBRARIES}
)

if(INSTALL_EXAMPLES)
	install(TARGETS wiimotebridged
		RUNTIME DESTINATION bin COMPONENT examples)
endif()