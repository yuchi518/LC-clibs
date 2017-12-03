//
// Created by Yuchi Chen on 2017/10/9.
//

#include "mmo_pack.h"
#define is_unpkr_dbg        (false)

/// ===== MMPacker =====

static uint packerVersion_impl(Packer packer) {
    return PACKER_VERSION_V1;
}

static void packVarInt64_impl(Packer pkr, const uint key, int64 value) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }

    dyb_append_typdex(packer->dyb, pkrdex_integer, key);
    dyb_append_var_s64(packer->dyb, value);
}

static void packFloat_impl(Packer pkr, const uint key, float value) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }

    dyb_append_typdex(packer->dyb, pkrdex_float, key);
    dyb_append_float(packer->dyb, value);
}

static void packDouble_impl(Packer pkr, const uint key, double value) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }

    dyb_append_typdex(packer->dyb, pkrdex_double, key);
    dyb_append_double(packer->dyb, value);
}

static void packData_impl(Packer pkr, const uint key, uint8* value, uint len) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }

    dyb_append_typdex(packer->dyb, pkrdex_raw, key);
    dyb_append_data_with_var_len(packer->dyb, value, len);
}

static void packArray_impl(Packer pkr, const uint key, uint len) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }

    dyb_append_typdex(packer->dyb, pkrdex_array, key);
    dyb_append_var_u64(packer->dyb, len+1);
    //plat_io_printf_std("Pack array\n");
}

static void packArrayEnd_impl(Packer pkr, const uint key) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }

    dyb_append_typdex(packer->dyb, pkrdex_array, key);
    dyb_append_var_u64(packer->dyb, 0);
    //plat_io_printf_std("Pack array End\n");
}

static uint db_save_object_name(MMPacker packer, const char* name) {
    PnI this_p2i;
    HASH_FIND_STR(packer->class_name_to_id, name, this_p2i);
    if (this_p2i) return (uint)this_p2i->i;

    this_p2i = mgn_mem_alloc(pool_of_mmobj(packer), sizeof(*this_p2i));
    this_p2i->i = HASH_COUNT(packer->class_name_to_id);
    this_p2i->p = (void*)name;

    HASH_ADD_KEYPTR(hh, packer->class_name_to_id, this_p2i->p, plat_cstr_length(this_p2i->p), this_p2i);

    dyb_append_typdex(packer->dyb, pkrdex_db, pkrdb_class_names);
    dyb_append_var_u64(packer->dyb, (uint64)this_p2i->i);
    dyb_append_cstring_with_var_len(packer->dyb, name);
    return (uint)this_p2i->i;
}

static void packObject_impl(Packer pkr, uint key, void* stru);
static void _packObject_impl(MMPacker packer, const uint key, MMObject obj) {
    if (obj == null) return;
    mgn_memory_pool* pool = pool_of_mmobj(obj);
    void* mem_addr = obj;
    PnI this_p2i, this_i2p;
    /// processing_i initialize in here is to avoid redundant serialization.
    int processing_i = HASH_COUNT(packer->obj_to_id);

    // save object reference
    HASH_FIND_PTR(packer->obj_to_id, &mem_addr, this_p2i);

    if (this_p2i == null) {
        this_p2i = mgn_mem_alloc(pool, sizeof(*this_p2i));
        size_t idx = HASH_COUNT(packer->obj_to_id);
        this_p2i->i = (int)idx;
        this_p2i->p = mem_addr;
        HASH_ADD_PTR(packer->obj_to_id, p, this_p2i);

        this_i2p = mgn_mem_alloc(pool, sizeof(*this_i2p));
        this_i2p->i = (int)idx;
        this_i2p->p = mem_addr;
        HASH_ADD_INT(packer->id_to_obj, i, this_i2p);
    }

    // output object reference
    dyb_append_typdex(packer->dyb, pkrdex_object_ref, key);
    dyb_append_var_u64(packer->dyb, (uint)this_p2i->i);

    if (packer->level > 1) {
        // not root object
        return;
    }

    /// Packing objects into memory
    while (HASH_COUNT(packer->id_to_obj) > (uint)processing_i) {
        HASH_FIND_INT(packer->id_to_obj, &processing_i, this_i2p);
        if (this_i2p == null) {
            plat_io_printf_err("This is impossible\n");
            return;
        }
        mem_addr = obj = this_i2p->p;
        const char* name = name_of_last_mmobj(obj);

        // save object name & index
        uint classname_num = db_save_object_name(packer, name);
        dyb_append_typdex(packer->dyb, pkrdex_object, (uint)processing_i);
        dyb_append_var_u64(packer->dyb, classname_num);

        mmStruBase base = base_of_first_mmobj(obj)->pre_base;       // use first one to find last one
        base->pack(base, packer);

        // End object
        dyb_append_typdex(packer->dyb, pkrdex_object, (uint)processing_i);

        processing_i++;
    }
}

static void packObject_impl(Packer pkr, const uint key, void* stru) {
    if (stru == null) {
        return;
    }
    MMObject obj = toMMObject(stru);
    if (obj == null) {
        plat_io_printf_err("This is not support object.\n");
        return;
    }
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }

    packer->level++;
    _packObject_impl(packer, key, obj);
    packer->level--;
}

static void packNextContext_impl(Packer pkr, void* stru)
{
    MMPacker packer = toMMPacker(pkr);
    const char* name = name_of_mmobj(stru);
    // save object name & index
    uint classname_num = db_save_object_name(packer, name);
    dyb_append_typdex(packer->dyb, pkrdex_context, classname_num);
}


MMPacker initMMPacker(MMPacker obj, Unpacker unpkr) {
    (void)unpkr;
    set_function_for_mmobj(obj, packerVersion, packerVersion_impl);
    set_function_for_mmobj(obj, packVarInt64, packVarInt64_impl);
    set_function_for_mmobj(obj, packFloat, packFloat_impl);
    set_function_for_mmobj(obj, packDouble, packDouble_impl);
    set_function_for_mmobj(obj, packData, packData_impl);
    set_function_for_mmobj(obj, packObject, packObject_impl);
    set_function_for_mmobj(obj, packArray, packArray_impl);
    set_function_for_mmobj(obj, packArrayEnd, packArrayEnd_impl);
    set_function_for_mmobj(obj, packNextContext, packNextContext_impl);

    obj->dyb = dyb_create(null, 64);
    if (obj->dyb == null) return null;

    // Append a version number first.
    dyb_append_typdex(obj->dyb, pkrdex_function, pkrfn_version);
    dyb_append_var_u64(obj->dyb, PACKER_VERSION_V1);
    return obj;
}

void destroyMMPacker(MMPacker obj) {
    if (obj->dyb) {
        dyb_release(obj->dyb);
    }

    PnI this_p2i, tmp_p2i, this_i2p, tmp_i2p;
    mgn_memory_pool* pool = pool_of_mmobj(obj);
    // release
    HASH_ITER(hh, obj->obj_to_id, this_p2i, tmp_p2i) {
        HASH_DEL(obj->obj_to_id, this_p2i);
        mgn_mem_release(pool, this_p2i);
    }

    HASH_ITER(hh, obj->id_to_obj, this_i2p, tmp_i2p) {
        HASH_DEL(obj->id_to_obj, this_i2p);
        mgn_mem_release(pool, this_i2p);
    }

    HASH_ITER(hh, obj->class_name_to_id, this_p2i, tmp_p2i) {
        HASH_DEL(obj->class_name_to_id, this_p2i);
        mgn_mem_release(pool, this_p2i);
    }
}


/// ===== MMUnpacker  =====
bool process(MMUnpacker unpkr);
#define CLASS_NAME          "__cls.name__"

/// ===== TmpContext =====
// Use to distinguish a context map from object properties

typedef struct TmpContext {

}*TmpContext;

MMSubObject(TmpContext, MMMap, null, null, null);

//===

static MMObject getByStringFromStack(MMUnpacker unpacker, const char* key) {
    MMObject obj = getLastItemFromMMList(unpacker->stack);
    MMMap map = toMMMap(obj);
    if (map) {
        MMString string = autorelease_mmobj(allocMMStringWithCString(pool_of_mmobj(obj), key));
        return getMMMapItemValue(map, toMMPrimary(string));
    }
    return null;
}

static MMObject getByIntFromStack(MMUnpacker unpacker, int idx) {
    MMObject obj = getLastItemFromMMList(unpacker->stack);
    MMMap map = toMMMap(obj);
    if (map) {
        MMInt val = autorelease_mmobj(allocMMIntWithValue(pool_of_mmobj(obj), idx));
        return getMMMapItemValue(map, toMMPrimary(val));
    }
    MMList list = toMMList(obj);
    if (list) {
        return getMMListItem(list, (uint)idx);
    }
    return null;
}

static void popContextInStack(MMUnpacker unpacker) {
    while (toTmpContext(getLastItemFromMMList(unpacker->stack))!=null)
        popMMListItem(unpacker->stack);
}

static uint unpackerVersion_impl(Unpacker unpkr)
{
    (void)unpkr;       // TODO: try to check unpkr
    return UNPACKER_VERSION_V1;
}

static int64 unpackVarInt64_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        plat_io_printf_err("Unsupported unpacker.(%s)\n", name_of_last_mmobj(unpkr));
        return 0;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    MMObject obj = getByIntFromStack(unpacker, key);
    MMLong val = toMMLong(obj);

    if (val==null) {
        plat_io_printf_err("Incorrect object@%u.(%s)\n", key, name_of_last_mmobj(obj));
        return 0;
    }

    if (is_unpkr_dbg) plat_io_printf_dbg("Unpacker var int:%lld\n", val->value);

    return val->value;
}

static float unpackFloat_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        plat_io_printf_err("Unsupported unpacker.(%s)\n", name_of_last_mmobj(unpkr));
        return 0;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    MMObject obj = getByIntFromStack(unpacker, key);
    MMFloat val = toMMFloat(obj);

    if (val==null) {
        plat_io_printf_err("Incorrect object@%u.(%s)\n", key, name_of_last_mmobj(obj));
        return 0;
    }

    if (is_unpkr_dbg) plat_io_printf_dbg("Unpacker var int:%f\n", val->value);

    return val->value;
}

static double unpackDouble_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        plat_io_printf_err("Unsupported unpacker.(%s)\n", name_of_last_mmobj(unpkr));
        return 0;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    MMObject obj = getByIntFromStack(unpacker, key);
    MMDouble val = toMMDouble(obj);

    if (val==null) {
        plat_io_printf_err("Incorrect object@%u.(%s)\n", key, name_of_last_mmobj(obj));
        return 0;
    }

    if (is_unpkr_dbg) plat_io_printf_dbg("Unpacker var int:%f\n", val->value);

    return val->value;
}

static uint8* unpackData_impl(Unpacker unpkr, const uint key, uint* p_len)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        plat_io_printf_err("Unsupported unpacker.(%s)\n", name_of_last_mmobj(unpkr));
        return null;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    MMObject obj = getByIntFromStack(unpacker, key);
    MMData dat = toMMData(obj);

    if (dat==null) {
        plat_io_printf_err("Incorrect object@%u.(%s)\n", key, name_of_last_mmobj(obj));
        return null;
    }

    if (p_len) *p_len = dat->size;

    if (dat->size!=0 && ((uint8*)dat->data)[dat->size-1]=='\0') {
        if (is_unpkr_dbg) plat_io_printf_dbg("Unpacked string:%s\n", (char*)dat->data);
    } else {
        if (is_unpkr_dbg) plat_io_printf_dbg("Unpacked data: size:%u\n", dat->size);
    }

    return dat->data;
}


static void* unpackObject_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        plat_io_printf_err("Unsupported unpacker.(%s)\n", name_of_last_mmobj(unpkr));
        return null;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    mgn_memory_pool* pool = pool_of_mmobj(unpacker);

    MMObject obj = getByIntFromStack(unpacker, key);
    if (obj == null) {
        return null;
    }
    MMInt num = toMMInt(obj);

    obj = getMMMapItemValue(unpacker->objects, toMMPrimary(num));
    // if object is referred before, this is a strong reference.
    MMReference reference = toMMReference(obj);
    if (reference) {
        //if (is_unpkr_dbg) plat_io_printf_dbg("Reference(%s): %p\n", name_of_mmobj(reference->reference), reference->reference);
        void* p = last_mmobj(reference->reference);
        if (is_unpkr_dbg) plat_io_printf_dbg("Reference: %p (%s)\n", p, name_of_last_mmobj(p));
        return p;
    }
    MMMap map = toMMMap(obj);

    obj = getMMMapItemValue(map, toMMPrimary(autorelease_mmobj(allocMMStringWithCString(pool, CLASS_NAME))));
    MMString obj_name = toMMString(obj);

    if (obj_name == null) {
        plat_io_printf_err("Why can't find object@%u name.\n", key);
        return null;
    }

    if (is_unpkr_dbg) plat_io_printf_dbg("Unpacking object: %s\n", obj_name->value);
    // push object's properties to stack
    pushMMListItem(unpacker->stack, toMMObject(map));

    obj = getMMMapItemValue(unpacker->allocators, toMMPrimary(obj_name));
    MMReference fnref = toMMReference(obj);
    if (fnref == null) {
        plat_io_printf_err("Not support unpack? Do you register (%s) to unpacker?\n", obj_name->value);
        return null;
    }
    void*(*alloc)(mgn_memory_pool*,Unpacker) = fnref->reference;

    unpacker->last_object_num = retain_mmobj(num);
    void* ooo = alloc(pool, unpacker);

    if (ooo == null) {
        plat_io_printf_err("Can't not unpack object.(%s)\n", obj_name->value);
    }

    if (unpacker->last_object_num) {
        // this mean the object is a non-initialization instance.
        // replace the real object into objects.
        if (ooo) {
            reference = autorelease_mmobj(allocMMReferenceWithStrongReference(pool, toMMObject(ooo)));
            addMMMapItem(unpacker->objects, toMMPrimary(unpacker->last_object_num), toMMObject(reference));
        }
        release_mmobj(unpacker->last_object_num);
        unpacker->last_object_num = null;
    } else {
        // pop last context
        popContextInStack(unpacker);
    }

    // pop object's properties
    popMMListItem(unpacker->stack);
    if (is_unpkr_dbg) plat_io_printf_dbg("Unpacked object: %s (%p)\n", obj_name->value, mem_addr_of_mmobj(ooo));

    return autorelease_mmobj(ooo);
}

static uint unpackArray_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        plat_io_printf_err("Unsupported unpacker.(%s)\n", name_of_last_mmobj(unpkr));
        return 0;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    MMObject obj = getByIntFromStack(unpacker, key);
    MMList list = toMMList(obj);

    if (list) {
        pushMMListItem(unpacker->stack, obj);
        if (is_unpkr_dbg) plat_io_printf_dbg("Unpacked array: count:%u\n", sizeOfMMList(list));
        return sizeOfMMList(list);
    } else {
        plat_io_printf_err("Why is not array?(%s)\n", name_of_last_mmobj(obj));
        return 0;
    }
}

static void unpackArrayEnd_impl(Unpacker unpkr, const uint key)
{
    (void)key;  // TODO: try to use key to verify unpack pair.
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        plat_io_printf_err("Unsupported unpacker.(%s)\n", name_of_last_mmobj(unpkr));
        return;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    popMMListItem(unpacker->stack);
}

static void unpackNextContext_impl(Unpacker unpkr, void* stru)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        plat_io_printf_err("Unsupported unpacker.(%s)\n", name_of_last_mmobj(unpkr));
        return;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    if (unpacker->last_object_num) {
        // replace the real object into objects.
        mgn_memory_pool* pool = pool_of_mmobj(unpacker);
        MMObject obj = toMMObject(stru);
        if (is_unpkr_dbg) plat_io_printf_dbg("Add to ref(%s): %p\n", name_of_mmobj(obj), obj);
        MMReference reference = autorelease_mmobj(allocMMReferenceWithStrongReference(pool, obj));
        addMMMapItem(unpacker->objects, toMMPrimary(unpacker->last_object_num), toMMObject(reference));
        release_mmobj(unpacker->last_object_num);
        unpacker->last_object_num = null;
    } else {
        // if not first context, need to pop last context
        popContextInStack(unpacker);
    }

    const char* name = name_of_mmobj(stru);
    MMObject obj = getByStringFromStack(unpacker, name);
    MMMap map = toMMMap(obj);

    if (map) {
        if (is_unpkr_dbg) plat_io_printf_dbg("- Switch to context:%s\n", name);
        pushMMListItem(unpacker->stack, obj);
    } else {
        // this is possible if init & pack are not pair, just implement an empty one.
        plat_io_printf_err("Why is not a map? Does object (%s:%s) implement init & pack at same time?\n", name, name_of_last_mmobj(obj));
    }

}

static void registerAllocator_impl(Unpacker unpkr, const char* obj_name, void* fn)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return;
    }

    mgn_memory_pool* pool = pool_of_mmobj(unpkr);
    if (unpacker->allocators == null) {
        unpacker->allocators = allocMMMap(pool);
    }
    addMMMapItem(unpacker->allocators,
                 toMMPrimary(autorelease_mmobj(allocMMStringWithCString(pool, obj_name))),
                 toMMObject(autorelease_mmobj(allocMMReferenceWithReference(pool, fn))));
}

static void pushIntoStack(MMUnpacker unpkr, int index, MMObject value) {
    MMObject context = getLastItemFromMMList(unpkr->stack);
    MMMap map = toMMMap(context);
    if (map) {
        addMMMapItem(map, toMMPrimary(autorelease_mmobj(allocMMIntWithValue(pool_of_mmobj(unpkr), index))), value);
        return;
    }
    MMList list = toMMList(context);
    if (list) {
        pushMMListItem(list, value);
        return;
    }

    plat_io_printf_err("What is this?(%s)\n", name_of_last_mmobj(context));
}

static void pushKeyVal(MMUnpacker unpkr, char* key, MMObject value) {
    MMObject context = getLastItemFromMMList(unpkr->stack);
    MMMap map = toMMMap(context);
    if (map) {
        addMMMapItem(map, toMMPrimary(autorelease_mmobj(allocMMStringWithCString(pool_of_mmobj(unpkr), key))), value);
        return;
    }

    plat_io_printf_err("What is this?(%s)\n", name_of_last_mmobj(context));
}

bool process(MMUnpacker unpkr) {
    uint8 type;
    uint index;
    MMObject current_obj = null;
    uint current_obj_num = (uint)~0;
    mgn_memory_pool* pool = pool_of_mmobj(unpkr);

    unpkr->roots = allocMMMap(pool);
    unpkr->objects = allocMMMap(pool);
    unpkr->stack = allocMMList(pool);
    pushMMListItem(unpkr->stack, toMMObject(unpkr->roots));

    /// Unpacking objects from memory in general form
    while (dyb_get_remainder(unpkr->dyb))
    {
        dyb_next_typdex(unpkr->dyb, &type, &index);
        switch ((pkrdex)type) {
            case pkrdex_integer: {
                int64 value = dyb_next_var_s64(unpkr->dyb);
                if (is_unpkr_dbg) plat_io_printf_dbg("%sVar int@%d: %lld\n", (current_obj?"- ":""), index, value);
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMLongWithValue(pool, value))));
                break;
            }
            case pkrdex_float: {
                float value = dyb_next_float(unpkr->dyb);
                if (is_unpkr_dbg) plat_io_printf_dbg("%sFloat@%d: %f\n", (current_obj?"- ":""), index, value);
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMFloatWithValue(pool, value))));
                break;
            }
            case pkrdex_double: {
                double value = dyb_next_double(unpkr->dyb);
                if (is_unpkr_dbg) plat_io_printf_dbg("%sDouble@%d: %f\n", (current_obj?"- ":""), index, value);
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMDoubleWithValue(pool, value))));
                break;
            }
            case pkrdex_raw: {
                uint size=0;
                uint8* data = dyb_next_data_with_var_len(unpkr->dyb, &size);
                if (size!=0 && data[size-1]=='\0') {
                    if (is_unpkr_dbg) plat_io_printf_dbg("%sData@%d: %s, size: %u\n", (current_obj?"- ":""), index, (const char*)data, size);
                } else {
                    if (is_unpkr_dbg) plat_io_printf_dbg("%sData@%d: size: %u\n", (current_obj?"- ":""), index, size);
                }
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMDataWithData(pool, data, size))));
                break;
            }
            case pkrdex_array: {
                uint64 size = dyb_next_var_u64(unpkr->dyb);
                if (size > 0) {
                    if (is_unpkr_dbg) plat_io_printf_dbg("%sArray@%d Start, size: %lld\n", (current_obj?"- ":""), index, size-1);
                    MMObject list = toMMObject(autorelease_mmobj(allocMMList(pool)));
                    pushIntoStack(unpkr, index, list);
                    pushMMListItem(unpkr->stack, list);
                } else {
                    if (is_unpkr_dbg) plat_io_printf_dbg("%sArray@%d End.\n", (current_obj?"- ":""), index);
                    // TODO: check index is matched.
                    popMMListItem(unpkr->stack);
                }
                break;
            }
            case pkrdex_object_ref: {
                uint64 num = dyb_next_var_u64(unpkr->dyb);
                if (is_unpkr_dbg) plat_io_printf_dbg("%sObject ref@%d: #%lld\n", (current_obj?"- ":""), index, num);
                // we push a number as obj reference.
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMIntWithValue(pool, (int)num))));
                break;
            }
            case pkrdex_object: {
                if (current_obj == null) {
                    // new one, put properties map
                    current_obj = toMMObject(autorelease_mmobj(allocMMMap(pool)));
                    current_obj_num = index;
                    //uint size;
                    int classname_num = (int)dyb_next_var_u64(unpkr->dyb);
                    PnI this_i2p;
                    HASH_FIND_INT(unpkr->id_to_class_name, &classname_num, this_i2p);
                    if (this_i2p == null) {
                        plat_io_printf_err("Lost class name?(%d)\n", classname_num);
                        return false;
                    } else {
                        if (is_unpkr_dbg) plat_io_printf_dbg("New object#%u: %s\n", (unsigned int)current_obj_num, (const char*)this_i2p->p);
                    }
                    pushMMListItem(unpkr->stack, current_obj);
                    addMMMapItem(unpkr->objects, toMMPrimary(autorelease_mmobj(allocMMIntWithValue(pool, current_obj_num))), current_obj);
                    pushKeyVal(unpkr, CLASS_NAME, toMMObject(autorelease_mmobj(allocMMStringWithCString(pool, this_i2p->p))));
                } else {
                    if (current_obj_num != index) {
                        plat_io_printf_err("Missed object end. (#%u!=#%u)\n", (unsigned int)current_obj_num, (unsigned int)index);
                        return false;
                    }
                    if (is_unpkr_dbg) plat_io_printf_dbg("End object#%u\n", (unsigned int)current_obj_num);
                    while (getLastItemFromMMList(unpkr->stack) != current_obj) {
                        // context is always in object.
                        popMMListItem(unpkr->stack);
                    }
                    current_obj_num = (uint)~0;
                    current_obj = null;
                    popMMListItem(unpkr->stack);
                }
                break;
            }
            case pkrdex_context: {
                if (!current_obj) {
                    plat_io_printf_err("Incorrect status.");
                    return false;
                }
                int classname_num = (int)index;
                PnI this_i2p;
                HASH_FIND_INT(unpkr->id_to_class_name, &classname_num, this_i2p);
                if (this_i2p == null) {
                    plat_io_printf_err("Lost class name?(%d)\n", classname_num);
                    return false;
                } else {
                    if (is_unpkr_dbg) plat_io_printf_dbg("- Next context: %s\n", (const char*)this_i2p->p);
                }

                while (getLastItemFromMMList(unpkr->stack) != current_obj) {
                    // context is always in object.
                    popMMListItem(unpkr->stack);
                }

                MMObject context = toMMObject(autorelease_mmobj(allocTmpContext(pool)));
                pushKeyVal(unpkr, this_i2p->p, context);
                pushMMListItem(unpkr->stack, context);
                break;
            }
            case pkrdex_db: {
                switch((pkrdb)index)
                {
                    case pkrdb_class_names:
                    {
                        // class names db
                        PnI this_i2p;
                        int classname_num = (int)dyb_next_var_u64(unpkr->dyb);
                        HASH_FIND_INT(unpkr->id_to_class_name, &classname_num, this_i2p);
                        if (this_i2p) {
                            plat_io_printf_err("Duplicate class name?(%d)\n", classname_num);
                            return false;
                        } else {
                            uint size;
                            this_i2p = mgn_mem_alloc(pool, sizeof(*this_i2p));
                            this_i2p->i = classname_num;
                            this_i2p->p = dyb_next_cstring_with_var_len(unpkr->dyb, &size);
                            HASH_ADD_INT(unpkr->id_to_class_name, i, this_i2p);
                            if (is_unpkr_dbg) plat_io_printf_dbg("$DB$, class name #%d: %s\n", classname_num, (const char*)this_i2p->p);
                        }
                        break;
                    }
                    default:
                    {
                        plat_io_printf_err("Unsupported db.(%d)\n", index);
                        return false;
                    }
                }
                break;
            }
            case pkrdex_function: {
                switch((pkrfn)index) {
                    case pkrfn_version:
                    {
                        uint64 version = dyb_next_var_u64(unpkr->dyb);
                        if (version == UNPACKER_VERSION_V1) {
                            if (is_unpkr_dbg) plat_io_printf_dbg("Using version 1 pack format\n");
                        } else {
                            plat_io_printf_err("Unsupported pack version.(%llu)\n", version);
                        }
                        break;
                    }
                    default:
                    {
                        plat_io_printf_err("Unsupported function.(%d)\n", index);
                        return false;
                    }
                }
                break;
            }
            default: {
                plat_io_printf_err("Unsupported pack/unpack format.(%d)\n", type);
                return false;
            }
        }
    }

    if (is_unpkr_dbg) plat_io_printf_dbg("Depth of stack:%u\n", sizeOfMMList(unpkr->stack));
    uint i;
    for (i=0; i<sizeOfMMList(unpkr->stack); i++) {
        if (is_unpkr_dbg) plat_io_printf_dbg("%s\n", name_of_last_mmobj(getMMListItem(unpkr->stack, i)));
    }
    return true;
}

MMUnpacker initMMUnpacker(MMUnpacker obj, Unpacker unpkr) {
    (void)unpkr;
    set_function_for_mmobj(obj, unpackerVersion, unpackerVersion_impl);
    set_function_for_mmobj(obj, unpackVarInt64, unpackVarInt64_impl);
    set_function_for_mmobj(obj, unpackFloat, unpackFloat_impl);
    set_function_for_mmobj(obj, unpackDouble, unpackDouble_impl);
    set_function_for_mmobj(obj, unpackData, unpackData_impl);
    set_function_for_mmobj(obj, unpackObject, unpackObject_impl);
    set_function_for_mmobj(obj, unpackArray, unpackArray_impl);
    set_function_for_mmobj(obj, unpackArrayEnd, unpackArrayEnd_impl);
    set_function_for_mmobj(obj, unpackNextContext, unpackNextContext_impl);
    set_function_for_mmobj(obj, registerAllocator, registerAllocator_impl);
    return obj;
}

void destroyMMUnpacker(MMUnpacker obj) {
    mgn_memory_pool* pool = pool_of_mmobj(obj);
    PnI this_i2p, tmp_i2p;
    HASH_ITER(hh, obj->id_to_class_name, this_i2p, tmp_i2p) {
        HASH_DEL(obj->id_to_class_name, this_i2p);
        mgn_mem_release(pool, this_i2p);
    }
    if (obj->dyb) {
        dyb_release(obj->dyb);
    }
    release_mmobj(obj->objects);
    release_mmobj(obj->roots);
    release_mmobj(obj->stack);
    release_mmobj(obj->allocators);
    release_mmobj(obj->last_object_num);
}

MMUnpacker allocMMUnpackerWithData(mgn_memory_pool* pool, uint8* data, uint len) {
    MMUnpacker unpacker = allocMMUnpacker(pool);
    if (unpacker) {
        unpacker->dyb = dyb_refer(null, data, len, false);
        if (unpacker->dyb == null) {
            release_mmobj(unpacker);
            return null;
        }
    }
    return unpacker;
}