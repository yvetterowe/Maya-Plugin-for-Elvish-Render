file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.c")
file(GLOB SOURCES "*.cpp")

include_directories("${ER_INCLUDE_DIR}")

add_library(eiSHADER SHARED ${HEADERS} ${SOURCES})
target_link_libraries(eiSHADER eiCORE)
target_link_libraries(eiSHADER eiAPI)

if(ER_REQUIRE_UNIT_TEST)
	add_subdirectory(UnitTests)
endif()
