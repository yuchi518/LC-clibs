//
// Created by Yuchi Chen on 2017/8/27.
//

#ifndef PROC_LA_MM_OBJ_H_H
#define PROC_LA_MM_OBJ_H_H

#include "plat_type.h"
#include "plat_mgn_mem.h"

/*#ifndef container_of_base
#ifdef __GNUC__
#define member_type(type, member) __typeof__ (((type *)0)->member)
#else
#define member_type(type, member) const void
#endif

#define container_of_base(ptr, type) ((type *)( \
    (void*)(member_type(type, isb) *){ ptr } - ((size_t) &((type *)0)->isb) ))
#define container_of_stru(ptr, type) ((type *)( \
    (void*)(member_type(type, iso) *){ ptr } - ((size_t) &((type *)0)->iso) ))
#endif*/

struct mmBase;
typedef struct mmObj {
    uint _oid;
    mgn_memory_pool* _pool;
} *mmObj;

typedef void* Unpacker;
typedef void* Packer;

typedef struct mmBase {
    struct mmBase* pre_base;                                                // parent's base address or last child's base address
    void (*destroy)(struct mmBase* base);
    void* (*find)(struct mmBase* base, uint mmid, uint utilid);             // for cast
    mmObj (*find_obj)(struct mmBase* base);                                 // mmobj address is used for memory management.
    const char* (*name)(void);
    void (*hash)(struct mmBase* base, void** key, uint* key_len);
    void (*pack)(struct mmBase* base, Packer pkr);
} *mmBase;

plat_inline mmBase __stru2base(void* stru) {
    struct {
        struct mmObj a;
        struct mmBase b;
        struct {
            uint8 _dummy;
        } c;
    } obj;
    return (mmBase)((/*(void*)*/stru) - (((uint)&obj.c) - ((uint)&obj.b)));
}

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
plat_inline void hash_##stru_name(mmBase base, void** key, uint* key_len) {                     \
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
    ptr->isb.pre_base = last_child_base;                                                        \
    ptr->isb.destroy = destroy_##stru_name;                                                     \
    ptr->isb.find = find_##stru_name;                                                           \
    ptr->isb.find_obj = find_obj_##stru_name;                                                   \
    ptr->isb.name = name_##stru_name;                                                           \
    ptr->isb.hash = hash_##stru_name;                                                           \
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
    mmBase base = __stru2base(stru);                                                            \
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
    ptr->isb.hash = null;                                                                       \
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
    mmBase base = __stru2base(stru);                                                            \
    return (struct stru_name*)(base->find(base, (oid), (oid)));                                 \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* baseTo##stru_name(mmBase base) {                                  \
    return (struct stru_name*)(base->find(base, (oid), (oid)));                                 \
}                                                                                               \


plat_inline void* __retain_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    return (mgn_mem_retain(obj->_pool, obj) == null)?null:stru;
}
#define retain_mmobj(stru) ((typeof(stru))__retain_mmobj(stru))

plat_inline void __trigger_release_flow(void* mem) {
    struct {
        struct mmObj isa;
        struct mmBase isb;
    } *ptr;
    ptr = mem;                                          /*it's first obj*/
    ptr->isb.pre_base->destroy(ptr->isb.pre_base);      /*call last child destroy*/
}

plat_inline void release_mmobj(void* stru) {
    if (stru == null) return;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);                   /*find first obj*/
    mgn_mem_release_w_cb(obj->_pool, obj, __trigger_release_flow);
}

plat_inline void* __autorelease_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    mgn_mem_autorelease_w_cb(obj->_pool, obj, __trigger_release_flow);
    return stru;
}
#define autorelease_mmobj(stru) ((typeof(stru))__autorelease_mmobj(stru))

plat_inline uint retain_count_of_mmobj(void* stru) {
    if (stru == null) return 0;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    return mgn_mem_retained_count(obj->_pool, obj);
}

plat_inline const char* name_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    return base->name();
}

plat_inline mgn_memory_pool* pool_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    return obj->_pool;
}

plat_inline uint oid_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    return obj->_oid;
}

static void set_hash_for_mmobj(void* stru, void (*hash)(mmBase /*base*/, void** /*key*/, uint* /*key_len*/)) {
    if (stru == null) return;
    mmBase base = __stru2base(stru);
    base->hash = hash;
}

plat_inline mmBase find_first_base(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    struct {
        struct mmObj isa;
        struct mmBase isb;
    } *ptr;
    ptr = (void*)base->find_obj(base);
    base = &ptr->isb;

    return base;
}

plat_inline void hash_of_mmobj(void* stru, void** key, void* key_len) {
    if (stru == null || key == null || key_len == null) return;
    mmBase base = find_first_base(stru)->pre_base;       // use first one to find last one
    while(base->hash == null) {
        // search last one hash implementation
        base = base->pre_base;
    }

    base->hash(base, key, key_len);
}

plat_inline const char* last_name_of_mmobj(void* stru) {
    if (stru == null) return null;
    return find_first_base(stru)->pre_base->name();
}

plat_inline bool is_oid(void* stru, uint oid) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    return base->find(base, oid, oid)==null?true:false;
}

plat_inline void pack(void* stru, Packer pkr) {
    if (stru == null) return;
    mmBase base = find_first_base(stru)->pre_base;       // use first one to find last one
    base->pack(base, pkr);
}

plat_inline void* unpack(Unpacker unpkr) {
    //mgn_memory_pool* pool = pool_of_mmobj(unpkr);

    return null;
}

/// samples
/// ================ MMObj ===========
#define MMOBJ_ROOT              (0xFFFF0000)
#define MMOBJ_CHILD             (0xFFFF0001)
#define MMOBJ_SON               (0xFFFF0002)
/// ===== Root =====

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
