//
// Created by Yuchi Chen on 2017/8/27.
//

#ifndef PROC_LA_MM_OBJ_H_H
#define PROC_LA_MM_OBJ_H_H

#include "plat_type.h"
#include "plat_mgn_mem.h"

#ifndef container_of_base
#ifdef __GNUC__
#define member_type(type, member) __typeof__ (((type *)0)->member)
#else
#define member_type(type, member) const void
#endif

#define container_of_base(ptr, type) ((type *)( \
    (char *)(member_type(type, isb) *){ ptr } - ((size_t) &((type *)0)->isb) ))
#define container_of_stru(ptr, type) ((type *)( \
    (char *)(member_type(type, iso) *){ ptr } - ((size_t) &((type *)0)->iso) ))
#endif

struct mmBase;
typedef struct mmObj {
    uint _oid;
    mgn_memory_pool* _pool;
} *mmObj;

typedef struct mmBase {
    struct mmBase* pre_base;                                                // parent's base address or last child's base address
    void (*destroy)(struct mmBase* base);
    void* (*find)(struct mmBase* base, uint mmid, uint utilid);             // for cast
    mmObj (*find_obj)(struct mmBase* base);                                 // mmobj address is used for memory management.
    const char* (*name)(void);
    void (*hash)(struct mmBase* base, void** key, uint* key_len);
} *mmBase;

static inline mmBase __stru2base(void* stru) {
    struct {
        struct mmObj a;
        struct mmBase b;
        struct {
            uint8 _dummp;
        } c;
    } obj;
    return (mmBase)(((void*)stru) - (((uint)&obj.c) - ((uint)&obj.b)));
}

#define MMRootObject(oid, stru_name, fn_init, fn_destroy)                           \
typedef struct stru_name* (*p_init##stru_name)(struct stru_name* obj);              \
typedef void (*p_destroy##stru_name)(struct stru_name* obj);                        \
                                                                                    \
typedef struct {                                                                    \
    struct mmObj isa;                                                               \
    struct mmBase isb;                                                              \
    struct stru_name iso;                                                           \
} MM__##stru_name;                                                                  \
static inline mmBase pos_b_##stru_name(void* ptr) {                                 \
               return ((mmBase) &((MM__##stru_name*)ptr)->isb);  }                  \
static inline struct stru_name* pos_o_##stru_name(void* ptr) {                      \
               return ((struct stru_name*) &((MM__##stru_name*)ptr)->iso);  }       \
                                                                                    \
static inline void* find_##stru_name(mmBase ptr_base, uint mmid, uint untilid) {    \
    if (mmid == oid) {                                                              \
        MM__##stru_name* p = container_of_base(ptr_base, MM__##stru_name);          \
        return &p->iso;                                                             \
    } else if (mmid == untilid) {                                                   \
        return ptr_base->pre_base->find(ptr_base->pre_base, mmid, oid);             \
    } else if (oid != untilid) {                                                    \
        return ptr_base->pre_base->find(ptr_base->pre_base, mmid, untilid);         \
    }                                                                               \
    return null;                                                                    \
}                                                                                   \
                                                                                    \
static inline mmObj find_obj_##stru_name(mmBase base) {                             \
    return (void*)container_of_base(base, MM__##stru_name);                         \
}                                                                                   \
                                                                                    \
static inline const char* name_##stru_name(void) {                                  \
    static const char* _name = "" #stru_name;                                       \
    return _name;                                                                   \
}                                                                                   \
                                                                                    \
static inline void destroy_##stru_name(mmBase ptr_base) {                           \
    if (fn_destroy != null) {                                                       \
        MM__##stru_name* ptr = container_of_base(ptr_base, MM__##stru_name);        \
        fn_destroy(&ptr->iso);                                                      \
    }                                                                               \
}                                                                                   \
                                                                                    \
static inline void hash_##stru_name(mmBase base, void** key, uint* key_len) {       \
    if (key) *key = &(base->pre_base);                                              \
    if (key_len) *key_len = sizeof(base->pre_base);                                 \
}                                                                                   \
                                                                                    \
static inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                \
                                        mmBase last_child_base, uint mmid) {        \
    MM__##stru_name* ptr = p;                                                       \
    ptr->isa._pool = pool;                                                          \
    ptr->isa._oid = mmid;                                                           \
    ptr->isb.pre_base = last_child_base;                                            \
    ptr->isb.destroy = destroy_##stru_name;                                         \
    ptr->isb.find = find_##stru_name;                                               \
    ptr->isb.find_obj = find_obj_##stru_name;                                       \
    ptr->isb.name = name_##stru_name;                                               \
    ptr->isb.hash = hash_##stru_name;                                               \
    if (fn_init != null && fn_init(&ptr->iso) == null) {                            \
        return null;                                                                \
    }                                                                               \
    return p;                                                                       \
}                                                                                   \
                                                                                    \
static inline struct stru_name* alloc##stru_name(mgn_memory_pool* pool) {           \
    MM__##stru_name* ptr =                                                          \
          mgn_mem_alloc(pool, sizeof(MM__##stru_name));                             \
    if (init_##stru_name(pool, ptr, &ptr->isb, oid) == null) {                      \
        mgn_mem_release(pool, ptr);                                                 \
        return null;                                                                \
    }                                                                               \
    return &ptr->iso;                                                               \
}                                                                                   \
                                                                                    \
static inline struct stru_name* to##stru_name(void* stru) {                         \
    mmBase base = __stru2base(stru);                                                \
    return (struct stru_name*)(base->find(base, oid, oid));                         \
}                                                                                   \
                                                                                    \
static inline struct stru_name* baseTo##stru_name(mmBase base) {                    \
    return (struct stru_name*)(base->find(base, oid, oid));                         \
}

#define MMSubObject(oid, stru_name, sup_name, fn_init, fn_destroy)                  \
typedef struct stru_name* (*p_init##stru_name)(struct stru_name* obj);              \
typedef void (*p_destroy##stru_name)(struct stru_name* obj);                        \
                                                                                    \
typedef struct {                                                                    \
    MM__##sup_name iss;                                                             \
    struct mmBase isb;                                                              \
    struct stru_name iso;                                                           \
} MM__##stru_name;                                                                  \
                                                                                    \
static inline mmBase pos_b_##stru_name(void* ptr) {                                 \
               return ((mmBase) &((MM__##stru_name*)ptr)->isb);  }                  \
static inline struct stru_name* pos_o_##stru_name(void* ptr) {                      \
               return ((struct stru_name*) &((MM__##stru_name*)ptr)->iso);  }       \
                                                                                    \
static inline void* find_##stru_name(mmBase ptr_base, uint mmid, uint untilid) {    \
    if (mmid == oid) {                                                              \
        MM__##stru_name*p = container_of_base(ptr_base, MM__##stru_name);           \
        return &p->iso;                                                             \
    } else if (mmid == untilid) {                                                   \
        return ptr_base->pre_base->find(ptr_base->pre_base, mmid, oid);             \
    } else if (oid != untilid) {                                                    \
        return ptr_base->pre_base->find(ptr_base->pre_base, mmid, untilid);         \
    }                                                                               \
    return null;                                                                    \
}                                                                                   \
                                                                                    \
static inline mmObj find_obj_##stru_name(mmBase base) {                             \
    return (void*)container_of_base(base, MM__##stru_name);                         \
}                                                                                   \
                                                                                    \
static inline const char* name_##stru_name(void) {                                  \
    static const char* _name = "" #stru_name;                                       \
    return _name;                                                                   \
}                                                                                   \
                                                                                    \
static inline void destroy_##stru_name(mmBase ptr_base) {                           \
    if (fn_destroy != null) {                                                       \
        MM__##stru_name* ptr = container_of_base(ptr_base, MM__##stru_name);        \
        ptr->isb.pre_base->destroy(ptr->isb.pre_base);                              \
        fn_destroy(&ptr->iso);                                                      \
    }                                                                               \
}                                                                                   \
                                                                                    \
static inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                \
                                        mmBase last_child_base, uint mmid) {        \
    if (init_##sup_name(pool, p, last_child_base, mmid) == null) {                  \
        return null;                                                                \
    }                                                                               \
    MM__##stru_name* ptr = p;                                                       \
    ptr->isb.pre_base = pos_b_##sup_name(p);                                        \
    ptr->isb.destroy = destroy_##stru_name;                                         \
    ptr->isb.find = find_##stru_name;                                               \
    ptr->isb.find_obj = find_obj_##stru_name;                                       \
    ptr->isb.name = name_##stru_name;                                               \
    ptr->isb.hash = null;                                                           \
    if (fn_init != null && fn_init(&ptr->iso) == null) {                            \
        return null;                                                                \
    }                                                                               \
    return p;                                                                       \
}                                                                                   \
                                                                                    \
static inline struct stru_name* alloc##stru_name(mgn_memory_pool* pool) {           \
    MM__##stru_name* ptr =                                                          \
          mgn_mem_alloc(pool, sizeof(MM__##stru_name));                             \
    if (init_##stru_name(pool, ptr, &ptr->isb, oid) == null) {                      \
        mgn_mem_release(pool, ptr);                                                 \
        return null;                                                                \
    }                                                                               \
    return &ptr->iso;                                                               \
}                                                                                   \
                                                                                    \
static inline struct stru_name* to##stru_name(void* stru) {                         \
    mmBase base = __stru2base(stru);                                                \
    return (struct stru_name*)(base->find(base, oid, oid));                         \
}                                                                                   \
                                                                                    \
static inline struct stru_name* baseTo##stru_name(mmBase base) {                    \
    return (struct stru_name*)(base->find(base, oid, oid));                         \
}

static inline void* __retain_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    return (mgn_mem_retain(obj->_pool, obj) == null)?null:stru;
}
#define retain_mmobj(stru) ((typeof(stru))__retain_mmobj(stru))

static inline void __trigger_release_flow(void* mem) {
    struct obj {
        struct mmObj isa;
        struct mmBase isb;
    } *ptr;
    ptr = (struct obj*)mem;                             /*it's first obj*/
    ptr->isb.pre_base->destroy(ptr->isb.pre_base);      /*call last child destroy*/
}

static inline void release_mmobj(void* stru) {
    if (stru == null) return;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);                   /*find first obj*/
    mgn_mem_release_w_cb(obj->_pool, obj, __trigger_release_flow);
}

static inline void* __autorelease_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    mgn_mem_autorelease_w_cb(obj->_pool, obj, __trigger_release_flow);
    return stru;
}
#define autorelease_mmobj(stru) ((typeof(stru))__autorelease_mmobj(stru))

static inline const char* name_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    return base->name();
}

static inline mgn_memory_pool* pool_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    return obj->_pool;
}

static inline uint oid_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = __stru2base(stru);
    mmObj obj = base->find_obj(base);
    return obj->_oid;
}

static void set_hash_for_mmobj(void* stru, void (*hash)(mmBase base, void** key, uint* key_len)) {
    if (stru == null) return;
    mmBase base = __stru2base(stru);
    base->hash = hash;
}

static inline void hash_of_mmobj(void* stru, void** key, void* key_len) {
    if (stru == null || key == null || key_len == null) return;
    mmBase base = __stru2base(stru);
    struct objobj {
        struct mmObj isa;
        struct mmBase isb;
    };
    base = ((struct objobj*)base->find_obj(base))->isb.pre_base;       // use first one to find last one
    while(base->hash == null) {
        // search last one hash implementation
        base = base->pre_base;
    }

    base->hash(base, key, key_len);
}

/// samples
/// ================ MMObj ===========
#define MMOBJ_ROOT              (0xFFFF0000)
#define MMOBJ_CHILD             (0xFFFF0001)
#define MMOBJ_SON               (0xFFFF0002)
/// ===== Root =====

typedef struct MMRoot {

}*MMRoot;

static inline MMRoot initMMRoot(MMRoot obj) {
    return obj;
}

static inline void destroyMMRoot(MMRoot obj) {

}

MMRootObject(MMOBJ_ROOT, MMRoot, initMMRoot, destroyMMRoot);

/// ====== Child =====
typedef struct MMChild {

}*MMChild;

static inline MMChild initMMChild(MMChild obj) {
    return obj;
}

static inline void destroyMMChild(MMChild obj) {

}

MMSubObject(MMOBJ_CHILD, MMChild, MMRoot, initMMChild, destroyMMChild);

/// ===== Son =====
typedef struct MMSon {

}*MMSon;

static inline MMSon initMMSon(MMSon obj) {
    return obj;
}

static inline void destroyMMSon(MMSon obj) {

}

MMSubObject(MMOBJ_SON, MMSon, MMChild, initMMSon, destroyMMSon);




#endif //PROC_LA_MM_OBJ_H_H
