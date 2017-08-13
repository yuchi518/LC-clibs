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
// Created by yuchi on 3/18/16.
//

#ifndef _PLAT_C_STRING_
#define _PLAT_C_STRING_

#include "plat_type.h"

#ifdef __KERNEL__
// TO-DO: add header
#else
#include <string.h>
#endif

plat_inline uint plat_cstr_length(const char* string)
{
    uint len = (uint)strlen(string);
    return len;
}



#endif //DYBUF_C_PLAT_STRING_H_H


