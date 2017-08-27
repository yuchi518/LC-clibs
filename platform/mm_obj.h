//
// Created by Yuchi Chen on 2017/8/27.
//

#ifndef PROC_LA_MM_OBJ_H_H
#define PROC_LA_MM_OBJ_H_H

#include "plat_type.h"
#include "plat_mgn_mem.h"

#ifndef container_of
#ifdef __GNUC__
#define member_type(type, member) __typeof__ (((type *)0)->member)
#else
#define member_type(type, member) const void
#endif

#define container_of(ptr, type) ((type *)( \
    (char *)(member_type(type, is_a) *){ ptr } - ((size_t) &((type *)0)->is_a) ))
#endif

typedef struct mmObj {
    uint mmid;
    void* pool;
    void* last_child;
} *mmObj;

typedef struct mmBase {
    void* super;
    void* init;
    void* destroy;
} *mmBase;

#define MMRootObject(cid, stru_name, fn_init, fn_destroy)                           \
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
static inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                \
                                        void* last_child) {                         \
    MM__##stru_name* ptr = p;                                                       \
    ptr->isa.pool = pool;                                                           \
    ptr->isa.mmid = cid;                                                            \
    ptr->isa.last_child = last_child;                                               \
    ptr->isb.super = null;                                                          \
    ptr->isb.init = fn_init;                                                        \
    ptr->isb.destroy = fn_destroy;                                                  \
    if (fn_init != null && fn_init(&ptr->iso) == null) {                            \
        return null;                                                                \
    }                                                                               \
    return p;                                                                       \
}                                                                                   \
                                                                                    \
static inline mmObj alloc##stru_name(mgn_memory_pool* pool) {                       \
    MM__##stru_name* ptr =                                                          \
          mgn_mem_alloc(pool, sizeof(MM__##stru_name));                             \
    if (init_##stru_name(pool, ptr, &ptr->iso) == null) {                           \
        mgn_mem_release(pool, ptr);                                                 \
        return null;                                                                \
    }                                                                               \
    return (void*)ptr;                                                              \
}

#define MMSubObject(cid, stru_name, sup_name, fn_init, fn_destroy)                  \
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
static inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                \
                                        void* last_child) {                         \
    MM__##stru_name* ptr = p;                                                       \
    ptr->isb.super = pos_o_##sup_name(p);                                           \
    ptr->isb.init = fn_init;                                                        \
    ptr->isb.destroy = fn_destroy;                                                  \
    if (init_##sup_name(pool, p, last_child) == null) {                             \
        return null;                                                                \
    }                                                                               \
    if (fn_init != null && fn_init(&ptr->iso) == null) {                            \
        return null;                                                                \
    }                                                                               \
    return p;                                                                       \
}                                                                                   \
                                                                                    \
static inline mmObj alloc##stru_name(mgn_memory_pool* pool) {                       \
    MM__##stru_name* ptr =                                                          \
          mgn_mem_alloc(pool, sizeof(MM__##stru_name));                             \
    if (init_##stru_name(pool, ptr, &ptr->iso) == null) {                           \
        mgn_mem_release(pool, ptr);                                                 \
        return null;                                                                \
    }                                                                               \
    return (void*)ptr;                                                              \
}

/// ===== Root =====
typedef struct MMRoot {

}*MMRoot;

static inline MMRoot initMMRoot(MMRoot obj) {
    return obj;
}

static inline void destroyMMRoot(MMRoot obj) {

}

MMRootObject(0, MMRoot, initMMRoot, destroyMMRoot);

/// ====== Child =====
typedef struct MMChild {

}*MMChild;

static inline MMChild initMMChild(MMChild obj) {
    return obj;
}

static inline void destroyMMChild(MMChild obj) {

}

MMSubObject(1, MMChild, MMRoot, initMMChild, destroyMMChild);

/// ===== Son =====
typedef struct MMSon {

}*MMSon;

static inline MMSon initMMSon(MMSon obj) {
    return obj;
}

static inline void destroyMMSon(MMSon obj) {

}

MMSubObject(1, MMSon, MMChild, initMMSon, destroyMMSon);




#endif //PROC_LA_MM_OBJ_H_H
