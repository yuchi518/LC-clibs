/*
 * plat_c, platform independent library for c
 * Copyright (C) 2016 Yuchi (yuchi518@gmail.com)

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses>.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

//
// Created by Yuchi on 2016/2/12.
//

#ifndef _PLAT_C_TYPE_
#define _PLAT_C_TYPE_

#ifndef _NO_STD_INC_
#define _NO_STD_INC_    0           // 0: use standard lib (c or system lib), 1: disable (use to test platform lib independence)
#endif


typedef enum {
    false = 0,
    true,
} __attribute__((packed)) boolean;

typedef boolean bool;

typedef unsigned char       byte;
typedef unsigned int        uint;
typedef signed char         int8;
typedef signed short        int16;
typedef signed int          int32;
typedef signed long long    int64;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

#define null            (0)
#define nil             (0)
#define plat_inline     static inline

#endif //_PLAT_C_TYPE_



