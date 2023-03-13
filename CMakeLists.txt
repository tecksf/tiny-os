cmake_minimum_required(VERSION 3.24)
project(foots_stone C)

enable_language(ASM C)

set(CMAKE_C_STANDARD 99)

#add_executable(sign tools/sign.c)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_CURRENT_BINARY_DIR}/lib)


include_directories(
    src/base
    src/kernel/memory
    src/kernel/libs
    src/kernel/driver
    src/kernel/interrupt
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

set(CFLAGS -g -Wall -O0 -nostdlib -m32 -fno-builtin -nostdinc -fno-stack-protector)
set(LD_FLAGS -m32 -nostdlib)

file(GLOB_RECURSE KERNEL_SRC src/kernel/*.c)
set(INTER_SRC src/kernel/interrupt/trap_entry.S src/kernel/interrupt/vectors.S)
add_executable(kernel ${KERNEL_SRC} ${INTER_SRC})
target_compile_options(kernel PUBLIC ${CFLAGS})
target_link_options(kernel PUBLIC -Wl,-Ttext 0x100000 -Wl,-e kern_init ${LD_FLAGS})


set(IMAGE kernel.img)
add_custom_target(create_image
    COMMAND dd if=/dev/zero of=${IMAGE} count=10000
    COMMAND dd if=${EXECUTABLE_OUTPUT_PATH}/boot_block_bin of=${IMAGE} conv=notrunc
    COMMAND dd if=${EXECUTABLE_OUTPUT_PATH}/kernel of=${IMAGE} seek=1 conv=notrunc
    DEPENDS write_boot_block kernel)

add_custom_target(start_qemu
    COMMAND qemu-system-i386 -no-reboot -parallel stdio -hda ${CMAKE_CURRENT_BINARY_DIR}/${IMAGE} -serial null
    DEPENDS create_image)
