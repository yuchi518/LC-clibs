//
// Created by Yuchi Chen on 2017/10/9.
//

#ifndef PROC_LA_MMO_PACK_H
#define PROC_LA_MMO_PACK_H

#include "mmo.h"
#include "mmo_ext.h"
#include "dybuf.h"

/**
 *  Packer/Unpacker, Version 1.
 */
typedef enum {
    pkrdex_integer      = 0x1,  // type, index, variable sign integer
    pkrdex_float        = 0x2,  // type, index, float
    pkrdex_double       = 0x3,  // type, index, double

    pkrdex_raw          = 0x7,  // type, index, size(variable unsigned integer), data
    pkrdex_array        = 0x8,  // type, index, size(variable unsigned integer)

    pkrdex_object_ref   = 0xa,  // type, index, object number(variable unsigned integer)
    pkrdex_object       = 0xb,  // type, object number(variable unsigned integer), class name num, object data ....     -> Start
                                // type, object number(variable unsigned integer)                                       -> End
    pkrdex_db           = 0xc,  // type, index (Database category), ...
                                // type, 0 (class name db), class name number (variable unsigned integer), class name string
    pkrdex_function     = 0xf,  // type, index (function number)
                                // type, 0: version, version number(variable unsigned integer)
} pkrdex;

typedef enum {
    pkrfn_version       = 0x1,
} pkrfn;

typedef enum {
    pkrdb_class_names   = 0x1,
} pkrdb;


/**
 * *** NOTICE ***
 * Pointer p is for reference, it doesn't own the memory.
 * After pack or unpack object, all pointer should be discarded
 * and don't use it again.
 * Sometimes p refers to the memory of packing objects, and sometimes
 * it refers the memory of dybuf. It means packing objects and dybuf
 * should keep memory ownership until pack/unpack done.
 */
typedef struct {
    int i;
    void* p;
    UT_hash_handle hh;
}* PnI;

/// ===== Packer =====
typedef struct MMPacker {
    int level;
    PnI obj_to_id;
    PnI id_to_obj;
    PnI class_name_to_id; // name -> id
    dybuf* dyb;
}*MMPacker;

MMPacker initMMPacker(MMPacker obj, Unpacker unpkr);
void destroyMMPacker(MMPacker obj);

MMSubObject(MMOBJ_PACKER, MMPacker, MMObject , initMMPacker, destroyMMPacker, null);


/// ===== Unpacker =====
typedef struct MMUnpacker {
    MMMap roots;
    MMMap objects;
    PnI id_to_class_name; // id -> name
    dybuf* dyb;
}*MMUnpacker;

MMUnpacker initMMUnpacker(MMUnpacker obj, Unpacker unpkr);
void destroyMMUnpacker(MMUnpacker obj);

MMSubObject(MMOBJ_UNPACKER, MMUnpacker, MMObject , initMMUnpacker, destroyMMUnpacker, null);

MMUnpacker allocMMUnpackerWithData(mgn_memory_pool* pool, uint8* data, uint len);

#endif //PROC_LA_MMO_PACK_H






