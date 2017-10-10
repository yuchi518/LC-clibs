//
// Created by Yuchi Chen on 2017/10/9.
//

#ifndef PROC_LA_MMO_PACK_H
#define PROC_LA_MMO_PACK_H

#include "mmo.h"
#include "mmo_ext.h"

typedef struct {
    int i;
    void* p;
    UT_hash_handle hh;
}* PnI;

/// ===== Packer =====
typedef struct MMPacker {
    PnI p2i;
    PnI i2p;
    int level;
}*MMPacker;

MMPacker initMMPacker(MMPacker obj, Unpacker unpkr);
void destroyMMPacker(MMPacker obj);

MMSubObject(MMOBJ_PACKER, MMPacker, MMObject , initMMPacker, destroyMMPacker, null);


/// ===== Unpacker =====
typedef struct MMUnpacker {

}*MMUnpacker;

MMUnpacker initMMUnpacker(MMUnpacker obj, Unpacker unpkr);
void destroyMMUnpacker(MMUnpacker obj);

MMSubObject(MMOBJ_UNPACKER, MMUnpacker, MMObject , initMMUnpacker, destroyMMUnpacker, null);




#endif //PROC_LA_MMO_PACK_H






