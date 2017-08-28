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
    (char *)(member_type(type, isb) *){ ptr } - ((size_t) &((type *)0)->isb) ))
#endif

struct mmBase;
typedef struct mmObj {
    void* pool;
} *mmObj;

typedef struct mmBase {
    struct mmBase* pre_base;    // parent's base address or last child's base address
    void* init;
    void* destroy;
    void* (*find)(struct mmBase* ptr_base, uint mmid, uint utilid);
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
static inline void* find_##stru_name(mmBase ptr_base, uint mmid, uint untilid) {    \
    if (mmid == cid) {                                                              \
        MM__##stru_name* p = container_of(ptr_base, MM__##stru_name);               \
        return &p->iso;                                                             \
    } else if (mmid == untilid) {                                                   \
        return ptr_base->pre_base->find(ptr_base->pre_base, mmid, cid);             \
    } else if (cid != untilid) {                                                    \
        return ptr_base->pre_base->find(ptr_base->pre_base, mmid, untilid);         \
    }                                                                               \
    return null;                                                                    \
}                                                                                   \
                                                                                    \
static inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                \
                                        void* last_child_base) {                    \
    MM__##stru_name* ptr = p;                                                       \
    ptr->isa.pool = pool;                                                           \
    ptr->isb.pre_base = last_child_base;                                            \
    ptr->isb.init = fn_init;                                                        \
    ptr->isb.destroy = fn_destroy;                                                  \
    ptr->isb.find = find_##stru_name;                                               \
    if (fn_init != null && fn_init(&ptr->iso) == null) {                            \
        return null;                                                                \
    }                                                                               \
    return p;                                                                       \
}                                                                                   \
                                                                                    \
static inline struct stru_name* alloc##stru_name(mgn_memory_pool* pool) {           \
    MM__##stru_name* ptr =                                                          \
          mgn_mem_alloc(pool, sizeof(MM__##stru_name));                             \
    if (init_##stru_name(pool, ptr, &ptr->isb) == null) {                           \
        mgn_mem_release(pool, ptr);                                                 \
        return null;                                                                \
    }                                                                               \
    return &ptr->iso;                                                               \
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
static inline void* find_##stru_name(mmBase ptr_base, uint mmid, uint untilid) {    \
    if (mmid == cid) {                                                              \
        MM__##stru_name*p = container_of(ptr_base, MM__##stru_name);                \
        return &p->iso;                                                             \
    } else if (mmid == untilid) {                                                   \
        return ptr_base->pre_base->find(ptr_base->pre_base, mmid, cid);             \
    } else if (cid != untilid) {                                                    \
        return ptr_base->pre_base->find(ptr_base->pre_base, mmid, untilid);         \
    }                                                                               \
    return null;                                                                    \
}                                                                                   \
                                                                                    \
static inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                \
                                        void* last_child_base) {                    \
    MM__##stru_name* ptr = p;                                                       \
    ptr->isb.pre_base = pos_b_##sup_name(p);                                        \
    ptr->isb.init = fn_init;                                                        \
    ptr->isb.destroy = fn_destroy;                                                  \
    ptr->isb.find = find_##stru_name;                                               \
    if (init_##sup_name(pool, p, last_child_base) == null) {                        \
        return null;                                                                \
    }                                                                               \
    if (fn_init != null && fn_init(&ptr->iso) == null) {                            \
        return null;                                                                \
    }                                                                               \
    return p;                                                                       \
}                                                                                   \
                                                                                    \
static inline struct stru_name* alloc##stru_name(mgn_memory_pool* pool) {           \
    MM__##stru_name* ptr =                                                          \
          mgn_mem_alloc(pool, sizeof(MM__##stru_name));                             \
    if (init_##stru_name(pool, ptr, &ptr->isb) == null) {                           \
        mgn_mem_release(pool, ptr);                                                 \
        return null;                                                                \
    }                                                                               \
    return &ptr->iso;                                                               \
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
