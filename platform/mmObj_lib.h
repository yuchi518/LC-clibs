//
// Created by Yuchi Chen on 2017/9/2.
//

#ifndef PROC_LA_MMOBJ_LIB_H
#define PROC_LA_MMOBJ_LIB_H

#include "mmObj.h"
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

    MMOBJ_CONTAINER     = 0xFFFF0101,
    MMOBJ_MAP,
    MMOBJ_LIST,

    MMOBJ_MAPITEM       = 0xFFFF0201,
};

/// ===== Object =====
typedef struct MMObject {

}* MMObject;

static inline MMObject initMMObject(MMObject obj) {
    return obj;
}

static inline void destroyMMObject(MMObject obj) {

}

MMRootObject(MMOBJ_OBJECT, MMObject, initMMObject, destroyMMObject);

/// ====== Primary type =====
typedef struct MMPrimary {
}*MMPrimary;

static inline MMPrimary initMMPrimary(MMPrimary primary) {
    return primary;
}

static inline void destroyMMPrimary(MMPrimary primary) {

}

MMSubObject(MMOBJ_PRIMARY, MMPrimary, MMObject, initMMPrimary, destroyMMPrimary);

/// ====== Primary type - Int (32bits) =====
typedef struct MMInt {
    uint32 value;
}*MMInt;

static inline void hash_of_MMInt(mmBase base, void** key, uint* key_len);

static inline MMInt initMMInt(MMInt obj) {
    set_hash_for_mmobj(obj, hash_of_MMInt);
    return obj;
}

static inline void destroyMMInt(MMInt obj) {
}

MMSubObject(MMOBJ_INT, MMInt, MMPrimary, initMMInt, destroyMMInt);

static inline void hash_of_MMInt(mmBase base, void** key, uint* key_len)
{
    MMInt obj = baseToMMInt(base);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}


/// ====== Primary type - Long (64bits) =====
typedef struct MMLong {
    uint64 value;
}*MMLong;

static inline void hash_of_MMLong(mmBase base, void** key, uint* key_len);

static inline MMLong initMMLong(MMLong obj) {
    set_hash_for_mmobj(obj, hash_of_MMLong);
    return obj;
}

static inline void destroyMMLong(MMLong oint) {

}

MMSubObject(MMOBJ_LONG, MMLong, MMPrimary, initMMLong, destroyMMLong);

static inline void hash_of_MMLong(mmBase base, void** key, uint* key_len)
{
    MMLong obj = baseToMMLong(base);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}


/// ====== Primary type - Float (32bits) =====
typedef struct MMFloat {
    float value;
}*MMFloat;

static inline void hash_of_MMFloat(mmBase base, void** key, uint* key_len);

static inline MMFloat initMMFloat(MMFloat obj) {
    set_hash_for_mmobj(obj, hash_of_MMFloat);
    return obj;
}

static inline void destroyMMFloat(MMFloat obj) {

}

MMSubObject(MMOBJ_FLOAT, MMFloat, MMPrimary, initMMFloat, destroyMMFloat);

static inline void hash_of_MMFloat(mmBase base, void** key, uint* key_len)
{
    MMFloat obj = baseToMMFloat(base);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}


/// ====== Primary type - Double (64bits) =====
typedef struct MMDouble {
    double value;
}*MMDouble;

static inline void hash_of_MMDouble(mmBase base, void** key, uint* key_len);

static inline MMDouble initMMDouble(MMDouble obj) {
    set_hash_for_mmobj(obj, hash_of_MMDouble);
    return obj;
}

static inline void destroyMMDouble(MMDouble obj) {

}

MMSubObject(MMOBJ_DOUBLE, MMDouble, MMPrimary, initMMDouble, destroyMMDouble);

static inline void hash_of_MMDouble(mmBase base, void** key, uint* key_len)
{
    MMDouble obj = baseToMMDouble(base);
    if (key) *key = &obj->value;
    if (key_len) *key_len = sizeof(obj->value);
}

/// ====== Primary type - String =====
typedef struct MMString {
    char* value;
}*MMString;

static inline void hash_of_MMString(mmBase base, void** key, uint* key_len);

static inline MMString initMMString(MMString obj) {
    set_hash_for_mmobj(obj, hash_of_MMString);
    return obj;
}

static inline void destroyMMString(MMString obj) {
    if (obj->value) {
        mgn_memory_pool* pool = pool_of_mmobj(obj);
        mgn_mem_release(pool, obj->value);
        obj->value = null;
    }
}

MMSubObject(MMOBJ_STRING, MMString, MMPrimary, initMMString, destroyMMString);

static inline void hash_of_MMString(mmBase base, void** key, uint* key_len)
{
    MMString string = baseToMMString(base);
    char* c = string->value;
    if (key) *key = c;
    if (key_len) *key_len = plat_cstr_length(c);
}

static inline MMString allocMMStringWithCString(mgn_memory_pool* pool, char* string) {
    uint len = plat_cstr_length(string);
    char* new_string = mgn_mem_alloc(pool, len+1);
    plat_mem_copy(new_string, string, len+1);
    MMString obj = allocMMString(pool);
    if (obj == null) {
        mgn_mem_release(pool, new_string);
        return null;
    }
    obj->value = new_string;

    return obj;
}

/// ====== Container =====
typedef struct MMContainer {

}*MMContainer;

static inline MMContainer initMMContainer(MMContainer obj) {
    return obj;
}

static inline void destroyMMContainer(MMContainer obj) {

}

MMSubObject(MMOBJ_CONTAINER, MMContainer, MMObject, initMMContainer, destroyMMContainer);

/// ====== MapItem =====
typedef struct MMMapItem {
    MMPrimary key;
    MMObject value;
    UT_hash_handle hh;
}*MMMapItem;

static inline MMMapItem initMMMapItem(MMMapItem item) {
    item->key = null;
    item->value = null;
    return item;
}

static inline void destroyMMMapItem(MMMapItem item) {
    if (item->key) release_mmobj(item->key);
    if (item->value) release_mmobj(item->value);
}

MMSubObject(MMOBJ_MAPITEM, MMMapItem, MMObject, initMMMapItem, destroyMMMapItem);

static inline MMMapItem allocMMMapItemWithKeyValue(mgn_memory_pool* pool, MMPrimary key, MMObject value) {
    if (key == null || value == null) return null;
    MMMapItem item = allocMMMapItem(pool);
    if (item == null) return null;
    item->key = retain_mmobj(key);
    item->value = retain_mmobj(value);
    return item;
}

static inline MMMapItem replaceMMMapItemKeyValue(MMMapItem item, MMPrimary key, MMObject value) {
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

static inline MMMap initMMMap(MMMap map) {
    map->rootItem = null;
    return map;
}

static inline void destroyMMMap(MMMap map) {
    MMMapItem item;
    MMMapItem tmp;

    HASH_ITER(hh, map->rootItem, item, tmp)
    {
        HASH_DEL(map->rootItem, item);
        release_mmobj(item);
    }
}

static inline void addMMMapItem(MMMap map, MMPrimary key, MMObject value)
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

static inline MMObject getMMMapItemValue(MMMap map, MMPrimary key) {
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

static inline void removeMMMapItem(MMMap map, MMPrimary key) {
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

MMSubObject(MMOBJ_MAP, MMMap, MMContainer, initMMMap, destroyMMMap);

/// ====== List =====
typedef struct MMList {
    UT_array list;
}*MMList;

/*void ut_ast_init(void *ptr)
{
    *(void**)ptr = null;
}*/

static inline void ut_ast_copy(void *dst, const void *src)
{
    void** dst_obj = (void**)dst;
    void** src_obj = (void**)src;
    *dst_obj = *src_obj;
    retain_mmobj(*dst_obj);
}

static inline void ut_ast_del(void *ptr)
{
    release_mmobj(*(void**)ptr);
}


static inline MMList initMMList(MMList obj) {
    static UT_icd ut_ast__icd = { sizeof(void*), /*ut_ast_init*/null, ut_ast_copy, ut_ast_del };
    utarray_init(&obj->list, &ut_ast__icd);
    return obj;
}

static inline void destroyMMList(MMList obj) {
    utarray_done(&obj->list);
}

MMSubObject(MMOBJ_LIST, MMList, MMContainer, initMMList, destroyMMList);

static inline void pushMMListItem(MMList list, MMObject item) {
    utarray_push_back(&list->list, &item);
}

static inline MMObject popMMListItem(MMList list) {
    MMObject obj = (MMObject)utarray_back(&list->list);;
    retain_mmobj(obj);
    utarray_pop_back(&list->list);
    return autorelease_mmobj(obj);
}

#endif //PROC_LA_MMOBJ_LIB_H



