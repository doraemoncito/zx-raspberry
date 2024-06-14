#!/usr/bin/env bash

echo "Synchronizing the Virtual Machine's clock"
sudo ntpdate time.nist.gov

# Number of make jobs to run = number of CPUs + 1
export NUM_JOBS=$(($(nproc) + 1))

## DEBUGGING:
## build-boot-image.sh > build.log 2>&1
##CROSS_COMPILE=arm-none-eabi- make ARCH=arm --trace -j "${NUM_JOBS}" distclean rpi_defconfig all

#echo "Compiling U-Boot for Raspberry Pi 0 W (32 bits) using ${NUM_JOBS} jobs"
#CROSS_COMPILE=arm-none-eabi- make ARCH=arm -j "${NUM_JOBS}" distclean rpi_0_w_defconfig all
#file u-boot
#cp u-boot.bin ../kernel.img

#echo "Compiling U-Boot for Raspberry Pi 1 (32 bits)"
#CROSS_COMPILE=arm-none-eabi- make ARCH=arm -j "${NUM_JOBS}" distclean rpi_defconfig all
#file u-boot
#cp u-boot.bin ../kernel.img

#echo "Compiling U-Boot for Raspberry Pi 2 (32 bits)"
#CROSS_COMPILE=arm-none-eabi- make -j "${NUM_JOBS}" ARCH=arm distclean rpi_2_defconfig all
#file u-boot
#mv u-boot.bin ../kernel7.img
#
#echo "Compiling U-Boot for Raspberry Raspberry Pi 3 Model B+ (32 bits)"
#CROSS_COMPILE=arm-none-eabi- make -j "${NUM_JOBS}" ARCH=arm distclean rpi_3_32b_defconfig all
#file u-boot
#mv u-boot.bin ../kernel8-32.img

echo "Compiling U-Boot for Raspberry Pi 4B (32 bits)"
CROSS_COMPILE=arm-none-eabi- make -j "${NUM_JOBS}" ARCH=arm distclean rpi_4_32b_defconfig all
file u-boot
cp u-boot.bin ../kernel7l.img

#echo "Compiling U-Boot for Raspberry Pi 4B (64 bits)"
#CROSS_COMPILE=aarch64-none-elf- make -j "${NUM_JOBS}" distclean rpi_4_defconfig all
#file u-boot
#cp u-boot.bin ../kernel8.img

echo "Done!"
