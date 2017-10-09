//
// Created by Yuchi Chen on 2017/8/27.
//

#ifndef PROC_LA_MM_OBJ_H_H
#define PROC_LA_MM_OBJ_H_H

#include "plat_type.h"
#include "plat_mgn_mem.h"
#include "plat_string.h"

typedef struct mmObj_fn {
    const char *name;          /* key */
    void* fn;
    UT_hash_handle hh;         /* makes this structure hashable */
} *mmObj_fn;

typedef struct mmObj {
    uint _oid;
    mgn_memory_pool* _pool;
    mmObj_fn _fns;
} *mmObj;

typedef void* Unpacker;
typedef void* Packer;

typedef struct mmBase {
    struct mmBase* pre_base;                                                // parent's base address or last child's base address
    void (*destroy)(struct mmBase* base);
    void* (*find)(struct mmBase* base, uint mmid, uint utilid);             // for cast
    mmObj (*find_obj)(struct mmBase* base);                                 // mmobj address is used for memory management.
    const char* (*name)(void);
    void (*pack)(struct mmBase* base, Packer pkr);
} *mmBase;

#define MMRootObject(oid, stru_name, fn_init, fn_destroy, fn_pack)                              \
                                                                                                \
struct MM__##stru_name {                                                                        \
    struct mmObj isa;                                                                           \
    struct mmBase isb;                                                                          \
    struct stru_name iso;                                                                       \
};                                                                                              \
                                                                                                \
plat_inline mmBase pos_b_##stru_name(void* ptr) {                                               \
               return (mmBase) &((struct MM__##stru_name*)ptr)->isb; }                          \
plat_inline struct stru_name* pos_o_##stru_name(void* ptr) {                                    \
               return (struct stru_name*) &((struct MM__##stru_name*)ptr)->iso;  }              \
                                                                                                \
plat_inline void* find_##stru_name(mmBase base, uint mmid, uint untilid) {                      \
    if (mmid == (oid)) {                                                                        \
        struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb; \
        return &ptr->iso;                                                                       \
    } else if (mmid == untilid) {                                                               \
        return base->pre_base->find(base->pre_base, mmid, (oid));                               \
    } else if ((oid) != untilid) {                                                              \
        return base->pre_base->find(base->pre_base, mmid, untilid);                             \
    }                                                                                           \
    return null;                                                                                \
}                                                                                               \
                                                                                                \
plat_inline mmObj find_obj_##stru_name(mmBase base) {                                           \
    struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb;     \
    return (void*)ptr;                                                                          \
}                                                                                               \
                                                                                                \
plat_inline const char* name_##stru_name(void) {                                                \
    static const char* _name = "" #stru_name;                                                   \
    return _name;                                                                               \
}                                                                                               \
                                                                                                \
plat_inline void destroy_##stru_name(mmBase base) {                                             \
    void (*destroy_impl)(struct stru_name* base) = fn_destroy;                                  \
    if (destroy_impl != null) {                                                                 \
        struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb; \
        destroy_impl(&ptr->iso);                                                                \
    }                                                                                           \
}                                                                                               \
                                                                                                \
plat_inline void hash_##stru_name(void* stru, void** key, uint* key_len) {                      \
    mmBase base = stru - (uint)&((struct MM__##stru_name*)0)->iso                               \
                       + (uint)&((struct MM__##stru_name*)0)->isb;                              \
    if (key) *key = &(base->pre_base);                                                          \
    if (key_len) *key_len = sizeof(base->pre_base);                                             \
}                                                                                               \
                                                                                                \
plat_inline void pack_##stru_name(mmBase base, Packer pkr);                                     \
plat_inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                              \
                                        mmBase last_child_base, uint mmid,                      \
                                                        Unpacker unpkr) {                       \
    struct MM__##stru_name* ptr = p;                                                            \
    ptr->isa._pool = pool;                                                                      \
    ptr->isa._oid = mmid;                                                                       \
    ptr->isa._fns = null;                                                                       \
    ptr->isb.pre_base = last_child_base;                                                        \
    ptr->isb.destroy = destroy_##stru_name;                                                     \
    ptr->isb.find = find_##stru_name;                                                           \
    ptr->isb.find_obj = find_obj_##stru_name;                                                   \
    ptr->isb.name = name_##stru_name;                                                           \
    set_hash_for_mmobj(&ptr->iso, hash_##stru_name);                                            \
    ptr->isb.pack = pack_##stru_name;                                                           \
    struct stru_name* (*init_impl)(struct stru_name*, Unpacker) = fn_init;                      \
    if (init_impl != null && init_impl(&ptr->iso, unpkr) == null) {                             \
        return null;                                                                            \
    }                                                                                           \
    return p;                                                                                   \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* alloc##stru_name(mgn_memory_pool* pool) {                         \
    struct MM__##stru_name* ptr =                                                               \
          mgn_mem_alloc(pool, sizeof(struct MM__##stru_name));                                  \
    if (init_##stru_name(pool, ptr, &ptr->isb, (oid), null) == null) {                          \
        mgn_mem_release(pool, ptr);                                                             \
        return null;                                                                            \
    }                                                                                           \
    return &ptr->iso;                                                                           \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* unpack##stru_name(mgn_memory_pool* pool,                          \
                                                        Unpacker unpkr) {                       \
    struct MM__##stru_name* ptr =                                                               \
          mgn_mem_alloc(pool, sizeof(struct MM__##stru_name));                                  \
    if (init_##stru_name(pool, ptr, &ptr->isb, (oid), unpkr) == null) {                         \
        mgn_mem_release(pool, ptr);                                                             \
        return null;                                                                            \
    }                                                                                           \
    return &ptr->iso;                                                                           \
}                                                                                               \
                                                                                                \
plat_inline void pack_##stru_name(mmBase base, Packer pkr) {                                    \
    struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb;     \
    void (*pack_impl)(struct stru_name*, Packer) = fn_pack;                                     \
    if (pack_impl != null) pack_impl(&ptr->iso, pkr);                                           \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* to##stru_name(void* stru) {                                       \
    if (stru == null) return null;                                                              \
    mmBase base = base_of_mmobj(stru);                                                          \
    return (struct stru_name*)(base->find(base, (oid), (oid)));                                 \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* baseTo##stru_name(mmBase base) {                                  \
    return (struct stru_name*)(base->find(base, (oid), (oid)));                                 \
}                                                                                               \


#define MMSubObject(oid, stru_name, sup_name, fn_init, fn_destroy, fn_pack)                     \
                                                                                                \
struct MM__##stru_name {                                                                        \
    struct MM__##sup_name iss;                                                                  \
    struct mmBase isb;                                                                          \
    struct stru_name iso;                                                                       \
};                                                                                              \
                                                                                                \
plat_inline mmBase pos_b_##stru_name(void* ptr) {                                               \
               return (mmBase) &((struct MM__##stru_name*)ptr)->isb;  }                         \
plat_inline struct stru_name* pos_o_##stru_name(void* ptr) {                                    \
               return (struct stru_name*) &((struct MM__##stru_name*)ptr)->iso;  }              \
                                                                                                \
plat_inline void* find_##stru_name(mmBase base, uint mmid, uint untilid) {                      \
    if (mmid == (oid)) {                                                                        \
        struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb; \
        return &ptr->iso;                                                                       \
    } else if (mmid == untilid) {                                                               \
        return base->pre_base->find(base->pre_base, mmid, (oid));                               \
    } else if ((oid) != untilid) {                                                              \
        return base->pre_base->find(base->pre_base, mmid, untilid);                             \
    }                                                                                           \
    return null;                                                                                \
}                                                                                               \
                                                                                                \
plat_inline mmObj find_obj_##stru_name(mmBase base) {                                           \
    struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb;     \
    return (void*)ptr;                                                                          \
}                                                                                               \
                                                                                                \
plat_inline const char* name_##stru_name(void) {                                                \
    static const char* _name = "" #stru_name;                                                   \
    return _name;                                                                               \
}                                                                                               \
                                                                                                \
plat_inline void destroy_##stru_name(mmBase base) {                                             \
    /* destroy self first, then destroy super */                                                \
    struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb;     \
    void (*destroy_impl)(struct stru_name* base) = fn_destroy;                                  \
    if (destroy_impl != null) {                                                                 \
        destroy_impl(&ptr->iso);                                                                \
    }                                                                                           \
    ptr->isb.pre_base->destroy(ptr->isb.pre_base);                                              \
}                                                                                               \
                                                                                                \
plat_inline void pack_##stru_name(mmBase base, Packer pkr);                                     \
plat_inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                              \
                                        mmBase last_child_base, uint mmid,                      \
                                                        Unpacker unpkr) {                       \
    /* init super first, then init self */                                                      \
    if (init_##sup_name(pool, p, last_child_base, mmid, unpkr) == null) {                       \
        return null;                                                                            \
    }                                                                                           \
    struct MM__##stru_name* ptr = p;                                                            \
    ptr->isb.pre_base = pos_b_##sup_name(p);                                                    \
    ptr->isb.destroy = destroy_##stru_name;                                                     \
    ptr->isb.find = find_##stru_name;                                                           \
    ptr->isb.find_obj = find_obj_##stru_name;                                                   \
    ptr->isb.name = name_##stru_name;                                                           \
    ptr->isb.pack = pack_##stru_name;                                                           \
    struct stru_name* (*init_impl)(struct stru_name*, Unpacker) = fn_init;                      \
    if (init_impl != null && init_impl(&ptr->iso, unpkr) == null) {                             \
        /*Init fail, destroy super.*/                                                           \
        void (*destroy_super_impl)(mmBase base) = ptr->isb.pre_base->destroy;                   \
        destroy_super_impl(ptr->isb.pre_base);                                                  \
        return null;                                                                            \
    }                                                                                           \
    return p;                                                                                   \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* alloc##stru_name(mgn_memory_pool* pool) {                         \
    struct MM__##stru_name* ptr =                                                               \
          mgn_mem_alloc(pool, sizeof(struct MM__##stru_name));                                  \
    if (init_##stru_name(pool, ptr, &ptr->isb, (oid), null) == null) {                          \
        mgn_mem_release(pool, ptr);                                                             \
        return null;                                                                            \
    }                                                                                           \
    return &ptr->iso;                                                                           \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* unpack##stru_name(mgn_memory_pool* pool,                          \
                                                        Unpacker unpkr) {                       \
    struct MM__##stru_name* ptr =                                                               \
          mgn_mem_alloc(pool, sizeof(struct MM__##stru_name));                                  \
    if (init_##stru_name(pool, ptr, &ptr->isb, (oid), unpkr) == null) {                         \
        mgn_mem_release(pool, ptr);                                                             \
        return null;                                                                            \
    }                                                                                           \
    return &ptr->iso;                                                                           \
}                                                                                               \
                                                                                                \
plat_inline void pack_##stru_name(mmBase base, Packer pkr) {                                    \
    struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb;     \
    pack_##sup_name(ptr->isb.pre_base, pkr);                                                    \
    void (*pack_impl)(struct stru_name*, Packer) = fn_pack;                                     \
    if (pack_impl != null) pack_impl(&ptr->iso, pkr);                                           \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* to##stru_name(void* stru) {                                       \
    if (stru == null) return null;                                                              \
    mmBase base = base_of_mmobj(stru);                                                          \
    return (struct stru_name*)(base->find(base, (oid), (oid)));                                 \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* baseTo##stru_name(mmBase base) {                                  \
    return (struct stru_name*)(base->find(base, (oid), (oid)));                                 \
}                                                                                               \

plat_inline mmBase __base_of_mmobj(void* stru) {
    struct {
        struct mmObj a;
        struct mmBase b;
        struct {
            uint8 _dummy;
        } c;
    } obj;
    return (mmBase)((/*(void*)*/stru) - (((uint)&obj.c) - ((uint)&obj.b)));
}
#define base_of_mmobj(stru) __base_of_mmobj(stru)

plat_inline mmBase __base_of_first_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    struct {
        struct mmObj isa;
        struct mmBase isb;
    } *ptr;
    ptr = (void*)base->find_obj(base);
    base = &ptr->isb;

    return base;
}
#define base_of_first_mmobj(stru) __base_of_first_mmobj(stru)

plat_inline mgn_memory_pool* __pool_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->find_obj(base);
    return obj->_pool;
}
#define pool_of_mmobj(stru) __pool_of_mmobj(stru)

plat_inline void* __retain_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->find_obj(base);
    return (mgn_mem_retain(obj->_pool, obj) == null)?null:stru;
}
#define retain_mmobj(stru) ((typeof(stru))__retain_mmobj(stru))

plat_inline void __trigger_release_mmobj(void* mem) {
    struct {
        struct mmObj isa;
        struct mmBase isb;
    } *ptr;
    ptr = mem;                                          /*it's first obj*/
    mmObj_fn fncb, tmp;
    HASH_ITER(hh, ptr->isa._fns, fncb, tmp) {
        HASH_DEL(ptr->isa._fns, fncb);
        mgn_mem_release(ptr->isa._pool, fncb);
    }
    ptr->isb.pre_base->destroy(ptr->isb.pre_base);      /*call last child destroy*/

}

plat_inline void __release_mmobj(void* stru) {
    if (stru == null) return;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->find_obj(base);                   /*find first obj*/
    mgn_mem_release_w_cb(obj->_pool, obj, __trigger_release_mmobj);
}
#define release_mmobj(stru) __release_mmobj(stru)

plat_inline void* __autorelease_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->find_obj(base);
    mgn_mem_autorelease_w_cb(obj->_pool, obj, __trigger_release_mmobj);
    return stru;
}
#define autorelease_mmobj(stru) ((typeof(stru))__autorelease_mmobj(stru))

plat_inline uint __retain_count_of_mmobj(void* stru) {
    if (stru == null) return 0;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->find_obj(base);
    return (uint)mgn_mem_retained_count(obj->_pool, obj);
}
#define retain_count_of_mmobj(stru) __retain_count_of_mmobj(stru)

plat_inline void* __set_function_for_mmobj(void* stru, const char* fn_type_name, void* fn) {
    if (stru == null) return 0;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->find_obj(base);
    mgn_memory_pool* pool = pool_of_mmobj(stru);
    mmObj_fn fncb = null;
    void* pre_fn = null;
    HASH_FIND_STR(obj->_fns, fn_type_name, fncb);
    if (fncb) {
        pre_fn = fncb->fn;
        HASH_DEL(obj->_fns, fncb);
    } else {
        fncb = mgn_mem_alloc(pool, sizeof(*fncb));
        fncb->name = fn_type_name;
    }
    fncb->fn = fn;
    HASH_ADD_KEYPTR( hh, obj->_fns, fncb->name, plat_cstr_length(fncb->name), fncb );
    return pre_fn;
}
#define set_function_for_mmobj(stru, fn_type, fn)  (fn_type)__set_function_for_mmobj(stru, "" #fn_type, fn)

plat_inline void* __get_function_for_mmobj(void* stru, const char* fn_type_name) {
    if (stru == null) return 0;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->find_obj(base);
    mmObj_fn fncb;
    HASH_FIND_STR(obj->_fns, fn_type_name, fncb);
    return fncb?fncb->fn:null;
}
#define get_function_for_mmobj(stru, fn_type)  (fn_type)__get_function_for_mmobj(stru, "" #fn_type)

plat_inline const char* __name_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    return base->name();
}
#define name_of_mmobj(stru) __name_of_mmobj(stru)

plat_inline const char* __name_of_last_mmobj(void* stru) {
    if (stru == null) return null;
    return base_of_first_mmobj(stru)->pre_base->name();
}
#define name_of_last_mmobj(stru) __name_of_last_mmobj(stru)

plat_inline uint __oid_of_last_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->find_obj(base);
    return obj->_oid;
}
#define oid_of_last_mmobj(stru) __oid_of_last_mmobj(stru)

typedef void (*mmobj_hash)(void* /*base*/, void** /*key*/, uint* /*key_len*/);
plat_inline void __set_hash_for_mmobj(void* stru, mmobj_hash hash) {
    if (stru == null) return;
    set_function_for_mmobj(stru, mmobj_hash, hash);
}
#define set_hash_for_mmobj(stru, hash) __set_hash_for_mmobj(stru, hash)

plat_inline void __hash_of_mmobj(void* stru, void** key, void* key_len) {
    if (stru == null || key == null || key_len == null) return;
    mmobj_hash hash = get_function_for_mmobj(stru, mmobj_hash);
    if (hash) hash(stru, key, key_len);
}
#define hash_of_mmobj(stru, key, key_len) __hash_of_mmobj(stru, key, key_len)

plat_inline bool __is_mmobj_kind_of_oid(void* stru, uint oid) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    return base->find(base, oid, oid)==null?true:false;
}
#define is_mmobj_kind_of_oid(stru, oid) __is_mmobj_kind_of_oid(stru, oid)

plat_inline void __pack_mmobj(void* stru, Packer pkr) {
    if (stru == null) return;
    mmBase base = base_of_first_mmobj(stru)->pre_base;       // use first one to find last one
    base->pack(base, pkr);
}
#define pack_mmobj(stru, pkr) __pack_mmobj(stru, pkr)

plat_inline void* __unpack_mmobj(Unpacker unpkr) {
    //mgn_memory_pool* pool = pool_of_mmobj(unpkr);

    return null;
}
#define unpack_mmobj(unpkr) __unpack_mmobj(unpkr)

/*
 * ============= Samples ================
 * Three objects: MMObj, MMChild, MMSon
 *
 * ======================================
 */
// ===========  MMObj ===========
#define MMOBJ_ROOT              (0xFFFF0000)
#define MMOBJ_CHILD             (0xFFFF0001)
#define MMOBJ_SON               (0xFFFF0002)
// ===== Root =====

typedef struct MMRoot {

}*MMRoot;

plat_inline MMRoot initMMRoot(MMRoot obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMRoot(MMRoot obj) {

}

MMRootObject(MMOBJ_ROOT, MMRoot, initMMRoot, destroyMMRoot, null);

/// ====== Child =====
typedef struct MMChild {

}*MMChild;

plat_inline MMChild initMMChild(MMChild obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMChild(MMChild obj) {

}

MMSubObject(MMOBJ_CHILD, MMChild, MMRoot, initMMChild, destroyMMChild, null);

/// ===== Son =====
typedef struct MMSon {

}*MMSon;

plat_inline MMSon initMMSon(MMSon obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMSon(MMSon obj) {

}

MMSubObject(MMOBJ_SON, MMSon, MMChild, initMMSon, destroyMMSon, null);




#endif //PROC_LA_MM_OBJ_H_H
