file(GLOB HEADERS "*.h")
file(GLOB C_SOURCES "*.c")
file(GLOB CPP_SOURCES "*.cpp")

add_definitions(-DeiAPI_EXPORTS)

include_directories("${ER_INCLUDE_DIR}")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
BISON_TARGET(eiParser ei_parser.yy "${CMAKE_CURRENT_BINARY_DIR}/ei_parser.yy.cpp")
 FLEX_TARGET(eiLexer  ei_parser.ll "${CMAKE_CURRENT_BINARY_DIR}/parser.ll.cpp")
ADD_FLEX_BISON_DEPENDENCY(eiLexer eiParser)

add_library(eiAPI SHARED ${HEADERS} ${C_SOURCES} ${CPP_SOURCES} ${BISON_eiParser_OUTPUTS} ${FLEX_eiLexer_OUTPUTS} ei_parser.ll ei_parser.yy)
target_link_libraries(eiAPI eiCORE)

add_executable(er er.c)
target_link_libraries(er eiAPI)

if(ER_REQUIRE_UNIT_TEST)
	add_subdirectory(UnitTests)
endif()
