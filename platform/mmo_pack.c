//
// Created by Yuchi Chen on 2017/10/9.
//

#include "mmo_pack.h"

typedef enum {
    pkrdex_integer      = 0x1,  // type, index, variable sign integer
    pkrdex_float        = 0x2,  // type, index, float
    pkrdex_double       = 0x3,  // type, index, double

    pkrdex_raw          = 0x7,  // type, index, size(variable unsigned integer), data
    pkrdex_array        = 0x8,  // type, index, size(variable unsigned integer)

    pkrdex_object_ref   = 0xa,  // type, index, object number(variable unsigned integer)
    pkrdex_object       = 0xb,  // type, object number(variable unsigned integer), class name num, object data ....  -> Start
                                // type, object number(variable unsigned integer)                                   -> End
    pkrdex_namedb       = 0xc,  // type, scope, class name number (variable unsigned integer), class name string
    pkrdex_function     = 0xf,  // type, index (function number)
                                // type, 0: version, version number(variable unsigned integer)
} pkrdex;

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
    dyb_append_var_u64(packer->dyb, len);
    plat_io_printf_std("Pack array\n");
}

static uint save_classname(MMPacker packer, const char* name) {
    PnI this_p2i;
    HASH_FIND_STR(packer->class_names, name, this_p2i);
    if (this_p2i) return (uint)this_p2i->i;

    this_p2i = mgn_mem_alloc(pool_of_mmobj(packer), sizeof(*this_p2i));
    this_p2i->i = HASH_COUNT(packer->class_names);
    this_p2i->p = (void*)name;

    HASH_ADD_KEYPTR(hh, packer->class_names, this_p2i->p, plat_cstr_length(this_p2i->p), this_p2i);

    dyb_append_typdex(packer->dyb, pkrdex_namedb, 0/*class name scope*/);
    dyb_append_var_u64(packer->dyb, (uint64)this_p2i->i);
    dyb_append_cstring_with_var_len(packer->dyb, name);
    return (uint)this_p2i->i;
}

static void packObject_impl(Packer pkr, const uint key, void* stru);
static void _packObject_impl(MMPacker packer, const uint key, MMObject obj) {
    mgn_memory_pool* pool = pool_of_mmobj(obj);
    void* mem_addr = obj;
    PnI this_p2i, this_i2p;

    HASH_FIND_PTR(packer->p2i, &mem_addr, this_p2i);

    if (this_p2i == null) {
        this_p2i = mgn_mem_alloc(pool, sizeof(*this_p2i));
        size_t idx = HASH_COUNT(packer->p2i);
        this_p2i->i = idx;
        this_p2i->p = mem_addr;
        HASH_ADD_PTR(packer->p2i, p, this_p2i);

        this_i2p = mgn_mem_alloc(pool, sizeof(*this_i2p));
        this_i2p->i = idx;
        this_i2p->p = mem_addr;
        HASH_ADD_INT(packer->i2p, i, this_i2p);
    }

    // output object reference
    dyb_append_typdex(packer->dyb, pkrdex_object_ref, key);
    dyb_append_var_u64(packer->dyb, (uint)this_p2i->i);

    if (packer->level > 1) {
        // not root object
        return;
    }

    int processing_i = 0;

    while (HASH_COUNT(packer->i2p) > (uint)processing_i) {
        HASH_FIND_INT(packer->i2p, &processing_i, this_i2p);
        if (this_i2p == null) {
            plat_io_printf_err("This is impossible\n");
            return;
        }
        mem_addr = obj = this_i2p->p;
        const char* name = name_of_last_mmobj(obj);

        // save object name & index
        uint classname_num = save_classname(packer, name);
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

void packNextContext_impl(Packer packer, const char* context/*classname*/)
{

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
    set_function_for_mmobj(obj, packNextContext, packNextContext_impl);

    obj->dyb = dyb_create(null, 64);
    if (obj->dyb == null) return null;

    // Append a version number first.
    dyb_append_typdex(obj->dyb, pkrdex_function, 0);
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
    HASH_ITER(hh, obj->p2i, this_p2i, tmp_p2i) {
        HASH_DEL(obj->p2i, this_p2i);
        mgn_mem_release(pool, this_p2i);
    }

    HASH_ITER(hh, obj->i2p, this_i2p, tmp_i2p) {
        HASH_DEL(obj->i2p, this_i2p);
        mgn_mem_release(pool, this_i2p);
    }

    HASH_ITER(hh, obj->class_names, this_p2i, tmp_p2i) {
        HASH_DEL(obj->class_names, this_p2i);
        mgn_mem_release(pool, this_p2i);
    }
}


/// ===== MMUnpacker  =====
bool process(MMUnpacker unpkr);

static uint unpackerVersion_impl(Unpacker unpkr)
{
    return UNPACKER_VERSION_V1;
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

static void unpackNextContext_impl(Unpacker unpkr, const char* context/*classname*/)
{
    MMUnpacker unpacker = toMMUnpacker(unpkr);
    if (unpacker == null) {
        return;
    }
    if (unpacker->roots==null) {
        process(unpacker);
    }

}

bool process(MMUnpacker unpkr) {
    uint8 type;
    uint index;
    MMMap current_obj = null;
    uint current_obj_num = (uint)~0;
    mgn_memory_pool* pool = pool_of_mmobj(unpkr);

    unpkr->roots = allocMMMap(pool_of_mmobj(unpkr));

    while (dyb_get_remainder(unpkr->dyb))
    {
        dyb_next_typdex(unpkr->dyb, &type, &index);
        switch ((pkrdex)type) {
            case pkrdex_integer: {
                if (!current_obj) {
                    plat_io_printf_err("Incorrect status.");
                }
                int64 value = dyb_next_var_s64(unpkr->dyb);
                plat_io_printf_std("var int(%d):%lld\n", index, value);
                break;
            }
            case pkrdex_float: {
                if (!current_obj) {
                    plat_io_printf_err("Incorrect status.");
                }
                float value = dyb_next_float(unpkr->dyb);
                plat_io_printf_std("float(%d):%f\n", index, value);
                break;
            }
            case pkrdex_double: {
                if (!current_obj) {
                    plat_io_printf_err("Incorrect status.");
                }
                double value = dyb_next_double(unpkr->dyb);
                plat_io_printf_std("double(%d):%f\n", index, value);
                break;
            }
            case pkrdex_raw: {
                if (!current_obj) {
                    plat_io_printf_err("Incorrect status.");
                }
                uint size;
                uint8* data = dyb_next_data_with_var_len(unpkr->dyb, &size);
                plat_io_printf_std("data(%d):%p, size:%u\n", index, (void*)data, size);
                break;
            }
            case pkrdex_array: {
                if (!current_obj) {
                    plat_io_printf_err("Incorrect status.");
                }
                uint64 size = dyb_next_var_u64(unpkr->dyb);
                plat_io_printf_std("array size(%d):%lld\n", index, size);
                break;
            }
            case pkrdex_object_ref: {
                //if (!current_obj) {
                //    plat_io_printf_err("Incorrect status.");
                //}
                uint64 num = dyb_next_var_u64(unpkr->dyb);
                plat_io_printf_std("object ref(%d):%lld\n", index, num);
                break;
            }
            case pkrdex_object: {
                if (current_obj == null) {
                    // new one
                    current_obj = allocMMMap(pool);
                    current_obj_num = index;
                    uint size;
                    int classname_num = dyb_next_var_u64(unpkr->dyb);
                    PnI this_i2p;
                    HASH_FIND_INT(unpkr->class_names, &classname_num, this_i2p);
                    if (this_i2p == null) {
                        plat_io_printf_err("Lost class name?(%d)\n", classname_num);
                        return false;
                    } else {
                        plat_io_printf_std("New object: %s #%u\n", (const char*)this_i2p->p, (unsigned int)current_obj_num);
                    }
                } else {
                    if (current_obj_num != index) {
                        plat_io_printf_err("Missed object end. (%u!=%u)\n", (unsigned int)current_obj_num, (unsigned int)index);
                        return false;
                    }
                    plat_io_printf_std("End object: #%u\n", (unsigned int)current_obj_num);
                    current_obj_num = (uint)~0;
                    release_mmobj(current_obj);
                    current_obj = null;
                }
                break;
            }
            case pkrdex_namedb: {
                if (index == 0) {
                    // class name domain
                    PnI this_i2p;
                    int classname_num = (int)dyb_next_var_u64(unpkr->dyb);
                    HASH_FIND_INT(unpkr->class_names, &classname_num, this_i2p);
                    if (this_i2p) {
                        plat_io_printf_err("Duplicate class name?(%d)\n", classname_num);
                        return false;
                    } else {
                        uint size;
                        this_i2p = mgn_mem_alloc(pool, sizeof(*this_i2p));
                        this_i2p->i = classname_num;
                        this_i2p->p = dyb_next_cstring_with_var_len(unpkr->dyb, &size);
                        HASH_ADD_INT(unpkr->class_names, i, this_i2p);
                    }
                }
                break;
            }
            case pkrdex_function: {
                switch(index) {
                    case 0:
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
    return obj;
}

void destroyMMUnpacker(MMUnpacker obj) {
    release_mmobj(obj->objects);
    release_mmobj(obj->roots);
    if (obj->dyb) {
        dyb_release(obj->dyb);
    }
    PnI this_i2p, tmp_i2p;
    mgn_memory_pool* pool = pool_of_mmobj(obj);
    HASH_ITER(hh, obj->class_names, this_i2p, tmp_i2p) {
        HASH_DEL(obj->class_names, this_i2p);
        mgn_mem_release(pool, this_i2p);
    }
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