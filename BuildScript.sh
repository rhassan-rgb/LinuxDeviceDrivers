#!/bin/bash

#####################################################################################
# This file is part of Linux Device Drivers (LDD) project.                          #
#                                                                                   #
# Linux Device Drivers is free software: you can redistribute it and/or modify      #
# it under the terms of the GNU General Public License as published by              #
# the Free Software Foundation, either version 3 of the License, or                 #
# (at your option) any later version.                                               #
#                                                                                   #
# Linux Device Drivers is distributed in the hope that it will be useful,           #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                    #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                      #
# GNU General Public License for more details.                                      #
#                                                                                   #
# You should have received a copy of the GNU General Public License                 #
# along with Linux Device Drivers. If not, see <https://www.gnu.org/licenses/>.     #
#####################################################################################

#holds the architecture
ARCH=
#holds the cross compiler prefix
CROSS_COMPILE=
# holds the kernel directory
KDIR=
#indicates that a clean is required
IS_CLEAN="NO"
#indicate if it's cross compiled
IS_CROSS=
#extra flags
EXTRA_CFLAGS=

# copy dtb file
DTB_FILE=
COPY_DTB=

# Function to display help
usage() {
  echo "Usage: $0 --arch=ARCH --kern=KERNEL_SRC --cross=CROSS_COMPILE"
  echo "  --arch    Architecture Type, current: $(uname -m)"
  echo "            could be: arm, arm64, ...etc"
  echo "  --kern    Kernel Source Location current /lib/modules/$(uname -r)/build/"
  echo "  --cross   Cross compiler prefix"
  echo "            could be: arm-linux-gnueabihf- , arm-none-linux-gnueabi-, ...etc"
  echo "  --get_dtb copy dtb file to current location, currently only arm supported"
}

# Parse command-line options
# $# => number of arguments
while [[ $# -gt 0 ]]; do
# switch case
    # $1 first argument of the argument list (curent argument)
    case $1 in
        -a|--arch=*) 
            # Selecting the architecture e.g. -a arm or --arch=arm
            # ARCH passed as --arch=
            if [[ $1 == --arch=* ]]; then
                # slice the string after =
                ARCH="${1#*=}"
            else
            # ARCH passed as -a  
                # get the next argument
                ARCH="$2"
                # shft arguments so $1 is equal to $2
                shift
            fi
            #shift rguments to point to the next one 
            shift
        ;;
        -k|--kern=*)
            # Selecting the kernel Location e.g. -k /path/to/kernel or --kern=/path/to/kernel
            # kernel passed as --kern=
            if [[ $1 == --kern=* ]]; then
            # slice the string after =
                KDIR="${1#*=}"
            else
            # KERN passed as -k
                # get the next argument
                KDIR="$2"
                # shft arguments so $1 is equal to $2
                shift
            fi
            #shift rguments to point to the next one 
            shift
        ;;
        -c|--cross=*)
            # Selecting the cross compiler prefix e.g. -c comp-prefix- or --cross=comp-prefix-
            # cross toolcahin prefix passed as --cross=
            if [[ $1 == --cross=* ]]; then
                # slice the string after =
                CROSS_COMPILE="${1#*=}"
            else
            # cross toolchain passed as -c
                # get the next argument
                CROSS_COMPILE="$2"
                # shft arguments so $1 is equal to $2
                shift
            fi
            IS_CROSS=${CROSS_COMPILE}
            #shift rguments to point to the next one 
            shift
        ;;
        --clean)
            # chatching clean will tringger make clean
            IS_CLEAN="YES"
            shift
        ;;
        --get-dtb=*)
            # slice the string after =
            DTB_FILE="${1#*=}"
            COPY_DTB="YES"
            shift
        ;; 
        *)
            # passing any invalid option will cause the script to fail
            echo "Invalid option: $1"
            usage
            exit
        ;;
  esac
done

# check variables
# native compile
if [[ -z ${IS_CROSS} ]]; then
    ARCH=$(uname -m)
    KDIR="/lib/modules/$(uname -r)/build/"
    CROSS_COMPILE=""
    IS_CROSS="NATIVE"
    EXTRA_CFLAGS="-DHOST"
fi

# export variables required by make
export ARCH
export KDIR
export CROSS_COMPILE
export EXTRA_CFLAGS


# state the build parameters
echo "Build With Parameters: "
echo "  ARCH         : ${ARCH}"
echo "  KERNEL_SOURCE: ${KDIR}"
echo "  CROSS_COMPILE: ${IS_CROSS}"
echo "  IS_CLEAN     : ${IS_CLEAN}"
echo "  EXTRA_FLAGS  : ${EXTRA_FLAGS}"


# check for clean
if [[ "YES" == ${IS_CLEAN} ]]; then
    make clean
    exit
fi

make

if [[ 0 -eq $? && -n ${COPY_DTB} ]]; then
    echo "Copying ${DTB_FILE}..."
    cp ${KDIR}/arch/arm/boot/dts/${DTB_FILE} .
fi
