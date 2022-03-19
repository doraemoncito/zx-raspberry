#!/usr/bin/env bash

echo "Building stage 1 boot script:"
echo
mkimage -A arm -O linux -T script -C none -n "ZX Raspberry stage 1 bootloader" -d boot1.scr -a 0x01000000 -e 0x01000000 boot.scr.uimg
#dumpimage -l boot.scr.uimg
echo

echo "Building stage 2 boot script:"
echo
mkimage -A arm -O linux -T script -C none -n "ZX Raspberry stage 2 bootloader" -d boot2.scr -a 0x01100000 -e 0x01100000 /private/tftpboot/boot.scr.uimg
#dumpimage -l /private/tftpboot/boot.scr.uimg
echo

#echo "Building test image:"
#sudo mkimage -n "TEST 1.0.0" -A arm -O linux -T kernel -C none -a 0x00008000 -e 0x00008000 -d u-boot/u-boot.bin /private/tftpboot/zxraspberry.uimg
