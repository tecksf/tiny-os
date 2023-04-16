cmake_minimum_required(VERSION 3.24)
project(foots_stone C)

enable_language(ASM C)

set(CMAKE_C_STANDARD 11)

#add_executable(sign tools/sign.c)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)


include_directories(
    src/base
    src/utils
    src/libs
    src/kernel/debug
    src/kernel/memory
    src/kernel/driver
    src/kernel/process
    src/kernel/interrupt
    src/kernel/filesystem
    src/kernel/core
)

# ========= bootloader =========
set(DEFAULT_FLAGS -g -Wall -Os -nostdlib -nostdinc -m32 -fno-builtin)

add_executable(boot_block src/bootloader/boot_asm.S src/bootloader/boot_main.c)
target_compile_options(boot_block PUBLIC ${DEFAULT_FLAGS})
target_link_options(boot_block PUBLIC -Wl,-T ${CMAKE_SOURCE_DIR}/tools/boot.ld -m32 -nostdlib -no-pie -Wl,--build-id=none)

add_custom_target(write_boot_block
    COMMAND objcopy -S -O binary ${EXECUTABLE_OUTPUT_PATH}/boot_block ${EXECUTABLE_OUTPUT_PATH}/boot_block.bin
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/sign ${EXECUTABLE_OUTPUT_PATH}/boot_block.bin ${EXECUTABLE_OUTPUT_PATH}/boot_block_bin
    DEPENDS boot_block)
# ========= bootloader =========

set(DISABLE_SIMD -mno-mmx -mno-sse)
set(CFLAGS -g -Wall -O0 -nostdlib -m32 -no-pie -fno-builtin -nostdinc -fno-stack-protector ${DISABLE_SIMD})
set(LD_FLAGS -m32 -nostdlib -Wl,--trace)

file(GLOB COMMON_LIBS src/libs/*.c)
file(GLOB UTILS_SRC src/utils/*.c)

# ========= kernel =========

file(GLOB_RECURSE KERNEL_SRC src/kernel/*.c)
set(INTER_SRC src/kernel/interrupt/trap_entry.S src/kernel/interrupt/vectors.S)
set(PROC_SRC src/kernel/process/switch.S src/kernel/process/process_entry.S)
add_executable(kernel ${KERNEL_SRC} ${INTER_SRC} ${PROC_SRC} ${UTILS_SRC} ${COMMON_LIBS} src/kernel/entry.S)
target_compile_options(kernel PUBLIC ${CFLAGS})
target_link_options(kernel PUBLIC ${LD_FLAGS} -Wl,-T ${CMAKE_SOURCE_DIR}/tools/kernel.ld)

# ========= kernel =========

set(KERNEL_IMAGE kernel.img)
add_custom_target(create_kernel_image
    COMMAND dd if=/dev/zero of=${KERNEL_IMAGE} count=10000
    COMMAND dd if=${EXECUTABLE_OUTPUT_PATH}/boot_block_bin of=${KERNEL_IMAGE} conv=notrunc
    COMMAND dd if=${EXECUTABLE_OUTPUT_PATH}/kernel of=${KERNEL_IMAGE} seek=1 conv=notrunc
    DEPENDS write_boot_block kernel)


set(SWAP_IMAGE swap.img)
set(USER_IMAGE user.img)
set(TEMP_IMAGE temp.img)
add_custom_command(TARGET create_kernel_image
    POST_BUILD
    COMMAND dd if=/dev/zero of=${SWAP_IMAGE} count=10000
    COMMAND dd if=/dev/zero of=${USER_IMAGE} count=10000
    COMMAND dd if=/dev/zero of=${TEMP_IMAGE} count=10000
    VERBATIM)


set(START_SECTION 0)
list(APPEND APPS)
file(GLOB USER_LIBS src/user/libs/*.c)
file(GLOB USER_SRC src/user/*.c)


set(USER_CFLAGS -Wall -Os -s -nostdlib -m32 -fno-builtin -fno-stack-protector -nostdinc ${DISABLE_SIMD})
set(USER_LD_FLAGS -m32 -nostdlib -no-pie -Wl,--build-id=none)
foreach (F ${USER_SRC})
  get_filename_component(APP_NAME ${F} NAME_WE)
  add_executable(${APP_NAME} ${F} ${USER_LIBS} ${COMMON_LIBS} src/user/libs/init_main.S)
  target_include_directories(${APP_NAME} PUBLIC src/libs src/user/libs)
  target_compile_options(${APP_NAME} PUBLIC ${USER_CFLAGS})
  target_link_options(${APP_NAME} PUBLIC ${USER_LD_FLAGS} -Wl,-T ${CMAKE_SOURCE_DIR}/tools/user.ld)

  list(APPEND APPS ${APP_NAME})
  add_custom_command(TARGET create_kernel_image
      POST_BUILD
      COMMAND dd if=${EXECUTABLE_OUTPUT_PATH}/${APP_NAME} of=${USER_IMAGE} seek=${START_SECTION} conv=notrunc
      VERBATIM)
  math(EXPR START_SECTION "${START_SECTION}+8")
endforeach ()

add_dependencies(kernel ${APPS})
