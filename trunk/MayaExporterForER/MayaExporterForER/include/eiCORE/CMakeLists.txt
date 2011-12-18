# Win32 API for Windows and pthread library for Linux.
#
find_package(Threads REQUIRED)

if(MSVC)
	# User have to locate the path to the ws2_32.lib manually, its name is same in x86 and x64.
	#
	if(CMAKE_CL_64)
		find_library(WS "ws2_32" PATH_SUFFIXES "x64" DOC "Winsock2 Library" REQUIRED)
	else()
		find_library(WS "ws2_32" DOC "Winsock2 Library" REQUIRED)
	endif()
elseif(UNIX)
	# libdl is required to load DSO.
	#
	find_library(LIBDL "dl" REQUIRED)
endif()

# Grab the all header and source files.
#
file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.c")

include_directories("${ER_INCLUDE_DIR}")

add_definitions(-DeiCORE_EXPORTS)

add_library(eiCORE SHARED ${HEADERS} ${SOURCES})

if(MSVC)
	target_link_libraries(eiCORE ${WS})
elseif(UNIX)
	target_link_libraries(eiCORE ${LIBDL})
endif()

if(ER_REQUIRE_UNIT_TEST)
	add_subdirectory(UnitTests)
endif()
