cmake_minimum_required( VERSION 3.0 )

project( tests )

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COMPILE_FLAGS}" )

include_directories( ${PROJECT_SOURCE_DIR}/../src )
add_executable( test_runner 
	${PROJECT_SOURCE_DIR}/../src/main/eval.c 
	${PROJECT_SOURCE_DIR}/../src/main/bitmagic 
	${PROJECT_SOURCE_DIR}/../.test/tests.c
	)

target_compile_options( test_runner PUBLIC -g -fdiagnostics-color=always -fsanitize=address -fsanitize=leak -fsanitize=undefined -fno-sanitize-recover=all -fstack-protector-all -fstack-check -Wall -Wpedantic -Wextra  -Wcast-align -Wcast-qual -Wvla -Wshadow -Wsuggest-attribute=const -Wmissing-format-attribute -Wuninitialized -Winit-self  -Wdouble-promotion -Wno-unused-local-typedefs -O0 -isystem src/cmsis -I src -isystem src/mcu_support_package/inc -std=c99 -DSTM32F103xB -DUSE_FULL_ASSERT -DSTM32F10X_MD -DHSE_VALUE="8000000" )

target_link_libraries( test_runner PUBLIC -lm -fsanitize=address -fsanitize=leak -fsanitize=undefined )