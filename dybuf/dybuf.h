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

#ifndef DYBUF_C_DYBUF_H
#define DYBUF_C_DYBUF_H


#include "plat_type.h"
#include "plat_mem.h"
#include "plat_string.h"

#define dyb_inline              plat_inline

#define CACHE_SIZE_UNIT         16U

#ifndef MAX
#define MAX(a,b)                ((a)>=(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b)                ((a)<=(b)?(a):(b))
#endif

/**
 *  Memory allocator, create a memory and
 *  its size is larger or equal than (*size).
 *  TO-DO: reused memory
 */
dyb_inline void* dyb_mem_alloc(uint *size, boolean dyn)
{
    if (dyn)
    {
        *size = MAX(CACHE_SIZE_UNIT,*size);
        // TO-DO: reuse algorithm
        return plat_mem_allocate(*size);
    }
    else
        // fixed size
        return plat_mem_allocate(*size);
}

dyb_inline void dyb_mem_release(void* buf, uint size)
{
    plat_mem_release(buf);
}

dyb_inline void dyb_mem_copy(void* dest, void* src, uint size)
{
    plat_mem_copy(dest, src, size);
}

dyb_inline void dyb_mem_move(void* dest, void* src, uint size)
{
    plat_mem_move(dest, src, size);
}

dyb_inline uint32 dyb_swap_u32(uint32 value)
{
    union{
        uint32    u;
        uint8     bs[4];
    } v;
    //

    const int i = 1;
    if (((char*)&i)[0])
    {
        // little endian
        v.u = value;
        uint8 b;
        b = v.bs[0]; v.bs[0] = v.bs[3]; v.bs[3] = b;
        b = v.bs[1]; v.bs[1] = v.bs[2]; v.bs[2] = b;
        return v.u;
    }
    else
        return value;
}

dyb_inline uint64 dyb_swap_u64(uint64 value)
{
    union {
        uint64    u;
        uint8     bs[8];
    } v;

    const int i = 1;
    if (((char*)&i)[0]) {
        // little endian
        v.u = value;
        uint8 b;
        b = v.bs[0]; v.bs[0] = v.bs[7]; v.bs[7] = b;
        b = v.bs[1]; v.bs[1] = v.bs[6]; v.bs[6] = b;
        b = v.bs[2]; v.bs[2] = v.bs[5]; v.bs[5] = b;
        b = v.bs[3]; v.bs[3] = v.bs[4]; v.bs[4] = b;
        return v.u;
    }
    else
        return value;
}


/**
 * 0 <= mark <= position <= limit <= capacity
 * 1. clear() makes a buffer ready for a new sequence of channel-read or relative put operations:
 * It sets the limit to the capacity and the position to zero.
 * 2. flip() makes a buffer ready for a new sequence of channel-write or relative get operations:
 * It sets the limit to the current position and then sets the position to zero.
 * 3. rewind() makes a buffer ready for re-reading the data that it already contains:
 * It leaves the limit unchanged and sets the position to zero.
 */
struct dybuf
{
    byte* _data;
    uint _capacity;
    uint _limit;
    uint _position;
    uint _mark;
    boolean _fixedCapacity;

    boolean _should_release_instance;
    boolean _should_release_data;
};
typedef struct dybuf dybuf;

// reference mode, use memory from outside and never release/create the memory
dyb_inline dybuf* dyb_refer(dybuf* dyb, byte* data, uint capacity, boolean for_write)
{
    if (data == null)
    {
        // reference mode should pass data
        return null;
    }

    if (dyb == null)
    {
        uint size = sizeof(*dyb);
        dyb = (dybuf*)dyb_mem_alloc(&size, false);
        if (dyb == null) return null;
        dyb->_should_release_instance = true;
    }
    else
    {
        dyb->_should_release_instance = false;
    }

    dyb->_data = data;
    dyb->_capacity = capacity;
    dyb->_fixedCapacity = true;
    dyb->_should_release_data = false;

    if (for_write)
    {
        dyb->_limit = 0;
    }
    else
    {
        dyb->_limit = capacity;
    }

    dyb->_position = 0;
    dyb->_mark = 0;

    return dyb;
}

// for write mode
dyb_inline dybuf* dyb_create(dybuf* dyb, uint capacity)
{
    if (dyb == null)
    {
        uint size = sizeof(*dyb);
        dyb = (dybuf*)dyb_mem_alloc(&size, false);
        if (dyb == null) return null;
        dyb->_should_release_instance = true;
    }
    else
    {
        dyb->_should_release_instance = false;
    }

    dyb->_data = (byte*)dyb_mem_alloc(&capacity, true);
    dyb->_capacity = capacity;
    dyb->_limit = 0;
    dyb->_position = 0;
    dyb->_mark = 0;
    dyb->_fixedCapacity = false;
    dyb->_should_release_data = true;

    return dyb;
}

// for read mode
dyb_inline dybuf* dyb_copy(dybuf* dyb, byte* data, uint capacity, boolean no_copy)
{
    if (data == null) {
        return dyb_create(dyb, capacity);
    }

    if (dyb == null)
    {
        uint size = sizeof(*dyb);
        dyb = (dybuf*)dyb_mem_alloc(&size, false);
        if (dyb == null) return null;
        dyb->_should_release_instance = true;
    }
    else
    {
        dyb->_should_release_instance = false;
    }

    if (no_copy) {
        dyb->_data = data;
        dyb->_capacity = dyb->_limit = capacity;
    } else {
        uint origin_capacity = capacity;
        dyb->_data = (byte*)dyb_mem_alloc(&capacity, true);
        dyb_mem_copy(dyb->_data, data, origin_capacity);
        dyb->_limit = origin_capacity;
        dyb->_capacity = capacity;
    }

    dyb->_position = dyb->_mark = 0;
    dyb->_fixedCapacity = false;
    dyb->_should_release_data = true;

    return dyb;
}

dyb_inline void dyb_release(dybuf* dyb)
{
    if (dyb == null) return;

    if (dyb->_should_release_data)
    {
        dyb_mem_release(dyb->_data, dyb->_capacity);
    }

    if (dyb->_should_release_instance)
    {
        dyb_mem_release(dyb, sizeof(*dyb));
    }
}

dyb_inline int dyb_get_capacity(dybuf* dyb)
{
    return dyb->_capacity;
}

dyb_inline dybuf* dyb_set_capacity(dybuf* dyb, uint newCapacity)
{
    if (dyb->_fixedCapacity && newCapacity!=dyb->_capacity) {
        // error
        return null;
    }

    if (newCapacity == dyb->_capacity) return dyb;

    byte *newData = (byte*)dyb_mem_alloc(&newCapacity, true);
    dyb_mem_copy(newData, dyb->_data, MIN(dyb->_capacity, newCapacity));
    dyb_mem_release(dyb->_data, dyb->_capacity);
    dyb->_capacity = newCapacity;
    dyb->_data = newData;
    newData = null;

    if (dyb->_limit >= dyb->_capacity) {
        dyb->_limit = dyb->_capacity;
    }

    if (dyb->_position >= dyb->_capacity) {
        dyb->_position = dyb->_capacity;
    }

    if (dyb->_mark >= dyb->_capacity) {
        dyb->_mark = dyb->_capacity;
    }

    return dyb;
}

dyb_inline uint dyb_get_position(dybuf* dyb)
{
    return dyb->_position;
}

dyb_inline dybuf* dyb_set_position(dybuf* dyb, uint newPosition)
{
    if (newPosition > dyb->_limit) {
        // error
        return null;
    }

    if (newPosition < dyb->_mark) {
        // discard or set the same value
        dyb->_position = dyb->_mark;
    } else {
        dyb->_position = newPosition;
    }

    return dyb;
}

dyb_inline uint dyb_get_limit(dybuf* dyb)
{
    return dyb->_limit;
}

dyb_inline dybuf* dyb_set_limit(dybuf* dyb, uint newLimit)
{
    if (newLimit > dyb->_capacity) {
        if (dyb_set_capacity(dyb, newLimit) == null) {
            // error
            return null;
        }
    }

    dyb->_limit = newLimit;

    if (dyb->_mark > dyb->_limit) {
        dyb->_mark = dyb->_limit;
    }

    if (dyb->_position > dyb->_limit) {
        dyb->_position = dyb->_limit;
    }

    return dyb;
}

/**
 * The bytes between the buffer's current position and its limit, if any, are copied to the beginning of the buffer.
 * This is user for read. If use for write, the copy is meaningless.
 *
 * @return
 */
dyb_inline dybuf* dyb_compact(dybuf* dyb)
{
    if (dyb->_position == 0) return dyb;
    uint move_size = dyb->_limit - dyb->_position;
    dyb_mem_move(dyb->_data, dyb->_data+dyb->_position, move_size);
    dyb->_limit = move_size;
    dyb->_position = 0;
    dyb->_mark = 0;
    return dyb;
}

dyb_inline uint dyb_get_remainder(dybuf* dyb)
{
    return dyb->_limit - dyb->_position;
}


/// ====== Position operation

/**
 * Mark the current position, and go back later.
 *
 * @return
 */
dyb_inline dybuf* dyb_mark(dybuf* dyb) {
    dyb->_mark = dyb->_position;

    return dyb;
}

/**
 * Go to mark position.
 *
 * @return
 */
dyb_inline dybuf* dyb_reset(dybuf* dyb) {
    dyb->_position = dyb->_mark;

    return dyb;
}

/**
 * A new sequence, all reset to default.
 * Clears this buffer. The position is set to zero, the limit is set to the capacity, and the mark is discarded.
 *
 * @return
 */
dyb_inline dybuf* dyb_clear(dybuf* dyb) {
    dyb->_position = 0;
    dyb->_mark = 0;
    dyb->_limit = dyb->_capacity;

    return dyb;
}

/**
 * Flips this buffer. The limit is set to the current position and then the position is set to zero.
 * If the mark is defined then it is discarded.
 * This is use for write and then read.
 *
 * @return
 */
dyb_inline dybuf* dyb_flip(dybuf* dyb) {
    dyb->_limit = dyb->_position;
    dyb->_position = 0;
    dyb->_mark = 0;

    return dyb;
}

/**
 * Repeat a sequence again. The position is set to zero and the mark is discarded.
 *
 * @return
 */
dyb_inline dybuf* dyb_rewind(dybuf* dyb)
{
    dyb->_position = 0;
    dyb->_mark = 0;

    return dyb;
}


dyb_inline boolean dyb_next_bool(dybuf* dyb)
{
    return dyb->_data[dyb->_position++]!=0?true:false;
}

dyb_inline boolean dyb_peek_bool(dybuf* dyb)
{
    return dyb->_data[dyb->_position]!=0?true:false;
}

dyb_inline uint8 dyb_next_u8(dybuf* dyb)
{
    return dyb->_data[dyb->_position++];
}

dyb_inline uint8 dyb_peek_u8(dybuf* dyb)
{
    return dyb->_data[dyb->_position];
}

dyb_inline uint16 dyb_next_u16(dybuf* dyb)
{
    uint16 v = ((dyb->_data[dyb->_position++]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position++]&0x00ff);

    return v;
}

dyb_inline uint16 dyb_peek_u16(dybuf* dyb)
{
    uint16 v = ((dyb->_data[dyb->_position+0]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position+1]&0x00ff);

    return v;
}

dyb_inline uint32 dyb_next_u24(dybuf* dyb)
{
    uint32 v = ((dyb->_data[dyb->_position++]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position++]&0x00ff);

    return v;
}

dyb_inline uint32 dyb_peek_u24(dybuf* dyb)
{
    uint32 v = ((dyb->_data[dyb->_position+0]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position+1]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position+2]&0x00ff);

    return v;
}

dyb_inline uint32 dyb_next_u32(dybuf* dyb)
{
    uint32 v = ((dyb->_data[dyb->_position++]&0x00ff)<<24);
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position++]&0x00ff);

    return v;
}

dyb_inline uint32 dyb_peek_u32(dybuf* dyb)
{
    uint32 v = ((dyb->_data[dyb->_position+0]&0x00ff)<<24);
    v |= ((dyb->_data[dyb->_position+1]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position+2]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position+3]&0x00ff);

    return v;
}

dyb_inline uint64 dyb_next_u40(dybuf* dyb)
{
    uint64 v = ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<32);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<24);       // be careful, type convert is necessary
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position++]&0x00ff);

    return v;
}

dyb_inline uint64 dyb_peek_u40(dybuf* dyb)
{
    uint64 v = ((uint64)(dyb->_data[dyb->_position+0]&0x00ff)<<32);
    v |= ((uint64)(dyb->_data[dyb->_position+1]&0x00ff)<<24);       // be careful, type convert is necessary
    v |= ((dyb->_data[dyb->_position+2]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position+3]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position+4]&0x00ff);

    return v;
}

dyb_inline uint64 dyb_next_u48(dybuf* dyb)
{
    uint64 v = ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<40);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<32);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<24);       // be careful, type convert is necessary
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position++]&0x00ff);

    return v;
}

dyb_inline uint64 dyb_peek_u48(dybuf* dyb)
{
    uint64 v = ((uint64)(dyb->_data[dyb->_position+0]&0x00ff)<<40);
    v |= ((uint64)(dyb->_data[dyb->_position+1]&0x00ff)<<32);
    v |= ((uint64)(dyb->_data[dyb->_position+2]&0x00ff)<<24);       // be careful, type convert is necessary
    v |= ((dyb->_data[dyb->_position+3]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position+4]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position+5]&0x00ff);

    return v;
}

dyb_inline uint64 dyb_next_u56(dybuf* dyb)
{
    uint64 v = ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<48);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<40);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<32);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<24);       // be careful, type convert is necessary
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position++]&0x00ff);

    return v;
}

dyb_inline uint64 dyb_peek_u56(dybuf* dyb)
{
    uint64 v = ((uint64)(dyb->_data[dyb->_position+0]&0x00ff)<<48);
    v |= ((uint64)(dyb->_data[dyb->_position+1]&0x00ff)<<40);
    v |= ((uint64)(dyb->_data[dyb->_position+2]&0x00ff)<<32);
    v |= ((uint64)(dyb->_data[dyb->_position+3]&0x00ff)<<24);       // be careful, type convert is necessary
    v |= ((dyb->_data[dyb->_position+4]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position+5]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position+6]&0x00ff);

    return v;
}


dyb_inline uint64 dyb_next_u64(dybuf* dyb)
{
    uint64 v = ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<56);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<48);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<40);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<32);
    v |= ((uint64)(dyb->_data[dyb->_position++]&0x00ff)<<24);       // be careful, type convert is necessary
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position++]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position++]&0x00ff);

    return v;
}

dyb_inline uint64 dyb_peek_u64(dybuf* dyb)
{
    uint64 v = ((uint64)(dyb->_data[dyb->_position+0]&0x00ff)<<56);
    v |= ((uint64)(dyb->_data[dyb->_position+1]&0x00ff)<<48);
    v |= ((uint64)(dyb->_data[dyb->_position+2]&0x00ff)<<40);
    v |= ((uint64)(dyb->_data[dyb->_position+3]&0x00ff)<<32);
    v |= ((uint64)(dyb->_data[dyb->_position+4]&0x00ff)<<24);       // be careful, type convert is necessary
    v |= ((dyb->_data[dyb->_position+5]&0x00ff)<<16);
    v |= ((dyb->_data[dyb->_position+6]&0x00ff)<<8);
    v |= (dyb->_data[dyb->_position+7]&0x00ff);

    return v;
}


dyb_inline uint8* dyb_next_data_with_1byte_len(dybuf* dyb, uint *len)
{
    *len = dyb->_data[dyb->_position++];

    if ((*len)==0 || (dyb->_position + (*len)) > dyb->_limit) {
        return null;
    }

    uint8* data = dyb->_data + dyb->_position;
    dyb->_position += *len;

    return data;
}

dyb_inline uint8* dyb_next_data_with_2bytes_len(dybuf* dyb, uint *len)
{
    *len = dyb->_data[dyb->_position++];
    *len <<= 8;
    *len |= dyb->_data[dyb->_position++];

    if ((*len)==0 || (dyb->_position + (*len)) > dyb->_limit) {
        return null;
    }

    uint8* data = dyb->_data + dyb->_position;
    dyb->_position += *len;

    return data;
}

dyb_inline uint8* dyb_next_data_without_len(dybuf* dyb, uint len)
{
    if (len==0 || (dyb->_position + len) > dyb->_limit) {
        return null;
    }

    uint8* data = dyb->_data + dyb->_position;
    dyb->_position += len;

    return data;
}

dyb_inline void* dyb_next_structure(dybuf* dyb, void* structure, uint size)
{
    if (size==0 || (dyb->_position + size) > dyb->_limit) {
        return null;
    }

    dyb_mem_copy(structure, dyb->_data + dyb->_position, size);
    dyb->_position += size;

    return structure;
}


// append

dyb_inline dybuf* dyb_append_bool(dybuf* dyb, boolean value)
{
    if (dyb->_position+1 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+1);
    }
    dyb->_data[dyb->_position++] = value?1:0;
    return dyb;
}

dyb_inline dybuf* dyb_append_u8(dybuf* dyb, uint8 value)
{
    if (dyb->_position+1 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+1);
    }
    dyb->_data[dyb->_position++] = value;
    return dyb;
}

dyb_inline dybuf* dyb_append_u16(dybuf* dyb, uint16 value)
{
    if (dyb->_position+2 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+2);
    }
    dyb->_data[dyb->_position++] = (value>>8)&0x00ff;
    dyb->_data[dyb->_position++] = value&0x00ff;
    return dyb;
}

dyb_inline dybuf* dyb_append_u24(dybuf* dyb, uint32 value)
{
    if (dyb->_position+3 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+3);
    }
    dyb->_data[dyb->_position++] = (value>>16)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>8)&0x00ff;
    dyb->_data[dyb->_position++] = value&0x00ff;
    return dyb;
}


dyb_inline dybuf* dyb_append_u32(dybuf* dyb, uint32 value)
{
    if (dyb->_position+4 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+4);
    }
    dyb->_data[dyb->_position++] = (value>>24)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>16)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>8)&0x00ff;
    dyb->_data[dyb->_position++] = value&0x00ff;
    return dyb;
}

dyb_inline dybuf* dyb_append_u40(dybuf* dyb, uint64 value)
{
    if (dyb->_position+5 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+5);
    }
    //dyb->_data[dyb->_position++] = (value>>56)&0x00ff;
    //dyb->_data[dyb->_position++] = (value>>48)&0x00ff;
    //dyb->_data[dyb->_position++] = (value>>40)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>32)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>24)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>16)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>8)&0x00ff;
    dyb->_data[dyb->_position++] = value;

    return dyb;
}

dyb_inline dybuf* dyb_append_u48(dybuf* dyb, uint64 value)
{
    if (dyb->_position+6 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+6);
    }
    //dyb->_data[dyb->_position++] = (value>>56)&0x00ff;
    //dyb->_data[dyb->_position++] = (value>>48)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>40)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>32)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>24)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>16)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>8)&0x00ff;
    dyb->_data[dyb->_position++] = value;

    return dyb;
}

dyb_inline dybuf* dyb_append_u56(dybuf* dyb, uint64 value)
{
    if (dyb->_position+7 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+7);
    }
    //dyb->_data[dyb->_position++] = (value>>56)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>48)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>40)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>32)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>24)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>16)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>8)&0x00ff;
    dyb->_data[dyb->_position++] = value;

    return dyb;
}


dyb_inline dybuf* dyb_append_u64(dybuf* dyb, uint64 value)
{
    if (dyb->_position+8 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+8);
    }
    dyb->_data[dyb->_position++] = (value>>56)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>48)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>40)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>32)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>24)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>16)&0x00ff;
    dyb->_data[dyb->_position++] = (value>>8)&0x00ff;
    dyb->_data[dyb->_position++] = value;

    return dyb;
}

dyb_inline dybuf* dyb_append_data_with_1byte_len(dybuf* dyb, uint8* data, uint length)
{
    length &= 0x00ff;

    if (dyb->_position+length+1 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+length+1);
    }

    dyb->_data[dyb->_position++] = length;

    if (length > 0) {
        dyb_mem_copy(dyb->_data+dyb->_position, data, length);
        dyb->_position += length;
    }

    return dyb;
}

dyb_inline dybuf* dyb_append_data_with_2bytes_len(dybuf* dyb, uint8* data, uint length)
{
    length &= 0x00ffff;

    if (dyb->_position+length+2 > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+length+2);
    }

    dyb->_data[dyb->_position++] = (uint8)(length>>8);
    dyb->_data[dyb->_position++] = (uint8)length;

    if (length > 0) {
        dyb_mem_copy(dyb->_data+dyb->_position, data, length);
        dyb->_position += length;
    }

    return dyb;
}

dyb_inline dybuf* dyb_append_data_without_len(dybuf* dyb, uint8* data, uint length)
{
    if (dyb->_position+length > dyb->_limit) {
        dyb_set_limit(dyb, dyb->_position+length);
    }

    if (length > 0) {
        dyb_mem_copy(dyb->_data+dyb->_position, data, length);
        dyb->_position += length;
    }

    return dyb;
}

dyb_inline dybuf* dyb_append_structure(dybuf* dyb, void* structure, uint size)
{
    return dyb_append_data_without_len(dyb, (uint8*)structure, size);
}

dyb_inline uint8* dyb_get_data_before_current_position(dybuf* dyb, uint* len)
{
    *len = dyb->_position;
    return dyb->_data;
}

/// ===== type, index manage

enum {
    typdex_typ_none     = 0,                // none, empty
    typdex_typ_bool     = 1,                // 1 byte boolean
    typdex_typ_int      = 2,                // variable size int64
    typdex_typ_uint     = 3,                // variable size uint64
#if !defined(DISABLE_FP)
    typdex_typ_float    = 6,                // 4 bytes float
    typdex_typ_double   = 7,                // 8 bytes double
#endif    
    typdex_typ_string   = 0xa,              // variable length string
    typdex_typ_bytes    = 0xb,              // variable length binary
    typdex_typ_array    = 0xc,              // array of items
    typdex_typ_map      = 0xd,              // items map
    typdex_typ_f        = 0xf,              // functions
};

dyb_inline dybuf* dyb_append_typdex(dybuf* dyb, uint8 type, uint index)
{
    if (type <= 0x0f && index <= 0x07) {                                                    // header:0x00(1bits), type:0~0x0F(4bits), index:0~0x7(3bits)
        dyb_append_u8(dyb, (type<<3) | index);
    } else if (type <= 0x3F && index <= 0x0FF) {                                            // header:0x02(2bits), type:0~0x3F(6bits), index:0~0x00FF(8bits)
        dyb_append_u16(dyb, 0x8000 | ((uint16)type<<8) | index);
    } else if (type <= 0xFF && index <= 0x1FFF) {                                           // header:0x06(3bits), type:0~0xFF(8bits), index:0~0x1FFF(13bits)
        dyb_append_u24(dyb, 0xC00000 | ((uint32)type<<13) | index);
    } else if (type <= 0xFF && index <= 0x0FFFFF) {                                         // header:0x0E(4bits), type:0~0xFF(8bits), index:0~0x0FFFFF(20bits)
        dyb_append_u32(dyb, 0xE0000000 | ((uint32)type << 20) | index);
    } else {
        return null;
    }
    return dyb;
}


dyb_inline void dyb_next_typdex(dybuf* dyb, uint8* type, uint* index)
{
    uint8 typ = dyb_peek_u8(dyb);
    uint idx = 0;
    
    if ((typ&0x80)==0) {
        uint8 v = dyb_next_u8(dyb);
        typ = (v >> 3) & 0x0F;
        idx = v & 0x07;
    } else if ((typ&0x40)==0) {
        uint16 v = dyb_next_u16(dyb);
        typ = (uint8)(v >> 8) & 0x3F;
        idx = v & 0x00FF;
    } else if ((typ&0x20)==0) {
        uint32 v = dyb_next_u24(dyb);
        typ = (uint8)(v >> 13) & 0xFF;
        idx = v & 0x1FFF;
    } else if ((typ&0x10)==0) {
        uint32 v = dyb_next_u32(dyb);
        typ = (uint8)(v >> 20) & 0xFF;
        idx = v & 0x0FFFFF;
    } else {
        // error
    }

    if (type) *type = typ;
    if (index) *index = idx;
}

dyb_inline void dyb_peek_typdex(dybuf* dyb, uint8* type, uint* index)
{
    uint8 typ = dyb_peek_u8(dyb);
    uint idx = 0;

    if ((typ&0x80)==0) {
        uint8 v = dyb_peek_u8(dyb);
        typ = (v >> 3) & 0x0F;
        idx = v & 0x07;
    } else if ((typ&0x40)==0) {
        uint16 v = dyb_peek_u16(dyb);
        typ = (uint8)(v >> 8) & 0x3F;
        idx = v & 0x00FF;
    } else if ((typ&0x20)==0) {
        uint32 v = dyb_peek_u24(dyb);
        typ = (uint8)(v >> 13) & 0xFF;
        idx = v & 0x1FFF;
    } else if ((typ&0x10)==0) {
        uint32 v = dyb_peek_u32(dyb);
        typ = (uint8)(v >> 20) & 0xFF;
        idx = v & 0x0FFFFF;
    } else {
        // error
    }
    if (type) *type = typ;
    if (index) *index = idx;
}

/// var u64
dyb_inline dybuf* dyb_append_var_u64(dybuf* dyb, uint64 value)
{
    if (value<=0x7F) {
        dyb_append_u8(dyb, value);// 0 ~ 0x7F
    } else if (value <= (0x3FFF+0x80)) {
        // (0x7F+1) ~ 0x3FFF+(0x7F+1)
        // 0x80 ~ 0x3FFF+0x80
        dyb_append_u16(dyb, 0x8000 | (value-0x80));
    } else if (value <= (0x1FFFFFUL+0x4080UL)) {
        // (0x3FFF+(0x7F+1)+1) ~ 0x1FFFFF+(0x3FFF+(0x7F+1)+1)
        // 0x4080 ~ 0x1FFFFF + 0x4080
        dyb_append_u24(dyb, (uint32)(0xC00000UL | (value-0x4080UL)));
    } else if (value <= (0x0FFFFFFFUL+0x204080UL)) {
        // (0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1) ~ 0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)
        // 0x204080 ~ 0x0FFFFFFF + 0x204080
        dyb_append_u32(dyb, (uint32)(0xE0000000UL | (value-0x204080UL)));
    } else if (value <= (0x07FFFFFFFFUL+0x10204080UL)) {
        // (0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)+1) ~ 0x07FFFFFFFF+(0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)+1)
        // 0x10204080 ~ 0x07FFFFFFFF+0x10204080
        dyb_append_u40(dyb, 0xF000000000UL | (value-0x10204080UL));
    } else if (value <= (0x03FFFFFFFFFFUL+0x0810204080UL)) {
        //  (0x07FFFFFFFF+(0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)+1)) ~ 0x03FFFFFFFFFF+(0x07FFFFFFFF+(0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)+1))
        // 0x0810204080 ~ 0x03FFFFFFFFFF+0x0810204080
        dyb_append_u48(dyb, 0xF80000000000UL | (value-0x0810204080UL));
    } else if (value <= (0x01FFFFFFFFFFFFL+0x040810204080UL)) {
        // (0x03FFFFFFFFFF+(0x07FFFFFFFF+(0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)+1))+1) ~ 0x01FFFFFFFFFFFF+(0x03FFFFFFFFFF+(0x07FFFFFFFF+(0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)+1))+1)
        // 0x040810204080 ~ 0x01FFFFFFFFFFFF+0x040810204080
        dyb_append_u56(dyb, 0xFC000000000000UL | (value-0x040810204080UL));
    } else if (value <= (0x00FFFFFFFFFFFFFFUL+0x02040810204080UL)) {
        // (0x01FFFFFFFFFFFF+(0x03FFFFFFFFFF+(0x07FFFFFFFF+(0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)+1))+1)) ~ 0x00FFFFFFFFFFFFFF+((0x01FFFFFFFFFFFF+(0x03FFFFFFFFFF+(0x07FFFFFFFF+(0x0FFFFFFF+(0x1FFFFF+(0x3FFF+(0x7F+1)+1)+1)+1))+1)))
        // 0x02040810204080 ~ 0x00FFFFFFFFFFFFFF+0x02040810204080
        dyb_append_u64(dyb, 0xFE00000000000000UL | (value-0x02040810204080UL));
    } else {
        // 0x0102040810204080 ~ 0xFFFFFFFFFFFFFFFF
        dyb_append_u8(dyb, 0xFF);
        dyb_append_u64(dyb, value-0x0102040810204080UL);
    }
    return dyb;
}

dyb_inline uint64 dyb_next_var_u64(dybuf* dyb)
{
    uint8 b = dyb_next_u8(dyb);
    if ((b&0x80)==0) {
        return (b&0x7F);
    } else if ((b&0x40)==0) {
        return (((b&0x3F)<<8) | (dyb_next_u8(dyb) & 0x00FFUL))+0x80UL;
    } else if ((b&0x20)==0) {
        return (((b&0x1F)<<16) | (dyb_next_u16(dyb) & 0x00FFFFUL))+0x4080UL;
    } else if ((b&0x10)==0) {
        return (((b&0x0F)<<24) | (dyb_next_u24(dyb) & 0x00FFFFFFUL))+0x204080UL;
    } else if ((b&0x08)==0) {
        return (((uint64)(b&0x07)<<32) | (dyb_next_u32(dyb) & 0x00FFFFFFFFUL))+0x10204080UL;
    } else if ((b&0x04)==0) {
        return (((uint64)(b&0x03)<<40) | (dyb_next_u40(dyb) & 0x00FFFFFFFFFFUL))+0x0810204080UL;
    } else if ((b&0x02)==0) {
        return (((uint64)(b&0x01)<<48) | (dyb_next_u48(dyb) & 0x00FFFFFFFFFFFFUL))+0x040810204080UL;
    } else if ((b&0x01)==0) {
        return (dyb_next_u56(dyb) & 0x00FFFFFFFFFFFFFFUL)+0x02040810204080UL;
    } else {
        return dyb_next_u64(dyb)+0x0102040810204080UL;
    }
}


dyb_inline dybuf* dyb_append_var_s64(dybuf* dyb, int64 value)
{
    dyb_append_var_u64(dyb, ((value << 1) ^ (value >> 63)));
    return dyb;
}

dyb_inline int64 dyb_next_var_s64(dybuf* dyb)
{
    uint64 u = dyb_next_var_u64(dyb);
    return ((u&0x01)==0)?((int64)((u>>1)&0x7FFFFFFFFFFFFFFFUL)):((int64)(((u>>1)&0x7FFFFFFFFFFFFFFFUL) ^ 0xFFFFFFFFFFFFFFFFL));
}

#if !defined(DISABLE_FP)
dyb_inline dybuf* dyb_append_float(dybuf* dyb, float value)
{
    dyb_append_u32(dyb, dyb_swap_u32(*(uint32*)&value));
    return dyb;
}

dyb_inline float dyb_next_float(dybuf* dyb)
{
    uint32 v = dyb_swap_u32(dyb_next_u32(dyb));
    return *(float*)&v;
}

dyb_inline dybuf* dyb_append_double(dybuf* dyb, double value)
{
    dyb_append_u64(dyb, dyb_swap_u64(*(uint64*)&value));
    return dyb;
}

dyb_inline double dyb_next_double(dybuf* dyb)
{
    uint64 v = dyb_swap_u64(dyb_next_u64(dyb));
    return *(double*)&v;
}
#endif

dyb_inline dybuf* dyb_append_data_with_var_len(dybuf* dyb, uint8* data, uint size)
{
    dyb_append_var_u64(dyb, size);
    dyb_append_data_without_len(dyb, data, size);

    return dyb;
}

dyb_inline uint8* dyb_next_data_with_var_len(dybuf* dyb, uint* size)
{
    uint len = (uint)dyb_next_var_u64(dyb);
    if (size) *size = len;
    return dyb_next_data_without_len(dyb, len);
}


dyb_inline dybuf* dyb_append_cstring_with_var_len(dybuf* dyb, const char* string)
{
    uint size = plat_cstr_length(string);
    dyb_append_var_u64(dyb, size+1);
    dyb_append_data_without_len(dyb, (uint8*)string, size+1);

    return dyb;
}

dyb_inline char* dyb_next_cstring_with_var_len(dybuf* dyb, uint* size)
{
    uint len = (uint)dyb_next_var_u64(dyb);
    if (size) *size = (len-1);
    return (char*)dyb_next_data_without_len(dyb, len);
}


#endif //DYBUF_C_DYBUF_H



