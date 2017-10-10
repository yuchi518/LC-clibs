//
// Created by Yuchi Chen on 2017/10/9.
//

#include "mmo_pack.h"

typedef enum {
    pkrdex_object       = 0x0,
    pkrdex_integer      = 0x1,        // variable sign integer
    pkrdex_float        = 0x2,
    pkrdex_double       = 0x3,

    pkrdex_raw          = 0x7,

    pkrdex_array        = 0xa,

    pkrdex_function     = 0xf,
} pkrdex;

/// ===== MMPacker =====

static uint packerVersion_impl(Packer packer) {
    return 0x01;
}

static void packVarInt64_impl(Packer pkr, const uint key, int64 value) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }
}

static void packFloat_impl(Packer pkr, const uint key, float value) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }
}

static void packDouble_impl(Packer pkr, const uint key, double value) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }
}

static void packData_impl(Packer pkr, const uint key, uint8* value, uint len) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }
}

static void packArray_impl(Packer pkr, const uint key, uint len) {
    MMPacker packer = toMMPacker(pkr);
    if (packer == null) {
        plat_io_printf_err("Who am I?(%s)\n", name_of_last_mmobj(pkr));
        return;
    }
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

    if (packer->level > 1) {
        // not root object
        // TODO: output object index
        return;
    }

    int processing_i = 0;
    const char* name = name_of_last_mmobj(obj);
    PnI tmp_p2i, tmp_i2p;

    while (HASH_COUNT(packer->i2p) > processing_i) {
        HASH_FIND_INT(packer->i2p, processing_i, this_i2p);
        if (this_i2p == null) {
            plat_io_printf_err("This is impossible\n");
            return;
        }

        // TODO: save object name & index

        mmBase base = base_of_first_mmobj(obj)->pre_base;       // use first one to find last one
        base->pack(base, packer);

        // TODO: end object

        processing_i++;
    }

    // release
    HASH_ITER(hh, packer->p2i, this_p2i, tmp_p2i) {
        HASH_DEL(packer->p2i, this_p2i);
        mgn_mem_release(pool, this_p2i);
    }

    HASH_ITER(hh, packer->i2p, this_i2p, tmp_i2p) {
        HASH_DEL(packer->i2p, this_i2p);
        mgn_mem_release(pool, this_i2p);
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
    //obj->level
    (void)unpkr;
    set_function_for_mmobj(obj, packerVersion, packerVersion_impl);
    set_function_for_mmobj(obj, packVarInt64, packVarInt64_impl);
    set_function_for_mmobj(obj, packFloat, packFloat_impl);
    set_function_for_mmobj(obj, packDouble, packDouble_impl);
    set_function_for_mmobj(obj, packData, packData_impl);
    set_function_for_mmobj(obj, packObject, packObject_impl);
    set_function_for_mmobj(obj, packArray, packArray_impl);
    set_function_for_mmobj(obj, packNextContext, packNextContext_impl);
    return obj;
}

void destroyMMPacker(MMPacker obj) {

}


/// ===== MMUnpacker  =====
static uint unpackerVersion_impl(Unpacker unpkr)
{
    return 0;
}

static int64 unpackVarInt64_impl(Unpacker unpkr, const uint key)
{
    return 0;
}

static float unpackFloat_impl(Unpacker unpkr, const uint key)
{
    return 0;
}

static double unpackDouble_impl(Unpacker unpkr, const uint key)
{
    return 0;
}

static uint8* unpackData_impl(Unpacker unpkr, const uint key, uint* p_len)
{
    return 0;
}

static void* unpackObject_impl(Unpacker unpkr, const uint key)
{
    return null;
}

static uint unpackArray_impl(Unpacker unpkr, const uint key)
{
    return 0;
}

static void* unpackArrayItem_impl(Unpacker unpkr, const uint key, const uint index)
{
    return 0;
}

static void unpackNextContext_impl(Unpacker unpkr, const char* context/*classname*/)
{

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

}