//
// Created by Yuchi Chen on 2017/9/2.
//

#ifndef PROC_LA_MMOBJ_LIB_H
#define PROC_LA_MMOBJ_LIB_H

#include "mmo.h"
#include "uthash.h"
#include "utarray.h"
#include "plat_string.h"

enum {
    MMOBJ_OBJECT        = 0xFFFF0000,
    MMOBJ_PRIMARY,
    MMOBJ_INT,
    MMOBJ_LONG,
    MMOBJ_FLOAT,
    MMOBJ_DOUBLE,
    MMOBJ_STRING,

    MMOBJ_DATA          = 0xFFFF0081,

    MMOBJ_CONTAINER     = 0xFFFF0101,
    MMOBJ_MAP,
    MMOBJ_LIST,
    MMOBJ_MAPITEM,

    MMOBJ_PACKER       = 0xFFFF0201,
    MMOBJ_UNPACKER
};

/// ===== Object =====
typedef struct MMObject {

}* MMObject;

plat_inline MMObject initMMObject(MMObject obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMObject(MMObject obj) {

}

MMRootObject(MMOBJ_OBJECT, MMObject, initMMObject, destroyMMObject, null);

/// ====== Primary type =====
typedef struct MMPrimary {
}*MMPrimary;

plat_inline MMPrimary initMMPrimary(MMPrimary primary, Unpacker unpkr) {
    return primary;
}

plat_inline void destroyMMPrimary(MMPrimary primary) {

}

MMSubObject(MMOBJ_PRIMARY, MMPrimary, MMObject, initMMPrimary, destroyMMPrimary, null);

/// ====== Primary type - Int (32bits) =====
typedef struct MMInt {
    int32 value;
}*MMInt;

plat_inline void hash_of_MMInt(void*, void** key, uint* key_len);

plat_inline MMInt initMMInt(MMInt obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMInt);
    return obj;
}

plat_inline void destroyMMInt(MMInt obj) {
}

MMSubObject(MMOBJ_INT, MMInt, MMPrimary, initMMInt, destroyMMInt, null);

plat_inline void hash_of_MMInt(void* stru, void** key, uint* key_len)
{
    MMInt obj = toMMInt(stru);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
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

plat_inline MMLong initMMLong(MMLong obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMLong);
    return obj;
}

plat_inline void destroyMMLong(MMLong oint) {

}

MMSubObject(MMOBJ_LONG, MMLong, MMPrimary, initMMLong, destroyMMLong, null);

plat_inline void hash_of_MMLong(void* stru, void** key, uint* key_len)
{
    MMLong obj = toMMLong(stru);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
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

plat_inline MMFloat initMMFloat(MMFloat obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMFloat);
    return obj;
}

plat_inline void destroyMMFloat(MMFloat obj) {

}

MMSubObject(MMOBJ_FLOAT, MMFloat, MMPrimary, initMMFloat, destroyMMFloat, null);

plat_inline void hash_of_MMFloat(void* stru, void** key, uint* key_len)
{
    MMFloat obj = toMMFloat(stru);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
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

plat_inline MMDouble initMMDouble(MMDouble obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMDouble);
    return obj;
}

plat_inline void destroyMMDouble(MMDouble obj) {

}

MMSubObject(MMOBJ_DOUBLE, MMDouble, MMPrimary, initMMDouble, destroyMMDouble, null);

plat_inline void hash_of_MMDouble(void* stru, void** key, uint* key_len)
{
    MMDouble obj = toMMDouble(stru);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}

plat_inline MMDouble allocMMDoubleWithValue(mgn_memory_pool* pool, double value) {
    MMDouble obj = allocMMDouble(pool);
    if (obj) {
        obj->value = value;
    }
    return obj;
}

/// ====== Primary type - String =====
typedef struct MMString {
    char* value;
}*MMString;

plat_inline void hash_of_MMString(void* stru, void** key, uint* key_len);

plat_inline MMString initMMString(MMString obj, Unpacker unpkr) {
    set_hash_for_mmobj(obj, hash_of_MMString);
    return obj;
}

plat_inline void destroyMMString(MMString obj) {
    if (obj->value) {
        mgn_memory_pool* pool = pool_of_mmobj(obj);
        mgn_mem_release(pool, obj->value);
        obj->value = null;
    }
}

MMSubObject(MMOBJ_STRING, MMString, MMPrimary, initMMString, destroyMMString, null);

plat_inline void hash_of_MMString(void* stru, void** key, uint* key_len)
{
    MMString string = toMMString(stru);
    char* c = string->value;
    if (key) *key = c;
    if (key_len) *key_len = plat_cstr_length(c);
}

plat_inline MMString allocMMStringWithCString(mgn_memory_pool* pool, char* string) {
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


/// ===== MMData =====

typedef struct MMData {
    void* data;
    uint size;
}*MMData;

plat_inline MMData initMMData(MMData obj, Unpacker unpkr) {
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

}

MMSubObject(MMOBJ_DATA, MMData, MMObject , initMMData, destroyMMData, packMMData);

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

plat_inline MMContainer initMMContainer(MMContainer obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMContainer(MMContainer obj) {

}

MMSubObject(MMOBJ_CONTAINER, MMContainer, MMObject, initMMContainer, destroyMMContainer, null);

/// ====== MapItem =====
typedef struct MMMapItem {
    MMPrimary key;
    MMObject value;
    UT_hash_handle hh;
}*MMMapItem;

plat_inline MMMapItem initMMMapItem(MMMapItem item, Unpacker unpkr) {
    item->key = null;
    item->value = null;
    return item;
}

plat_inline void destroyMMMapItem(MMMapItem item) {
    if (item->key) release_mmobj(item->key);
    if (item->value) release_mmobj(item->value);
}

MMSubObject(MMOBJ_MAPITEM, MMMapItem, MMObject, initMMMapItem, destroyMMMapItem, null);

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

/// ====== Map =====
/**
 * Only support primary type (Subclass of MMPrimary) as key.
 * Should not change value of key after key was added into the map.
 * And oly support one primary type as key in a map.
 */
typedef struct MMMap {
    MMMapItem rootItem;
}*MMMap;

plat_inline MMMap initMMMap(MMMap map, Unpacker unpkr) {
    map->rootItem = null;
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
    HASH_ADD_KEYPTR(hh, map->rootItem, hash_key, hash_key_len, item);

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

MMSubObject(MMOBJ_MAP, MMMap, MMContainer, initMMMap, destroyMMMap, null);

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


plat_inline MMList initMMList(MMList obj, Unpacker unpkr) {
    static UT_icd ut_ast__icd = { sizeof(void*), /*ut_ast_init*/null, ut_ast_copy, ut_ast_del };
    utarray_init(&obj->list, &ut_ast__icd);
    return obj;
}

plat_inline void destroyMMList(MMList obj) {
    utarray_done(&obj->list);
}

MMSubObject(MMOBJ_LIST, MMList, MMContainer, initMMList, destroyMMList, null);

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

plat_inline void insertMMListItem(MMList list, MMObject item, uint idx) {
    utarray_insert(&list->list, &item, idx);
}

plat_inline MMObject getMMListItem(MMList list, uint idx) {
    return *(MMObject*)utarray_eltptr(&list->list, idx);
}

plat_inline void concatMMList(MMList dest_list, MMList a_list) {
    utarray_concat(&dest_list->list, &a_list->list);
}

/// ===== Packer =====
typedef struct MMPacker {

}*MMPacker;

plat_inline MMPacker initMMPacker(MMPacker obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMPacker(MMPacker obj) {

}

MMSubObject(MMOBJ_PACKER, MMPacker, MMObject , initMMPacker, destroyMMPacker, null);

/// ===== Unpacker =====
typedef struct MMUnpacker {

}*MMUnpacker;

plat_inline MMUnpacker initMMUnpacker(MMUnpacker obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMUnpacker(MMUnpacker obj) {

}

plat_inline void packMMUnpacker(MMUnpacker obj, Packer pkr) {

}

MMSubObject(MMOBJ_UNPACKER, MMUnpacker, MMObject , initMMUnpacker, destroyMMUnpacker, packMMUnpacker);


#endif //PROC_LA_MMOBJ_LIB_H



