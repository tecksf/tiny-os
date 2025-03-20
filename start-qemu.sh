#!/bin/bash

disk1=./cmake-build-debug/kernel.img
disk2=./cmake-build-debug/swap.img
disk3=./cmake-build-debug/user.img
disk4=./cmake-build-debug/temp.img

parameters="-no-reboot -parallel stdio -serial null"
if [ "$1" = "debug" ]; then
  parameters="-s -S "${parameters}
fi

echo $parameters

qemu-system-i386 ${parameters} \
  -drive id=disk1,if=ide,file=${disk1} \
  -drive id=disk2,if=ide,file=${disk2} \
  -drive id=disk3,if=ide,file=${disk3} \
  -drive id=disk4,if=ide,file=${disk4}
