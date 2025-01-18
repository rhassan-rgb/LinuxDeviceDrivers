#ifndef RGB_PLATFORM_H
#define RGB_PLATFORM_H
/*
 * This file is part of Linux Device Drivers (LDD) project.
 *
 * Linux Device Drivers is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Linux Device Drivers is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Linux Device Drivers. If not, see <https://www.gnu.org/licenses/>.
 */
#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

#define RDONLY 0x01u
#define WRONLY 0x10u
#define RDWR 0x11u

struct pcdev_platform_data
{
    int size;
    int perm;
    const char* serial_number;
};

#endif