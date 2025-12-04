cmake_minimum_required(VERSION 3.0)
project(ali_sdk C)


option(LOAD_TO_RAM "Load firmware to RAM using UART" OFF)
if(LOAD_TO_RAM)
    set(LINKER_SCRIPT "ld/ali_m3801_ram.ld")
else()
    set(LINKER_SCRIPT "ld/ali_m3801.ld")
endif(LOAD_TO_RAM)
unset(LOAD_TO_RAM CACHE)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR mips32)
set(CMAKE_CROSSCOMPILING 1)


set(CMAKE_C_COMPILER        mipsel-unknown-elf-gcc)
set(CMAKE_CXX_COMPILER      mipsel-unknown-elf-g++)
set(CMAKE_LINKER            mipsel-unknown-elf-ld)
set(CMAKE_OBJCOPY           mipsel-unknown-elf-objcopy)

set(CMAKE_C_FLAGS "-mips32r5 -mdouble-float -mno-gpopt -mno-mips16 -Wall -Wformat -nostdlib")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-mips32r5 -mdouble-float -mno-mips16 -Wl,-Map=output.map -EL ${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT}")

# hack to compile assembly with gcc
set(ASM_SRCS sdk/src/start.S)
set_source_files_properties(${ASM_SRCS} PROPERTIES LANGUAGE C)
list(APPEND CMAKE_C_SOURCE_FILE_EXTENSIONS S)

include_directories("sdk/include" "user/include")

add_library(syscalls STATIC sdk/src/syscalls.c)

add_executable(${PROJECT_NAME}
    sdk/src/start.S
    sdk/src/print.c
    sdk/src/uart.c
    sdk/src/gpio.c
    user/src/main.c
    sdk/src/entry.c
    sdk/src/fpu.c
)
target_link_libraries(${PROJECT_NAME} m c gcc syscalls)

# make binary from elf
add_custom_command(
    TARGET ${PROJECT_NAME}
    BYPRODUCTS ${PROJECT_NAME}.bin ${PROJECT_NAME}_padded.bin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${PROJECT_NAME} ${PROJECT_NAME}.bin
    COMMAND cat ${PROJECT_NAME}.bin /dev/zero | dd bs=16 count=262144 of=${PROJECT_NAME}_padded.bin
)

# flashing spi flash
add_custom_target(flash
    COMMAND sudo flashrom -p ch341a_spi -w ${PROJECT_NAME}_padded.bin
)
