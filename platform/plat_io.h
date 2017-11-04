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

#ifndef _PLAT_C_IO_
#define _PLAT_C_IO_

#include "plat_type.h"

#if !_NO_STD_INC_
#ifdef __KERNEL__
// add header
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#endif

#if _NO_STD_INC_
#else
#ifdef __KERNEL__
#define plat_io_printf_std(fmt, args...) printk("<6> " fmt, args)
#define plat_io_printf_dbg(fmt, args...) printk("<6> " fmt, args)
#define plat_io_printf_err(fmt, args...) printk("<3> " fmt, args)
#define plat_io_flush_std()
#else
#define plat_io_printf_std(format, args...) fprintf(stdout, format, ##args)
#define plat_io_printf_dbg(format, args...) fprintf(stdout, "%s[%s:%d] " format, __TIME__, __FILE__, __LINE__, ##args)
#define plat_io_printf_err(format, args...) fprintf(stderr, "%s[%s:%d] " format, __TIME__, __FILE__, __LINE__, ##args)
#define plat_io_flush_std() fflush(stdout)
#endif
#endif

plat_inline int plat_io_get_resource(const char* resource_name, void** content_memory, uint* size)
{
#if _NO_STD_INC_
    return -1;
#else
    FILE *f;

    f = fopen(resource_name, "rb");

    if(!f)
    {
        return -1;
    }

    fseek(f, 0L, SEEK_END);
    *size = ftell(f);

    if (!(*size))
    {
        *content_memory = NULL;
        fclose(f);
        return 0;
    }

    fseek(f, 0L, SEEK_SET);

    *content_memory = malloc(*size);
    if (!(*content_memory))
    {
        fclose(f);
        return -1;
    }

    fread(*content_memory, *size, 1, f);
    fclose(f);

    return 0;
#endif
}


#define PRINTF_HEXMEM(/*function pointer*/PrintF, /*memory addr(void*)*/addr, /*unsigned int*/ size, /*unsigned int*/ _seg_size) \
do {                                                                                            \
   unsigned int i;                                                                              \
   unsigned char buff[17];                                                                      \
   unsigned char *pc = (unsigned char*)addr;                                                    \
                                                                                                \
                                                                                                \
   PrintF(" Address   0 1 2 3  4 5 6 7  8 9 a b  c d e f        Dump\n");                       \
   PrintF("--------  -------- -------- -------- --------  ----------------\n");                 \
                                                                                                \
   unsigned int seg_size = _seg_size;                                                           \
   seg_size >>= 4;                                                                              \
   seg_size <<= 4;                                                                              \
                                                                                                \
   for (i = 0; i < size; i++) {                                                                 \
      if ((i % 16) == 0) {                                                                      \
         if (i != 0) {                                                                          \
            PrintF("  %s\n", buff);                                                             \
            if ((seg_size != 0) && (i % seg_size == 0)) PrintF("\n");                           \
         }                                                                                      \
         PrintF("%08X ", i);                                                                    \
      }                                                                                         \
                                                                                                \
      if ((i % 4) == 0) PrintF(" ");                                                            \
      PrintF("%02x", pc[i]);                                                                    \
                                                                                                \
      if ((pc[i] < 0x20) || (pc[i] > 0x7e))                                                     \
         buff[i % 16] = '.';                                                                    \
      else                                                                                      \
         buff[i % 16] = pc[i];                                                                  \
                                                                                                \
      buff[(i % 16) + 1] = '\0';                                                                \
   }                                                                                            \
                                                                                                \
   while ((i % 16) != 0) {                                                                      \
      if ((i % 4) == 0) PrintF(" ");                                                            \
      PrintF("  ");                                                                             \
      i++;                                                                                      \
   }                                                                                            \
                                                                                                \
   PrintF("  %s\n", buff);                                                                      \
} while(0)


#define PRINTF_HEXMEM_TO_TARGET(/*function pointer*/PrintF, /*output target*/target, /*memory addr(void*)*/addr, /*unsigned int*/ size, /*unsigned int*/ _seg_size) \
do {                                                                                            \
   unsigned int i;                                                                              \
   unsigned char buff[17];                                                                      \
   unsigned char *pc = (unsigned char*)addr;                                                    \
                                                                                                \
                                                                                                \
   PrintF(target, " Address   0 1 2 3  4 5 6 7  8 9 a b  c d e f        Dump\n");               \
   PrintF(target, "--------  -------- -------- -------- --------  ----------------\n");         \
                                                                                                \
   unsigned int seg_size = _seg_size;                                                           \
   seg_size >>= 4;                                                                              \
   seg_size <<= 4;                                                                              \
                                                                                                \
   for (i = 0; i < size; i++) {                                                                 \
      if ((i % 16) == 0) {                                                                      \
         if (i != 0) {                                                                          \
            PrintF(target, "  %s\n", buff);                                                     \
            if ((seg_size != 0) && (i % seg_size == 0)) PrintF(target, "\n");                   \
         }                                                                                      \
         PrintF(target, "%08X ", i);                                                            \
      }                                                                                         \
                                                                                                \
      if ((i % 4) == 0) PrintF(target, " ");                                                    \
      PrintF(target, "%02x", pc[i]);                                                            \
                                                                                                \
      if ((pc[i] < 0x20) || (pc[i] > 0x7e))                                                     \
         buff[i % 16] = '.';                                                                    \
      else                                                                                      \
         buff[i % 16] = pc[i];                                                                  \
                                                                                                \
      buff[(i % 16) + 1] = '\0';                                                                \
   }                                                                                            \
                                                                                                \
   while ((i % 16) != 0) {                                                                      \
      if ((i % 4) == 0) PrintF(target, " ");                                                    \
      PrintF(target, "  ");                                                                     \
      i++;                                                                                      \
   }                                                                                            \
                                                                                                \
   PrintF(target, "  %s\n", buff);                                                              \
} while(0)

#endif //_PLAT_C_IO_
