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

# Hold target user => default debian
TARGET_USER="debian"
# Hold target ip => default 192.168.7.2
TARGET_IP="192.168.7.2"

# send dtb flag
SEND_DTB="NO"

# send ko flag
SEND_KO="NO"

# send elf flag
SEND_ELF="NO"

#extentions to be sent
EXT_TOSEND=""


usage() {
  echo "Usage: $0 [--user=usernane] [--ip=ipv4_address] [option]"
  echo "  --user    target username, default \"debian\""
  echo "  --ip      target ipv4, default \"192.168.7.2\""
  echo "  --dtb     copy detb file name"
  echo "  --ko      copy ko (module) files"
  echo "  --elf     copy elf (executable) files"
}

if [[ $# -eq 0 ]]; then
    usage
    exit
fi

while [[ $# -gt 0 ]]; do
    case $1 in
    --user=*) 
        # passing user --user=
        TARGET_USER="${1#*=}"
        #shift rguments to point to the next one 
        shift
    ;;
    --ip=*)
        # passing target ip --ip=
        TARGET_IP="${1#*=}"
        #shift rguments to point to the next one 
        shift
    ;;
    --dtb)
        # dtb files to be sent
        SEND_DTB="YES"
        EXT_TOSEND="${EXT_TOSEND} *.dtb"
        shift
    ;;
    --ko)
        # ko files to be sent
        SEND_KO="YES"
        EXT_TOSEND="${EXT_TOSEND} *.ko"
        shift
    ;;
    --elf)
        # elf files to be sent
        SEND_ELF="YES"
        EXT_TOSEND="${EXT_TOSEND} *.elf"
        shift
    ;;
    -v)
        # make scp verbose
        EXT_TOSEND="-v ${EXT_TOSEND}"
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

echo copying....
scp ${EXT_TOSEND} ${TARGET_USER}@${TARGET_IP}:/home/${TARGET_USER}/inbox/