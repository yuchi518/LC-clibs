//
// Created by Yuchi Chen on 2017/10/9.
//

#include "mmo_pack.h"

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
    plat_io_printf_std("Pack array\n");
}

static void packArrayEnd_impl(Packer pkr, const uint key) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }

    dyb_append_typdex(packer->dyb, pkrdex_array, key);
    dyb_append_var_u64(packer->dyb, 0);
    plat_io_printf_std("Pack array End\n");
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
    mgn_memory_pool* pool = pool_of_mmobj(obj);
    void* mem_addr = obj;
    PnI this_p2i, this_i2p;

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

    int processing_i = 0;

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

        mmBase base = base_of_first_mmobj(obj)->pre_base;       // use first one to find last one
        base->pack(base, packer);

        // End object
        dyb_append_typdex(packer->dyb, pkrdex_object, (uint)processing_i);

        processing_i++;
    }
}

static void packObject_impl(Packer pkr, const uint key, void* stru) {
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
static uint unpackerVersion_impl(Unpacker unpkr)
{
    return UNPACKER_VERSION_V1;
}

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
        return getMMListItem(list, idx);
    }
    return null;
}

static int64 unpackVarInt64_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return 0;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    return 0;
}

static float unpackFloat_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return 0;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    return 0;
}

static double unpackDouble_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return 0;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    return 0;
}

static uint8* unpackData_impl(Unpacker unpkr, const uint key, uint* p_len)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return null;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    return null;
}

static void* unpackObject_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return null;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    MMObject obj = getByIntFromStack(unpacker, key);
    MMInt ref = toMMInt(obj);

    obj = getMMMapItemValue(unpacker->objects, toMMPrimary(ref));
    MMMap map = toMMMap(obj);

    obj = getMMMapItemValue(map, toMMPrimary(autorelease_mmobj(allocMMStringWithCString(pool_of_mmobj(unpacker), CLASS_NAME))));
    MMString obj_name = toMMString(obj);

    plat_io_printf_std("Object name:%s\n", obj_name->value);

    // TODO: unpack now...

    return null;
}

static uint unpackArray_impl(Unpacker unpkr, const uint key)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return 0;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    return 0;
}

static void* unpackArrayItem_impl(Unpacker unpkr, const uint key, const uint index)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return null;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    return null;
}

static void unpackNextContext_impl(Unpacker unpkr, void* stru)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

    // TODO: ~~~
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
                plat_io_printf_std("%sVar int@%d: %lld\n", (current_obj?"- ":""), index, value);
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMLongWithValue(pool, value))));
                break;
            }
            case pkrdex_float: {
                float value = dyb_next_float(unpkr->dyb);
                plat_io_printf_std("%sFloat@%d: %f\n", (current_obj?"- ":""), index, value);
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMFloatWithValue(pool, value))));
                break;
            }
            case pkrdex_double: {
                double value = dyb_next_double(unpkr->dyb);
                plat_io_printf_std("%sDouble@%d: %f\n", (current_obj?"- ":""), index, value);
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMDoubleWithValue(pool, value))));
                break;
            }
            case pkrdex_raw: {
                uint size=0;
                uint8* data = dyb_next_data_with_var_len(unpkr->dyb, &size);
                if (data[size-1] == '\0') {
                    plat_io_printf_std("%sData@%d: %s, size: %u\n", (current_obj?"- ":""), index, (const char*)data, size);
                } else {
                    plat_io_printf_std("%sData@%d: size: %u\n", (current_obj?"- ":""), index, size);
                }
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMDataWithData(pool, data, size))));
                break;
            }
            case pkrdex_array: {
                uint64 size = dyb_next_var_u64(unpkr->dyb);
                if (size > 0) {
                    plat_io_printf_std("%sArray@%d Start, size: %lld\n", (current_obj?"- ":""), index, size-1);
                    MMObject list = toMMObject(autorelease_mmobj(allocMMList(pool)));
                    pushIntoStack(unpkr, index, list);
                    pushMMListItem(unpkr->stack, list);
                } else {
                    plat_io_printf_std("%sArray@%d End.\n", (current_obj?"- ":""), index);
                    // TODO: check index is matched.
                    popMMListItem(unpkr->stack);
                }
                break;
            }
            case pkrdex_object_ref: {
                uint64 num = dyb_next_var_u64(unpkr->dyb);
                plat_io_printf_std("%sObject ref@%d: #%lld\n", (current_obj?"- ":""), index, num);
                // we push a number as obj reference.
                pushIntoStack(unpkr, index, toMMObject(autorelease_mmobj(allocMMIntWithValue(pool, (int)num))));
                break;
            }
            case pkrdex_object: {
                if (current_obj == null) {
                    // new one
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
                        plat_io_printf_std("New object#%u: %s\n", (unsigned int)current_obj_num, (const char*)this_i2p->p);
                    }
                    pushMMListItem(unpkr->stack, current_obj);
                    addMMMapItem(unpkr->objects, toMMPrimary(autorelease_mmobj(allocMMIntWithValue(pool, current_obj_num))), current_obj);
                    pushKeyVal(unpkr, CLASS_NAME, toMMObject(autorelease_mmobj(allocMMStringWithCString(pool, this_i2p->p))));
                } else {
                    if (current_obj_num != index) {
                        plat_io_printf_err("Missed object end. (#%u!=#%u)\n", (unsigned int)current_obj_num, (unsigned int)index);
                        return false;
                    }
                    plat_io_printf_std("End object#%u\n", (unsigned int)current_obj_num);
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
                    plat_io_printf_std("- Next context: %s\n", (const char*)this_i2p->p);
                }

                while (getLastItemFromMMList(unpkr->stack) != current_obj) {
                    // context is always in object.
                    popMMListItem(unpkr->stack);
                }

                MMObject context = toMMObject(autorelease_mmobj(allocMMMap(pool)));
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
                            plat_io_printf_std("$DB$, class name #%d: %s\n", classname_num, (const char*)this_i2p->p);
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
                            plat_io_printf_std("Using version 1 pack format\n");
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

    plat_io_printf_std("Depth of stack:%u\n", sizeOfMMList(unpkr->stack));
    uint i;
    for (i=0; i<sizeOfMMList(unpkr->stack); i++) {
        plat_io_printf_std("%s\n", name_of_last_mmobj(getMMListItem(unpkr->stack, i)));
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
    set_function_for_mmobj(obj, unpackArrayItem, unpackArrayItem_impl);
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