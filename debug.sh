#!/bin/bash

image=/home/tang/Programs/C/foots-stone/cmake-build-debug/kernel.img
qemu-system-i386 -s -S -no-reboot -parallel stdio -hda ${image} -serial null
