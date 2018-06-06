/*
 * dybuf, dynamic buffer library
 * Copyright (C) 2015-2016 Yuchi (yuchi518@gmail.com)

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
// Created by Yuchi on 2015/12/19.
//

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "dybuf.h"
#include "dypkt.h"
#include "plat_mgn_mem.h"
#include "mmo_unittest.h"

void dybuf_test(void)
{
    dybuf dyb0, *dyb1;
    int i=0;
    int64 v0, v1;
    double d0, d1;
    uint size;
    int diff = 0;

    for (i=0; i<1000; i++)
    {
        dyb_create(&dyb0, 32);

        v0 = rand();
        v0 *= rand();
        d0 = (double)v0/rand()/rand();

        dyb_append_var_s64(&dyb0, v0);
        dyb_append_double(&dyb0, d0);

        uint8* data = dyb_get_data_before_current_position(&dyb0, &size);

        dyb1 = dyb_copy(null, data, size, false);

        v1 = dyb_next_var_s64(dyb1);
        d1 = dyb_next_double(dyb1);

        if (v0 == v1)
            ;//plat_io_printf_std("v0(%lld) == v1(%lld)\n", v0, v1);
        else {
            plat_io_printf_std("v0(%lld) != v1(%lld)\n", v0, v1);
            diff++;
        }

        if (d0 == d1)
            ;//plat_io_printf_std("d0(%f) == d1(%f)\n", d0, d1);
        else{
            plat_io_printf_std("d0(%f) != d1(%f)\n", d0, d1);
            diff++;
        }

        dyb_release(&dyb0);
        dyb_release(dyb1);
    }

    plat_io_printf_std("diff: %d\n", diff);
}

void dybuf_test_ref(void)
{
    dybuf dyb0, dyb1;
    unsigned char data[32];
    int i=0;
    int64 v0, v1;
    double d0, d1;
    //uint size;
    int diff = 0;

    for (i=0; i<1000; i++)
    {
        dyb_refer(&dyb0, data, 32, true);

        v0 = rand();
        v0 *= rand();
        d0 = (double)v0/rand()/rand();

        dyb_append_var_s64(&dyb0, v0);
        dyb_append_double(&dyb0, d0);

        //uint8_t* data = dyb_get_data_before_current_position(&dyb0, &size);
        dyb_release(&dyb0);

        dyb_refer(&dyb1, data, 32, true);

        v1 = dyb_next_var_s64(&dyb1);
        d1 = dyb_next_double(&dyb1);

        if (v0 == v1)
            ;//plat_io_printf_std("v0(%lld) == v1(%lld)\n", v0, v1);
        else {
            plat_io_printf_std("v0(%lld) != v1(%lld)\n", v0, v1);
            diff++;
        }

        if (d0 == d1)
            ;//plat_io_printf_std("d0(%f) == d1(%f)\n", d0, d1);
        else{
            plat_io_printf_std("d0(%f) != d1(%f)\n", d0, d1);
            diff++;
        }

        dyb_release(&dyb1);
    }

    plat_io_printf_std("diff: %d\n", diff);
}

void dypkt_test(void)
{
    uint8 mem[1024];
    dypkt* dyp0 = dyp_pack(null, mem, 1024);
    dypkt* dyp1;
    uint index = 0;
    uint size;
    dype type;
    dyp_append_protocol(dyp0, "json");
    dyp_append_protocol_version(dyp0, 0x01020304u);
    dyp_append_bool(dyp0, index++, true);
    dyp_append_int(dyp0, index++, (int64)-1ll);
    dyp_append_uint(dyp0, index++, (uint64)-1ll);
    dyp_append_cstring(dyp0, index++, "I love dybuf lib!!");
    dyp_append_float(dyp0, index++, 0.1);
    dyp_append_double(dyp0, index++, 0.2);
    dyp_append_eof(dyp0);

    size = dyp_get_position(dyp0);
    dyp_release(dyp0);

    dyp1 = dyp_unpack(null, mem, size, false);

    while(dyp_get_remainder(dyp1)>0)
    {
        type = dyp_next_type(dyp1, &index);
        if (type==dype_f) plat_io_printf_std("_f.");
        else plat_io_printf_std("%2d.", index);
        switch (type)
        {
            case dype_f:
            {
                switch((dype_fid)index)
                {
                    case dype_f_eof:
                    {
                        dyp_next_eof(dyp1);
                        plat_io_printf_std("eof\n");
                        break;
                    }
                    case dype_f_version:
                    {
                        plat_io_printf_std("version: %llx\n", dyp_next_version(dyp1));
                        break;
                    }
                    case dype_f_protocol:
                    {
                        plat_io_printf_std("protocol: %s\n", dyp_next_protocol(dyp1, null));
                        break;
                    }
                    case dype_f_proto_version:
                    {
                        plat_io_printf_std("proto_ver: %llx\n", dyp_next_protocol_version(dyp1));
                        break;
                    }
                }
                break;
            }
            case dype_bool:
            {
                plat_io_printf_std("boolean: %s\n", dyp_next_bool(dyp1)?"true":"false");
                break;
            }
            case dype_int:
            {
                plat_io_printf_std("int: %llx\n", dyp_next_int(dyp1));
                break;
            }
            case dype_uint:
            {
                plat_io_printf_std("uint: %llx\n", dyp_next_uint(dyp1));
                break;
            }
            case dype_float:
            {
                plat_io_printf_std("float: %f\n", dyp_next_float(dyp1));
                break;
            }
            case dype_double:
            {
                plat_io_printf_std("double: %f\n", dyp_next_double(dyp1));
                break;
            }
            case dype_string:
            {
                plat_io_printf_std("cstr: %s\n", dyp_next_cstring(dyp1, null));
                break;
            }
            default:
            {
                // TODO: implement
                plat_io_printf_err("Not support case(%d).\n", type);
            }
        }
    }

    plat_io_printf_std("remainder: %u\n", dyp_get_remainder(dyp1));
    plat_io_printf_std("size: %u ? %u\n", size, dyp_get_position(dyp1));

    dyp_release(dyp1);
}

void mgn_m_test(void)
{
    mgn_memory_pool pool = NULL;

    void *m = NULL, *m2, *m3;
    m = mgn_mem_alloc(&pool, 100);
    m2 = mgn_mem_alloc(&pool, 100);
    m3 = mgn_mem_alloc(&pool, 100);
    m = mgn_mem_ralloc(&pool, m, 120);
    mgn_mem_retain(&pool, m3);
    mgn_mem_release(&pool, m);
    mgn_mem_release(&pool, m3);
    mgn_mem_autorelease(&pool, m3);
    mgn_mem_release_unused(&pool);
    mgn_mem_release_all(&pool);

}

#if defined(UNIT_TEST) && UNIT_TEST == 1
int main(int argc, char **argv)
{
    srand(time(NULL));

    dybuf_test();
    dybuf_test_ref();
    dypkt_test();

    mgn_m_test();

    unit_test_mmobj();

    return 0;
}

#endif

