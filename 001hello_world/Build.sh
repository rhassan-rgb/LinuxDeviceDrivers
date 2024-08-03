#!/bin/bash

ARCH=$(uname -m)
IS_CROSS=
CROSS_COMPILE=
KDIR=/lib/modules/$(uname -r)/build/

# Function to display help
usage() {
  echo "Usage: $0 --arch=ARCH --kern=KERNEL_SRC --cross=CROSS_COMPILE"
  echo "  --arch    Architecture Type, current: $(uname -m)"
  echo "  --kern    Kernel Source Location current ${KDIR}"
  echo "  --cross   Cross compiler prefix"
}

if [[ $# -eq 0 ]]; then
    usage
fi
# Parse command-line options
while [[ $# -gt 0 ]]; do
  case $1 in
    -a|--arch=*)
        if [[ $1 == --arch=* ]]; then
            ARCH="${1#*=}"
        else
            ARCH="$2"
            shift
        fi
        shift
        ;;
    -k|--kern=*)
        if [[ $1 == --kern=* ]]; then
            KDIR="${1#*=}"
        else
            KDIR="$2"
            shift
        fi
        shift
        ;;
    -c|--cross=*)
        IS_CROSS=true
        if [[ $1 == --cross=* ]]; then
            CROSS_COMPILE="${1#*=}"
        else
            CROSS_COMPILE="$2"
            shift
        fi
        shift
        ;;
    --clean)
        export KDIR
        make clean
        exit
        ;;
    *)
        echo "Invalid option: $1" >&2
        usage
        ;;
  esac
done

echo "Build With Parameters: "
echo "  ARCH: ${ARCH}"
echo "  KERNEL_SOURCE: ${KDIR}"
echo "  CROSS_COMPILE: ${CROSS_COMPILE}"

export ARCH
export KDIR

if [[ -n ${IS_CROSS} && -n ${ARCH} ]]; then
    echo "============================== CROSS COMPILING =============================="
    exit
fi 


echo "============================== NATIVE COMPILING =============================="

make host