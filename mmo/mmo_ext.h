/*
 * LC-clibs, lets.cool c libraries
 * Copyright (C) 2015-2018 Yuchi Chen (yuchi518@gmail.com)

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
// Created by Yuchi Chen on 2017/9/2.
//

#ifndef PROC_LA_MMOBJ_LIB_H
#define PROC_LA_MMOBJ_LIB_H

#include "mmo.h"
#include "uthash.h"
#include "utarray.h"
#include "plat_string.h"

/// ===== Object =====
typedef struct MMObject {

}* MMObject;

/*plat_inline MMObject initMMObject(MMObject obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMObject(MMObject obj) {

}*/

MMRootObject(MMObject, null, null, null);

/// ====== Primary type =====
typedef struct MMPrimary {
}*MMPrimary;

/*plat_inline MMPrimary initMMPrimary(MMPrimary primary, Unpacker unpkr) {
    return primary;
}

plat_inline void destroyMMPrimary(MMPrimary primary) {
}*/

MMSubObject(MMPrimary, MMObject, null, null, null);

/// ====== Primary type - String =====
typedef struct MMString {
    char* value;
}*MMString;

plat_inline void hash_of_MMString(void* stru, void** key, uint* key_len);
plat_inline int compareForMMString(void*, void*);
plat_inline MMString stringizeMMString(void*);
plat_inline MMString initMMString(MMString obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMString);
    set_compare_for_mmobj(obj, compareForMMString);
    set_stringize_for_mmobj(obj, stringizeMMString);
    if (is_unpacker_v1(unpkr)) {
        uint len;
        uint8* data;
        data = unpack_data(0, &len, unpkr);
        if (data && len) {
            mgn_memory_pool* pool = pool_of_mmobj(obj);
            obj->value = mgn_mem_alloc(pool, len);
            if (obj->value == null) return null;
            plat_mem_copy(obj->value, data, len);
        }
    }
    return obj;
}

plat_inline void destroyMMString(MMString obj) {
    if (obj->value) {
        mgn_memory_pool* pool = pool_of_mmobj(obj);
        mgn_mem_release(pool, obj->value);
        obj->value = null;
    }
}

plat_inline void packMMString(MMString obj, Packer pkr) {
    if (is_packer_v1(pkr))
        pack_data(0, obj->value, plat_cstr_length(obj->value)+1, pkr);
}

MMSubObject(MMString, MMPrimary, initMMString, destroyMMString, packMMString);

plat_inline void hash_of_MMString(void* stru, void** key, uint* key_len)
{
    MMString string = toMMString(stru);
    char* c = string->value;
    if (key) *key = c;
    if (key_len) *key_len = plat_cstr_length(c);
}

plat_inline int compareForMMString(void* this_stru, void* that_stru)
{
    const char* this_c = toMMString(this_stru)->value;
    const char* that_c = toMMString(that_stru)->value;
    if (this_c == that_c) return 0; // null
    if (this_c == null) return -1;
    if (that_c == null) return 1;
    /*while((*this_c) == (*that_c) && (*this_c)!='\0' && (*that_c)!='\0') {
        this_c++;
        that_c++;
    }
    return (*this_c) - (*that_c);*/
    return plat_cstr_compare(this_c, that_c);
}

plat_inline MMString stringizeMMString(void* stru) {
    MMString obj = toMMString(stru);
    return obj;
}

plat_inline MMString allocMMStringWithCString(mgn_memory_pool* pool, const char* string) {
    uint len = plat_cstr_length(string);
    char* new_string = mgn_mem_alloc(pool, len+1);
    if (new_string == null) return null;
    plat_mem_copy(new_string, string, len+1);
    MMString obj = allocMMString(pool);
    if (obj == null) {
        mgn_mem_release(pool, new_string);
        return null;
    }
    obj->value = new_string;

    return obj;
}

plat_inline MMString allocMMStringWithFormat(mgn_memory_pool* pool, const char* format, ...)
{
    va_list arg, arg_count;
    int len;
    va_start(arg, format);
    va_copy(arg_count, arg);

    len = vsnprintf(null, 0, format, arg_count);
    va_end(arg_count);

    if (len < 0) {
        va_end(arg);
        return null;
    }

    char* new_string = mgn_mem_alloc(pool, (size_t )len+1);
    if (new_string == null) {
        va_end(arg);
        return null;
    }

    if (len != vsnprintf(new_string, len+1, format, arg)) {
        plat_io_printf_err("What problem? It is impossible.\n");
    }
    va_end(arg);

    MMString obj = allocMMString(pool);
    if (obj == null) {
        mgn_mem_release(pool, new_string);
        return null;
    }

    obj->value = new_string;

    return obj;
}

#define mmstring(pool, format, ...)   autorelease_mmobj(allocMMStringWithFormat(pool, format, ##__VA_ARGS__))

/// ====== Primary type - Int (32bits) =====
typedef struct MMInt {
    int32 value;
}*MMInt;

plat_inline void hash_of_MMInt(void*, void** key, uint* key_len);
plat_inline int compareForMMInt(void*, void*);
plat_inline MMString stringizeMMInt(void*);
plat_inline MMInt initMMInt(MMInt obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMInt);
    set_compare_for_mmobj(obj, compareForMMInt);
    set_stringize_for_mmobj(obj, stringizeMMInt);
    if (is_unpacker_v1(unpkr)) {
        obj->value = (int32)unpack_varint(0, unpkr);
    }
    return obj;
}

/*plat_inline void destroyMMInt(MMInt obj) {

}*/

plat_inline void packMMInt(MMInt obj, Packer pkr) {
    if (is_packer_v1(pkr))
        pack_varint(0, obj->value, pkr);
}

MMSubObject(MMInt, MMPrimary, initMMInt, null, packMMInt);

plat_inline void hash_of_MMInt(void* stru, void** key, uint* key_len)
{
    MMInt obj = toMMInt(stru);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}

plat_inline int compareForMMInt(void* this_stru, void* that_stru)
{
    MMInt this_o = toMMInt(this_stru);
    MMInt that_o = toMMInt(that_stru);
    return this_o->value - that_o->value;
}

plat_inline MMString stringizeMMInt(void* stru) {
    MMInt obj = toMMInt(stru);
    return mmstring(pool_of_mmobj(obj), "%d", obj->value);
}

plat_inline MMInt allocMMIntWithValue(mgn_memory_pool* pool, int32 value) {
    MMInt obj = allocMMInt(pool);
    if (obj) {
        obj->value = value;
    }
    return obj;
}

/// ====== Primary type - Long (64bits) =====
typedef struct MMLong {
    int64 value;
}*MMLong;

plat_inline void hash_of_MMLong(void* stru, void** key, uint* key_len);
plat_inline int compareForMMLong(void*, void*);
plat_inline MMString stringizeMMLong(void*);
plat_inline MMLong initMMLong(MMLong obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMLong);
    set_compare_for_mmobj(obj, compareForMMLong);
    set_stringize_for_mmobj(obj, stringizeMMLong);
    if (is_unpacker_v1(unpkr)) {
        obj->value = unpack_varint(0, unpkr);
    }
    return obj;
}

/*plat_inline void destroyMMLong(MMLong obj) {
}*/

plat_inline void packMMLong(MMLong obj, Packer pkr) {
    if (is_packer_v1(pkr))
        pack_varint(0, obj->value, pkr);
}

MMSubObject(MMLong, MMPrimary, initMMLong, null, packMMLong);

plat_inline void hash_of_MMLong(void* stru, void** key, uint* key_len)
{
    MMLong obj = toMMLong(stru);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}

plat_inline int compareForMMLong(void* this_stru, void* that_stru)
{
    MMLong this_o = toMMLong(this_stru);
    MMLong that_o = toMMLong(that_stru);
    int64 diff = this_o->value - that_o->value;
    return diff>0?1:diff<0?-1:0;
}

plat_inline MMString stringizeMMLong(void* stru) {
    MMLong obj = toMMLong(stru);
    return mmstring(pool_of_mmobj(obj), "%ld", obj->value);
}

plat_inline MMLong allocMMLongWithValue(mgn_memory_pool* pool, int64 value) {
    MMLong obj = allocMMLong(pool);
    if (obj) {
        obj->value = value;
    }
    return obj;
}

/// ====== Primary type - Float (32bits) =====
typedef struct MMFloat {
    float value;
}*MMFloat;

plat_inline void hash_of_MMFloat(void* stru, void** key, uint* key_len);
plat_inline int compareForMMFloat(void*, void*);
plat_inline MMString stringizeMMFloat(void*);
plat_inline MMFloat initMMFloat(MMFloat obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMFloat);
    set_compare_for_mmobj(obj, compareForMMFloat);
    set_stringize_for_mmobj(obj, stringizeMMFloat);
    if (is_unpacker_v1(unpkr)) {
        obj->value = unpack_float(0, unpkr);
    }
    return obj;
}

/*plat_inline void destroyMMFloat(MMFloat obj) {
}*/

plat_inline void packMMFloat(MMFloat obj, Packer pkr) {
    if (is_packer_v1(pkr))
        pack_float(0, obj->value, pkr);
}

MMSubObject(MMFloat, MMPrimary, initMMFloat, null, packMMFloat);

plat_inline void hash_of_MMFloat(void* stru, void** key, uint* key_len)
{
    MMFloat obj = toMMFloat(stru);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}

plat_inline int compareForMMFloat(void* this_stru, void* that_stru)
{
    MMFloat this_o = toMMFloat(this_stru);
    MMFloat that_o = toMMFloat(that_stru);
    float diff = this_o->value - that_o->value;
    return diff>0?1:diff<0?-1:0;
}

plat_inline MMString stringizeMMFloat(void* stru) {
    MMFloat obj = toMMFloat(stru);
    return mmstring(pool_of_mmobj(obj), "%f", obj->value);
}

plat_inline MMFloat allocMMFloatWithValue(mgn_memory_pool* pool, float value) {
    MMFloat obj = allocMMFloat(pool);
    if (obj) {
        obj->value = value;
    }
    return obj;
}


/// ====== Primary type - Double (64bits) =====
typedef struct MMDouble {
    double value;
}*MMDouble;

plat_inline void hash_of_MMDouble(void* stru, void** key, uint* key_len);
plat_inline int compareForMMDouble(void*, void*);
plat_inline MMString stringizeMMDouble(void*);
plat_inline MMDouble initMMDouble(MMDouble obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMDouble);
    set_compare_for_mmobj(obj, compareForMMDouble);
    set_stringize_for_mmobj(obj, stringizeMMDouble);
    if (is_unpacker_v1(unpkr)) {
        obj->value = unpack_double(0, unpkr);
    }
    return obj;
}

/*plat_inline void destroyMMDouble(MMDouble obj) {
}*/

plat_inline void packMMDouble(MMDouble obj, Packer pkr) {
    if (is_packer_v1(pkr))
        pack_double(0, obj->value, pkr);
}

MMSubObject(MMDouble, MMPrimary, initMMDouble, null, packMMDouble);

plat_inline void hash_of_MMDouble(void* stru, void** key, uint* key_len)
{
    MMDouble obj = toMMDouble(stru);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}

plat_inline int compareForMMDouble(void* this_stru, void* that_stru)
{
    MMDouble this_o = toMMDouble(this_stru);
    MMDouble that_o = toMMDouble(that_stru);
    double diff = this_o->value - that_o->value;
    return diff>0?1:diff<0?-1:0;
}

plat_inline MMString stringizeMMDouble(void* stru) {
    MMDouble obj = toMMDouble(stru);
    return mmstring(pool_of_mmobj(obj), "%f", obj->value);
}

plat_inline MMDouble allocMMDoubleWithValue(mgn_memory_pool* pool, double value) {
    MMDouble obj = allocMMDouble(pool);
    if (obj) {
        obj->value = value;
    }
    return obj;
}

/// ===== Reference =====

typedef struct MMReference {
    void* reference;
    bool strong_reference;
}*MMReference;

plat_inline void hash_of_MMReference(void* stru, void** key, uint* key_len);

plat_inline MMReference initMMReference(MMReference obj, Unpacker unpkr) {
    (void)unpkr;
    set_hash_for_mmobj(obj, hash_of_MMReference);
    return obj;
}

plat_inline void destroyMMReference(MMReference obj) {
    if (obj->strong_reference) {
        release_mmobj(obj->reference);
    }
}

plat_inline void packMMReference(MMReference obj, Packer pkr) {
    (void)obj;
    (void)pkr;
}

MMSubObject(MMReference, MMPrimary, initMMReference, destroyMMReference, packMMReference);

plat_inline void hash_of_MMReference(void* stru, void** key, uint* key_len)
{
    MMReference obj = toMMReference(stru);
    if (key) *key = &obj->reference;
    if (key_len) *key_len = sizeof(obj->reference);
}

plat_inline MMReference allocMMReferenceWithReference(mgn_memory_pool* pool, void* reference) {
    MMReference obj = allocMMReference(pool);
    if (obj) {
        obj->reference = reference;
        obj->strong_reference = false;
    }
    return obj;
}

plat_inline MMReference allocMMReferenceWithStrongReference(mgn_memory_pool* pool, MMObject object) {
    MMReference obj = allocMMReference(pool);
    if (obj) {
        obj->reference = retain_mmobj(object);
        obj->strong_reference = true;
    }
    return obj;
}

/// ===== MMData =====

typedef struct MMData {
    void* data;
    uint size;
}*MMData;

plat_inline int compareForMMData(void*, void*);
plat_inline MMData initMMData(MMData obj, Unpacker unpkr) {
    set_compare_for_mmobj(obj, compareForMMData);
    if (is_unpacker_v1(unpkr)) {
        uint len;
        uint8* data;
        data = unpack_data(0, &len, unpkr);
        if (data && len) {
            mgn_memory_pool* pool = pool_of_mmobj(obj);
            obj->data = mgn_mem_alloc(pool, len);
            if (obj->data == null) return null;
            plat_mem_copy(obj->data, data, len);
            obj->size = len;
        }
    }
    return obj;
}

plat_inline void destroyMMData(MMData obj) {
    if (obj->data) {
        mgn_memory_pool* pool = pool_of_mmobj(obj);
        mgn_mem_release(pool, obj->data);
        obj->data = null;
    }
}

plat_inline void packMMData(MMData obj, Packer pkr) {
    if (is_packer_v1(pkr))
        pack_data(0, obj->data, obj->size, pkr);
}

MMSubObject(MMData, MMObject , initMMData, destroyMMData, packMMData);

plat_inline int compareForMMData(void* this_stru, void* that_stru)
{
    MMData this_data = toMMData(this_stru);
    MMData that_data = toMMData(that_stru);
    void* this_d = this_data->data;
    void* that_d = that_data->data;
    uint this_s = this_data->size;
    uint that_s = that_data->size;
    if (this_d == that_d) return 0; // null
    if (this_d == null) return -1;
    if (that_d == null) return 1;
    // if data is a c string, the result should be the same.
    uint size = this_s < that_s ? this_s : that_s;
    int diff = plat_mem_compare(this_d, that_d, size);
    return (diff != 0) ? diff : (int)(this_s - that_s);
}

plat_inline MMData allocMMDataWithData(mgn_memory_pool* pool, void* data, uint size)
{
    MMData obj = allocMMData(pool);
    if (obj) {
        obj->size = size;
        if (size > 0) {
            obj->data = mgn_mem_alloc(pool, size);
            if (obj->data == null) {
                release_mmobj(obj);
                return null;
            }
            if (data) plat_mem_copy(obj->data, data, size);
        } else {
            obj->data = null;
        }
    }
    return obj;
}

// NSData doesn't copy the memory, but own the memory.
plat_inline MMData allocMMDataWithDataNoCopy(mgn_memory_pool* pool, void* data, uint size)
{
    if (!((data!=null && size>0) || (data==null && size==0))) return null;
    MMData obj = allocMMData(pool);
    if (obj) {
        obj->size = size;
        obj->data = data;
    }
    return obj;
}


/// ====== Container =====
typedef struct MMContainer {

}*MMContainer;

/*plat_inline MMContainer initMMContainer(MMContainer obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMContainer(MMContainer obj) {
}*/

MMSubObject(MMContainer, MMObject, null, null, null);

/// ====== MapItem =====
typedef struct MMMapItem {
    MMPrimary key;
    MMObject value;
    UT_hash_handle hh;
}*MMMapItem;

plat_inline int compareForMMMapItem(void*, void*);
plat_inline MMMapItem initMMMapItem(MMMapItem item, Unpacker unpkr) {
    //item->key = null;
    //item->value = null;
    set_compare_for_mmobj(item, compareForMMMapItem);
    return item;
}

plat_inline void destroyMMMapItem(MMMapItem item) {
    if (item->key) release_mmobj(item->key);
    if (item->value) release_mmobj(item->value);
}

MMSubObject(MMMapItem, MMObject, initMMMapItem, destroyMMMapItem, null);

plat_inline MMMapItem allocMMMapItemWithKeyValue(mgn_memory_pool* pool, MMPrimary key, MMObject value) {
    if (key == null || value == null) return null;
    MMMapItem item = allocMMMapItem(pool);
    if (item == null) return null;
    item->key = retain_mmobj(key);
    item->value = retain_mmobj(value);
    return item;
}

plat_inline MMMapItem replaceMMMapItemKeyValue(MMMapItem item, MMPrimary key, MMObject value) {
    retain_mmobj(key);
    retain_mmobj(value);
    release_mmobj(item->key);
    release_mmobj(item->value);
    item->key = key;
    item->value = value;
    return item;
}

plat_inline int compareForMMMapItem(void* this_stru, void* that_stru) {
    MMMapItem this_mi = toMMMapItem(this_stru);
    MMMapItem that_mi = toMMMapItem(that_stru);
    int diff = compare_mmobjs(this_mi->key, that_mi->key);
    if (diff != 0) return diff;
    return compare_mmobjs(this_mi->value, that_mi->value);
}

/// ====== Map =====
/**
 * Only support primary type (Subclass of MMPrimary) as key.
 * Should not change value of key after key was added into the map.
 * And oly support one primary type as key in a map.
 */
typedef struct MMMap {
    bool sorted;
    MMMapItem rootItem;
}*MMMap;

plat_inline void addMMMapItem(MMMap map, MMPrimary key, MMObject value);
plat_inline int compareForMMMap(void*, void*);
plat_inline MMMap initMMMap(MMMap map, Unpacker unpkr) {
    map->rootItem = null;
    set_compare_for_mmobj(map, compareForMMMap);
    if (is_unpacker_v1(unpkr)) {
        map->sorted = unpack_varint(0, unpkr)?true:false;
        uint len = unpack_array(1, unpkr);
        uint i;
        if ((len&0x01)) return null;
        for (i=0; i<len; ) {
            MMPrimary primary = toMMPrimary(unpack_mmobj(i++, unpkr));
            MMObject value = toMMObject(unpack_mmobj(i++, unpkr));
            if (primary==null || value==null) return null;
            addMMMapItem(map, primary, value);
        }
        unpack_array_end(1, unpkr);
    }
    return map;
}

plat_inline void destroyMMMap(MMMap map) {
    MMMapItem item;
    MMMapItem tmp;

    HASH_ITER(hh, map->rootItem, item, tmp)
    {
        HASH_DEL(map->rootItem, item);
        release_mmobj(item);
    }
}

plat_inline void packMMMap(MMMap map, Packer pkr) {
    if (is_packer_v1(pkr)) {
        pack_varint(0, map->sorted?1:0, pkr);
        uint len = (uint)HASH_COUNT(map->rootItem);
        pack_array(1, len*2, pkr);
        MMMapItem item, tmp;
        HASH_ITER(hh, map->rootItem, item, tmp) {
            pack_mmobj(0, item->key, pkr);
            pack_mmobj(1, item->value, pkr);
        }
        pack_array_end(1, pkr);
    }
}

MMSubObject(MMMap, MMContainer, initMMMap, destroyMMMap, packMMMap);

plat_inline MMMap allocMMMapSorted(mgn_memory_pool* pool) {
    MMMap map = allocMMMap(pool);
    if (map) {
        map->sorted = true;
    }
    return map;
}

plat_inline uint sizeOfMMMap(MMMap map) {
    return (uint)HASH_COUNT(map->rootItem);
}

plat_inline void addMMMapItem(MMMap map, MMPrimary key, MMObject value)
{
    if (key == null || value==null) return;
    mgn_memory_pool* pool = pool_of_mmobj(map);
    MMMapItem item;
    void* hash_key = null;
    uint hash_key_len = 0;
    hash_of_mmobj(key, &hash_key, &hash_key_len);

    HASH_FIND(hh, map->rootItem, hash_key, (size_t)hash_key_len, item);
    if (item != null)
    {
        HASH_DEL(map->rootItem, item);
        replaceMMMapItemKeyValue(item, key, value);
    }
    else
    {
        item = allocMMMapItemWithKeyValue(pool, key, value);
    }
    if (map->sorted) {
        // Use ordered insertion for consistent comparison result.
        HASH_ADD_KEYPTR_INORDER(hh, map->rootItem, hash_key, hash_key_len, item, compare_mmobjs);
    } else {
        // Comparison result depends on insert insertion order
        HASH_ADD_KEYPTR(hh, map->rootItem, hash_key, hash_key_len, item);
    }
}

plat_inline MMObject getMMMapItemValue(MMMap map, MMPrimary key) {
    MMMapItem item;
    if (key==null) return null;
    void* hash_key = null;
    uint hash_key_len = 0;
    hash_of_mmobj(key, &hash_key, &hash_key_len);

    HASH_FIND(hh, map->rootItem, hash_key, (size_t)hash_key_len, item);
    if (item == null) {
        return null;
    }
    return item->value;
}

plat_inline void removeMMMapItem(MMMap map, MMPrimary key) {
    MMMapItem item;
    if (key==null) return;
    void* hash_key = null;
    uint hash_key_len = 0;
    hash_of_mmobj(key, &hash_key, &hash_key_len);

    HASH_FIND(hh, map->rootItem, hash_key, (size_t)hash_key_len, item);
    if (item == null) {
        return;
    }
    HASH_DEL(map->rootItem, item);
    release_mmobj(item);
}

plat_inline int compareForMMMap(void* this_stru, void* that_stru) {
    MMMap this_map = toMMMap(this_stru);
    MMMap that_map = toMMMap(that_stru);
    uint this_size = sizeOfMMMap(this_map);
    uint that_size = sizeOfMMMap(that_map);
    if (this_size != that_size) return this_size - that_size;

    // This comparison only works on ordered hash list.
    MMMapItem this_item = this_map->rootItem;
    MMMapItem that_item = that_map->rootItem;

    while (this_item != null && that_item != null) {
        int diff_key = compare_mmobjs(this_item->key, that_item->key);
        if (diff_key != 0) return diff_key;
        int diff_value = compare_mmobjs(this_item->value, that_item->value);
        if (diff_value != 0) return diff_value;
        this_item = this_item->hh.next;
        that_item = that_item->hh.next;
    }

    if (this_item != null) return 1;
    if (that_item != null) return -1;
    return 0;
}


/// ====== List =====
typedef struct MMList {
    UT_array list;
}*MMList;

/*void ut_ast_init(void *ptr)
{
    *(void**)ptr = null;
}*/

plat_inline void ut_ast_copy(void *dst, const void *src)
{
    MMObject* dst_obj = (MMObject*)dst;
    MMObject* src_obj = (MMObject*)src;
    *dst_obj = *src_obj;
    retain_mmobj(*dst_obj);
}

plat_inline void ut_ast_del(void *ptr)
{
    release_mmobj(*(MMObject*)ptr);
}

plat_inline int compareForMMList(void*, void*);
plat_inline MMList initMMList(MMList obj, Unpacker unpkr) {
    static UT_icd ut_ast__icd = { sizeof(void*), /*ut_ast_init*/null, ut_ast_copy, ut_ast_del };
    utarray_init(&obj->list, &ut_ast__icd);
    set_compare_for_mmobj(obj, compareForMMList);
    if (is_unpacker_v1(unpkr)) {
        uint len = unpack_array(0, unpkr);
        uint i;
        for (i=0; i<len;) {
            void* stru = unpack_mmobj(i++, unpkr);
            MMObject item = toMMObject(stru);
            if (item == null) {
                plat_io_printf_err("Why is null?(%p)(%s)\n", stru, name_of_last_mmobj(stru));
                return null;
            }
            utarray_push_back(&obj->list, &item);   // item will be retained by utarray
        }
        unpack_array_end(0, unpkr);
    }
    return obj;
}

plat_inline void destroyMMList(MMList obj) {
    utarray_done(&obj->list);
}

plat_inline void packMMList(MMList obj, Packer pkr) {
    if (is_packer_v1(pkr)) {
        uint len = utarray_len(&obj->list), i;
        pack_array(0, len, pkr);
        for (i=0; i<len; i++) {
            MMObject p = *(MMObject*)utarray_eltptr(&obj->list, i);
            pack_mmobj(0, p, pkr);
        }
        pack_array_end(0, pkr);
    }
}

MMSubObject(MMList, MMContainer, initMMList, destroyMMList, packMMList);

plat_inline uint sizeOfMMList(MMList list) {
    return utarray_len(&list->list);
}

plat_inline void pushMMListItem(MMList list, MMObject item) {
    utarray_push_back(&list->list, &item);
}

plat_inline MMObject popMMListItem(MMList list) {
    MMObject obj = *(MMObject*)utarray_back(&list->list);
    retain_mmobj(obj);
    utarray_pop_back(&list->list);
    return autorelease_mmobj(obj);
}

plat_inline MMObject getLastItemFromMMList(MMList list) {
    MMObject obj = *(MMObject*)utarray_back(&list->list);
    return obj;
}

plat_inline void insertMMListItem(MMList list, MMObject item, uint idx) {
    utarray_insert(&list->list, &item, idx);
}

plat_inline MMObject getMMListItem(MMList list, uint idx) {
    return *(MMObject*)utarray_eltptr(&list->list, idx);
}

plat_inline void concatMMList(MMList dest_list, MMList a_list) {
    utarray_concat(&dest_list->list, &a_list->list);
}

plat_inline int compareForMMList(void* this_stru, void* that_stru) {
    MMList this_list = toMMList(this_stru);
    MMList that_list = toMMList(that_stru);
    uint this_size = sizeOfMMList(this_list);
    uint that_size = sizeOfMMList(that_list);
    if (this_size != that_size) return this_size - that_size;
    uint i;
    for (i=0; i<this_size; i++) {
        MMObject this_obj = getMMListItem(this_list, i);
        MMObject that_obj = getMMListItem(that_list, i);
        int diff = compare_mmobjs(this_obj, that_obj);
        if (diff != 0) return diff;
    }
    return 0;
}


/// ===== register all allocators to unpacker
plat_inline void register_all_mmo_ext_to_unpacker(Unpacker unpkr) {
    registerMMIntToUnpacker(unpkr);
    registerMMLongToUnpacker(unpkr);
    registerMMDataToUnpacker(unpkr);
    registerMMStringToUnpacker(unpkr);
    registerMMFloatToUnpacker(unpkr);
    registerMMDoubleToUnpacker(unpkr);
    registerMMMapItemToUnpacker(unpkr);
    registerMMMapToUnpacker(unpkr);
    registerMMListToUnpacker(unpkr);
}


#endif //PROC_LA_MMOBJ_LIB_H



