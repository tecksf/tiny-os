#!/bin/bash

main_image=/home/tang/Programs/C/foots-stone/cmake-build-debug/kernel.img
disk2=/home/tang/Programs/C/foots-stone/cmake-build-debug/disk2.img
disk3=/home/tang/Programs/C/foots-stone/cmake-build-debug/disk3.img
disk4=/home/tang/Programs/C/foots-stone/cmake-build-debug/disk4.img

qemu-system-i386 -s -S -no-reboot -parallel stdio -serial null \
  -drive id=disk1,if=ide,file=${main_image} \
  -drive id=disk2,if=ide,file=${disk2} \
  -drive id=disk3,if=ide,file=${disk3} \
  -drive id=disk4,if=ide,file=${disk4}
