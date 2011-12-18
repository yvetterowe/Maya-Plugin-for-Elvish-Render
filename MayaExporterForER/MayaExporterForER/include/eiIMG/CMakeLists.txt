file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.c")

include_directories("${ER_INCLUDE_DIR}")

add_library(eiIMG SHARED ${HEADERS} ${SOURCES} rgbe.txt)
target_link_libraries(eiIMG eiCORE)
target_link_libraries(eiIMG eiAPI)

if(ER_REQUIRE_UNIT_TEST)
	add_subdirectory(UnitTests)
endif()
