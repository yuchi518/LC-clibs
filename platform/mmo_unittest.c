//
// Created by Yuchi Chen on 2017/8/27.
//

#include "mmo_unittest.h"

/// ========== Unit test ==========

void unit_test_mmobj(void)
{
    plat_io_printf_dbg("===== unit_test_mmobj - START =====\n");

    mgn_memory_pool pool = null;
    MMSon son = allocMMSon(&pool);
    MMChild child = toMMChild(son);
    MMRoot root = toMMRoot(child);

    plat_io_printf_dbg("Son's name = %s\n", name_of_mmobj(son));
    plat_io_printf_dbg("Child's name = %s\n", name_of_mmobj(child));
    plat_io_printf_dbg("Root's name = %s\n", name_of_mmobj(root));

    release_mmobj(child);
    if (mgn_mem_count_of_mem(&pool) != 0)
    {
        plat_io_printf_err("Memory leak? (%zu)\n", mgn_mem_count_of_mem(&pool));
    }

    MMString hi5 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi5"));
    MMInt v5 = autorelease_mmobj(allocMMInt(&pool));
    v5->value = 5;

    MMString hi6 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi6"));
    MMDouble v6 = autorelease_mmobj(allocMMDouble(&pool));
    v6->value = 6.0;

    MMMap map = allocMMMap(&pool);

    addMMMapItem(map, toMMPrimary(hi5), toMMObject(v5));
    addMMMapItem(map, toMMPrimary(hi6), toMMObject(v6));

    MMObject obj = getMMMapItemValue(map, toMMPrimary(hi5));
    if (oid_of_last_mmobj(obj) == oid_of_MMInt()) {
        plat_io_printf_dbg("Got it!! (%d)\n", toMMInt(obj)->value);
    } else {
        plat_io_printf_err("What is this?(%u)\n", oid_of_last_mmobj(obj));
    }

    obj = getMMMapItemValue(map, toMMPrimary(hi6));
    if (oid_of_last_mmobj(obj) == oid_of_MMDouble()) {
        plat_io_printf_dbg("Got it!! (%f)\n", toMMDouble(obj)->value);
    } else {
        plat_io_printf_err("What is this?(%u)\n", oid_of_last_mmobj(obj));
    }

    release_mmobj(map);
    if (mgn_mem_count_of_mem(&pool) != 0)
    {
        plat_io_printf_err("Memory leak? (%zu)\n", mgn_mem_count_of_mem(&pool));
    }

    map = allocMMMap(&pool);

    MMString hi7 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi7"));
    MMLong v7 = autorelease_mmobj(allocMMLong(&pool));
    v7->value = 7;

    MMString hi8 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi8"));
    MMLong v8 = autorelease_mmobj(allocMMLong(&pool));
    v8->value = 8;

    void* hash_key = null;
    uint hash_key_len = 0;
    hash_of_mmobj(toMMPrimary(v7), &hash_key, &hash_key_len);

    addMMMapItem(map, toMMPrimary(v7), toMMObject(hi7));
    addMMMapItem(map, toMMPrimary(v8), toMMObject(hi8));

    obj = getMMMapItemValue(map, toMMPrimary(v7));
    if (oid_of_last_mmobj(obj) == oid_of_MMString()) {
        plat_io_printf_dbg("Got it!! (%s)\n", toMMString(obj)->value);
    }

    /*release_mmobj(map);
    if (mgn_mem_count_of_mem(&pool) != 0)
    {
        plat_io_printf_err("Memory leak? (%zu)\n", mgn_mem_count_of_mem(&pool));
    }*/

    MMList list = allocMMList(&pool);
    MMString hi9 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi9"));
    MMFloat v9 = autorelease_mmobj(allocMMFloat(&pool));
    v9->value = 9;

    pushMMListItem(list, toMMObject(hi9));
    pushMMListItem(list, toMMObject(v9));
    pushMMListItem(list, toMMObject(autorelease_mmobj(map)));
    pushMMListItem(list, toMMObject(hi8));
    pushMMListItem(list, toMMObject(v7));

    MMPacker packer = allocMMPacker(&pool);

    pack_mmobj(0, list, packer);
    pack_mmobj(1, list, packer);
    pack_varint(2, 99, packer);

    uint len;
    uint8 *data = dyb_get_data_before_current_position(packer->dyb, &len);
    PRINTF_HEXMEM_TO_TARGET(fprintf, stdout, data, len, 256);

    MMUnpacker unpacker = allocMMUnpackerWithData(&pool, data, len);
    register_all_mmo_ext_to_unpacker(unpacker);

    MMList cloned_list = toMMList(unpack_mmobj(0, unpacker));     // auto release
    MMList cloned_list2 = toMMList(unpack_mmobj(1, unpacker));     // auto release

    if (sizeOfMMList(list) != sizeOfMMList(cloned_list)) {
        plat_io_printf_err("Why are they different?(%u!=%u)\n", sizeOfMMList(list), sizeOfMMList(cloned_list));
    }

    if (compare_mmobjs(list, cloned_list) != 0) {
        plat_io_printf_err("Why are they different?\n");
    }

    if (cloned_list != cloned_list2) {
        plat_io_printf_err("Why are they different?(%p!=%p)\n", cloned_list, cloned_list2);
    }

    if (!are_equal_mmobjs(cloned_list, cloned_list2)) {
        plat_io_printf_err("Why are they different?\n");
    }

    int64 vari = unpack_varint(2, unpacker);
    if (vari != 99) {
        plat_io_printf_err("What is this?(%lld != %d)\n", vari, 99);
    }

    //release_mmobj(cloned_list);
    release_mmobj(unpacker);
    release_mmobj(packer);
    release_mmobj(list);

    mgn_mem_release_unused(&pool);

    if (mgn_mem_count_of_mem(&pool) != 0)
    {
        plat_io_printf_err("Memory leak? (%zu)\n", mgn_mem_count_of_mem(&pool));
    }

    mgn_mem_release_all(&pool);

    plat_io_printf_dbg("===== unit_test_mmobj - END =====\n");
}