/*
 * plat_c, platform independent library for c
 * Copyright (C) 2017 Yuchi (yuchi518@gmail.com)

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


#ifndef _PLAT_C_MGN_MEM_H_
#define _PLAT_C_MGN_MEM_H_

/**
 * Managed memory API
 * 1. Declare a pool
 *    mgn_memory_pool pool = null;
 * 2. Allocate a memory
 *    void* m0 = mgn_mem_alloc(&pool, 100);
 * 3. Release a memory
 *    mgn_mem_release(&pool, m0);
 * 4. Extend a memory
 *    a) one owner
 *    m0 = mgn_mem_ralloc(&pool, m0, 120);
 *    b) more than one owner
 *    mgn_mem_release(&pool, m0);
 *    m0 = mgn_mem_alloc(&pool, 120);
 * 5. Retain a memory
 *    mgn_mem_retain(&pool, m0);
 * 6. Release a memory automatically
 *    mgn_mem_autorelease(&pool, m0);
 * 7. Release unused memories (auto)
 *    mgn_mem_release_unused(&pool);
 * 8. Release all memories
 *    mgn_mem_release_all(&pool);
 *
 */


#include "plat_type.h"
#include "plat_mem.h"
#include "plat_io.h"
#include "uthash.h"

typedef void (*callback_before_mem_released)(void* mem);
typedef struct _MGN_MEM_ {
    void* m;  // memory
    size_t s; // memory size
    size_t r; // retained count
    callback_before_mem_released cb;        // called before released, each mem should maintain different function ptr.
    // ***** Hash table anchor *****
    UT_hash_handle hh; /* makes this structure hashable */
} mgn_memory, *mgn_memory_pool;

#if 0
#define print_mgn_mem_err plat_io_printf_err
#define print_mgn_mem_dbg
#else
#define print_mgn_mem_err plat_io_printf_err
#define print_mgn_mem_dbg plat_io_printf_std
#endif

#define MGN_MEM_REALLOCATE_IF_MULTI_OWNERS        1

// This function shuold be used carefully, caller should always have its ownership,
// else it is difficult to decide how to maintain retained count.
// Should never call this function after autorelease.
static inline void* mgn_mem_ralloc(mgn_memory_pool* pool, void* origin_mem, size_t new_size)
{
    if (new_size == 0) return NULL;
    mgn_memory* mgn_m = NULL;
    HASH_FIND_PTR((*pool), &origin_mem, mgn_m);
    if (NULL != mgn_m)
    {
        if (new_size > mgn_m->s)
        {
            if (1 == mgn_m->r)
            {
                // realloc a new one
                HASH_DEL((*pool), mgn_m);
                void* new_m = plat_mem_allocate(new_size);
                if (NULL == new_m)
                {
                    print_mgn_mem_err("[MGN_MEM] Allocate memory error\n");
                    return NULL;                                    // error
                }
                plat_mem_copy(new_m, mgn_m->m, mgn_m->s);           // clone
                plat_mem_release(mgn_m->m);
                print_mgn_mem_dbg("[MGN_MEM] Removed memory (%p), size %zu - %u left\n", mgn_m->m, mgn_m->s, HASH_COUNT(*pool));
                mgn_m->m = new_m;
                mgn_m->s = new_size;
                HASH_ADD_PTR((*pool), m, mgn_m);                    // reset key
                print_mgn_mem_dbg("[MGN_MEM] Added memory (%p), size %zu - %u left\n", mgn_m->m, mgn_m->s, HASH_COUNT(*pool));
            }
            else
            {
                // more than one owners, give up ownership
                if (mgn_m->r > 0) mgn_m->r--;                           // if someone call ralloc after autorelease, should not decrease retained count
#if MGN_MEM_REALLOCATE_IF_MULTI_OWNERS
                // create a new one
                size_t old_size = mgn_m->s;
                mgn_m = plat_mem_allocate(sizeof(*mgn_m));              // allocate a zero memory
                if (NULL == mgn_m)
                {
                    print_mgn_mem_err("[MGN_MEM] Allocate memory error\n");
                    return NULL;                                        // error
                }
                void* new_m = plat_mem_allocate(new_size);              // allocate a zero memory
                if (NULL == new_m)
                {
                    print_mgn_mem_err("[MGN_MEM] Allocate memory error\n");
                    plat_mem_release(mgn_m);
                    return NULL;                                        // error
                }
                plat_mem_copy(new_m, origin_mem, old_size);             // clone
                mgn_m->m = new_m;
                mgn_m->s = new_size;
                mgn_m->r = 1;
                HASH_ADD_PTR((*pool), m, mgn_m);
                print_mgn_mem_dbg("[MGN_MEM] Added memory (%p), size %zu - %u left\n", mgn_m->m, mgn_m->s, HASH_COUNT(*pool));
#else
                // let user to decide how to do
                return NULL;
#endif
            }
        }
    }
    else
    {
        if (NULL != origin_mem)
        {
            print_mgn_mem_err("[MGN_MEM] Who's memory (%p)?\n", origin_mem);
            return NULL;                                       // where the origin_mem from?
        }
        mgn_m = plat_mem_allocate(sizeof(*mgn_m));             // allocate a zero memory
        if (NULL == mgn_m)
        {
            print_mgn_mem_err("[MGN_MEM] Allocate memory error\n");
            return NULL;                                       // error
        }
        void* new_m = plat_mem_allocate(new_size);             // allocate a zero memory
        if (NULL == new_m)
        {
            print_mgn_mem_err("[MGN_MEM] Allocate memory error\n");
            plat_mem_release(mgn_m);
            return NULL;                                       // error
        }
        mgn_m->m = new_m;
        mgn_m->s = new_size;
        mgn_m->r = 1;
        HASH_ADD_PTR((*pool), m, mgn_m);
        print_mgn_mem_dbg("[MGN_MEM] Added memory (%p), size %zu - %u left\n", mgn_m->m, mgn_m->s, HASH_COUNT(*pool));
    }

    return mgn_m->m;
}

static inline void* mgn_mem_alloc(mgn_memory_pool* pool, size_t new_size)
{
    return mgn_mem_ralloc(pool, NULL, new_size);
}

static inline void* mgn_mem_retain(mgn_memory_pool* pool, void* origin_mem)
{
    mgn_memory* mgn_m = NULL;
    HASH_FIND_PTR((*pool), &origin_mem, mgn_m);
    if (NULL != mgn_m)
    {
        mgn_m->r++;
        return mgn_m->m;
    }
    else
    {
        print_mgn_mem_err("[MGN_MEM] retain - Who's memory (%p)?\n", origin_mem);
        return NULL;
    }
}

static inline void _mgn_mem_release(mgn_memory_pool* pool, void* origin_mem, int release, callback_before_mem_released cb)
{
    mgn_memory* mgn_m = NULL;
    HASH_FIND_PTR((*pool), &origin_mem, mgn_m);
    if (NULL != mgn_m)
    {
        if (0 == mgn_m->r)
        {
            // error
            print_mgn_mem_err("MGN_MEM] Memory (%p) can't be released, retained count is zero.\n", mgn_m->m);
            return;
        }
        mgn_m->r--;
        mgn_m->cb = cb;
        if (0 == mgn_m->r && release)
        {
            if (cb) cb(mgn_m->m);
            HASH_DEL((*pool), mgn_m);
            plat_mem_release(mgn_m->m);
            print_mgn_mem_dbg("[MGN_MEM] Removed memory (%p), size %zu - %u left\n", mgn_m->m, mgn_m->s, HASH_COUNT(*pool));
            plat_mem_release(mgn_m);
        }
        else
        {
            print_mgn_mem_dbg("[MGN_MEM] Not Removed memory (%p) , size %zu retain count %zu, - %u left\n", mgn_m->m, mgn_m->s, mgn_m->r, HASH_COUNT(*pool));
        }
    }
    else
        print_mgn_mem_err("[MGN_MEM] Release - Who's memory (%p)?\n", origin_mem);    // error case ?
}

static inline void mgn_mem_release(mgn_memory_pool* pool, void* origin_mem)
{
    _mgn_mem_release(pool, origin_mem, 1, null);
}

static inline void mgn_mem_release_w_cb(mgn_memory_pool* pool, void* origin_mem, callback_before_mem_released cb)
{
    _mgn_mem_release(pool, origin_mem, 1, cb);
}

static inline void mgn_mem_autorelease(mgn_memory_pool* pool, void* origin_mem)
{
    _mgn_mem_release(pool, origin_mem, 0, null);
}

static inline void mgn_mem_autorelease_w_cb(mgn_memory_pool* pool, void* origin_mem, callback_before_mem_released cb)
{
    _mgn_mem_release(pool, origin_mem, 0, cb);
}

static inline void mgn_mem_release_unused(mgn_memory_pool* pool)
{
    mgn_memory* mgn_m;
    mgn_memory* tmp;
    HASH_ITER(hh, (*pool), mgn_m, tmp)
    {
        if (0 == mgn_m->r)
        {
            if (mgn_m->cb) mgn_m->cb(mgn_m->m);
            HASH_DEL((*pool), mgn_m);
            plat_mem_release(mgn_m->m);
            print_mgn_mem_dbg("[MGN_MEM] Removed memory (%p), size %zu - %u left\n", mgn_m->m, mgn_m->s, HASH_COUNT(*pool));
            plat_mem_release(mgn_m);
        }
    }
}

static inline void mgn_mem_release_all(mgn_memory_pool* pool)
{
    mgn_memory* mgn_m;
    mgn_memory* tmp;
    HASH_ITER(hh, (*pool), mgn_m, tmp)
    {
        // do not call released cb, because of unordered objects' allocation
        HASH_DEL((*pool), mgn_m);
        plat_mem_release(mgn_m->m);
        print_mgn_mem_dbg("[MGN_MEM] Removed memory (%p), size %zu - %u left\n", mgn_m->m, mgn_m->s, HASH_COUNT(*pool));
        plat_mem_release(mgn_m);
    }
}

static inline size_t mgn_mem_retained_count(mgn_memory_pool* pool, void* origin_mem)
{
    mgn_memory* mgn_m = NULL;
    HASH_FIND_PTR((*pool), &origin_mem, mgn_m);
    if (NULL != mgn_m)
    {
        return mgn_m->r;
    }
    else
    {
        print_mgn_mem_err("[MGN_MEM] Retained Count - Who's memory (%p)?\n", origin_mem);
        return 0;
    }
}

#endif