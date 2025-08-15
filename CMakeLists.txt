cmake_minimum_required(VERSION 3.15)
project(LTC6811_BMS C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build test executable" ON)

add_library(ltc6811 STATIC
	src/ltc6811.c
	src/ntc.c
	src/bms_master.c
)

target_include_directories(ltc6811 PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ltc6811 PUBLIC m)

if(BUILD_TESTS)
	add_executable(ltc6811_test
		tests/main.c
		src/mock_hal.c
	)
	target_include_directories(ltc6811_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_link_libraries(ltc6811_test PRIVATE ltc6811)
endif()