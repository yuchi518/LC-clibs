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
    uint32 __magic;
    mgn_memory_pool* _pool;
    mmObj_fn _fns;
} *mmObj;

typedef void* Unpacker;
typedef void* Packer;

typedef uint32 oidtyp;
struct mmBase;
typedef struct mmClass {
    oidtyp oid;
    const char* (*name)(void);
    void* (*find)(struct mmBase* base, uint mmid, uint utilid);             // for cast
    mmObj (*find_obj)(struct mmBase* base);                                 // mmobj address is used for memory management.
} *mmClass;

typedef struct mmBase {
    mmClass cls;
    struct mmBase* pre_base;                                                // parent's base address or last child's base address
    void (*destroy)(struct mmBase* base);
    void (*pack)(struct mmBase* base, Packer pkr);
} *mmBase;

plat_inline oidtyp stru_name_to_oid(const char* name) {
    oidtyp hash = 0;
    const char* c = name;
    while((*c) != '\0')
    {
        hash += (*c);
        hash += (hash << 10);
        hash ^= (hash >> 6);
        c++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    //plat_io_printf_std("%08X: %s\n", hash, name);
    return hash;
}

#define MMRootObject(stru_name, fn_init, fn_destroy, fn_pack)                                   \
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
plat_inline const char* name_##stru_name(void) {                                                \
    return "" #stru_name;                                                                       \
}                                                                                               \
                                                                                                \
plat_inline oidtyp oid_of_##stru_name(void) {                                                   \
    static oidtyp oid = 0;                                                                      \
    return oid?oid:(oid=stru_name_to_oid(name_##stru_name()));                                  \
}                                                                                               \
                                                                                                \
plat_inline void* find_##stru_name(mmBase base, uint mmid, uint untilid) {                      \
    if (mmid == base->cls->oid) {                                                               \
        struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb; \
        return &ptr->iso;                                                                       \
    } else if (mmid == untilid) {                                                               \
        return base->pre_base->cls->find(base->pre_base, mmid, base->cls->oid);                 \
    } else if (base->cls->oid != untilid) {                                                     \
        return base->pre_base->cls->find(base->pre_base, mmid, untilid);                        \
    }                                                                                           \
    return null;                                                                                \
}                                                                                               \
                                                                                                \
plat_inline mmObj find_obj_##stru_name(mmBase base) {                                           \
    struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb;     \
    return (void*)ptr;                                                                          \
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
plat_inline mmClass get_##stru_name##_class(void) {                                             \
    static struct mmClass cls;                                                                  \
    if (!cls.oid) {                                                                             \
        cls.oid = oid_of_##stru_name();                                                         \
        cls.find = find_##stru_name;                                                            \
        cls.find_obj = find_obj_##stru_name;                                                    \
        cls.name = name_##stru_name;                                                            \
    };                                                                                          \
    return &cls;                                                                                \
}                                                                                               \
plat_inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                              \
                                        mmBase last_child_base, Unpacker unpkr) {               \
    struct MM__##stru_name* ptr = p;                                                            \
    ptr->isa.__magic = 0x55667788;                                                              \
    ptr->isa._pool = pool;                                                                      \
    ptr->isa._fns = null;                                                                       \
    ptr->isb.cls = get_##stru_name##_class();                                                   \
    ptr->isb.pre_base = last_child_base;                                                        \
    ptr->isb.destroy = destroy_##stru_name;                                                     \
    ptr->isb.pack = pack_##stru_name;                                                           \
    set_hash_for_mmobj(&ptr->iso, hash_##stru_name);                                            \
    set_compare_for_mmobj(&ptr->iso, compare_mmobj_this_with_that);                             \
    struct stru_name* (*init_impl)(struct stru_name*, Unpacker) = fn_init;                      \
    if (init_impl != null) {                                                                    \
        call_f(unpkr, unpackNextContext, &ptr->iso);                                            \
        if (init_impl(&ptr->iso, unpkr) == null) {                                              \
            return null;                                                                        \
        }                                                                                       \
    }                                                                                           \
    return p;                                                                                   \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* alloc##stru_name(mgn_memory_pool* pool) {                         \
    struct MM__##stru_name* ptr =                                                               \
          mgn_mem_alloc(pool, sizeof(struct MM__##stru_name));                                  \
    if (init_##stru_name(pool, ptr, &ptr->isb, null) == null) {                                 \
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
    if (init_##stru_name(pool, ptr, &ptr->isb, unpkr) == null) {                                \
        mgn_mem_release(pool, ptr);                                                             \
        return null;                                                                            \
    }                                                                                           \
    return &ptr->iso;                                                                           \
}                                                                                               \
                                                                                                \
plat_inline void pack_##stru_name(mmBase base, Packer pkr) {                                    \
    struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb;     \
    void (*pack_impl)(struct stru_name*, Packer) = fn_pack;                                     \
    if (pack_impl != null) {                                                                    \
        call_f(pkr, packNextContext, &ptr->iso);                                                \
        pack_impl(&ptr->iso, pkr);                                                              \
    }                                                                                           \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* to##stru_name(void* stru) {                                       \
    if (stru == null) return null;                                                              \
    mmBase base = base_of_mmobj(stru);                                                          \
    oidtyp oid = oid_of_##stru_name();                                                          \
    return (struct stru_name*)(base->cls->find(base, oid, oid));                                \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* baseTo##stru_name(mmBase base) {                                  \
    oidtyp oid = oid_of_##stru_name();                                                          \
    return (struct stru_name*)(base->cls->find(base, oid, oid));                                \
}                                                                                               \
                                                                                                \
plat_inline void register##stru_name##ToUnpacker(Unpacker unpkr) {                              \
    register_allocator(name_##stru_name(), (void*)&unpack##stru_name, unpkr);                   \
}


#define MMSubObject(stru_name, sup_name, fn_init, fn_destroy, fn_pack)                          \
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
plat_inline const char* name_##stru_name(void) {                                                \
    return "" #stru_name;                                                                       \
}                                                                                               \
                                                                                                \
plat_inline oidtyp oid_of_##stru_name(void) {                                                   \
    static oidtyp oid = 0;                                                                      \
    return oid?oid:(oid=stru_name_to_oid(name_##stru_name()));                                  \
}                                                                                               \
                                                                                                \
plat_inline void* find_##stru_name(mmBase base, uint mmid, uint untilid) {                      \
    if (mmid == base->cls->oid) {                                                               \
        struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb; \
        return &ptr->iso;                                                                       \
    } else if (mmid == untilid) {                                                               \
        return base->pre_base->cls->find(base->pre_base, mmid, base->cls->oid);                 \
    } else if (base->cls->oid != untilid) {                                                     \
        return base->pre_base->cls->find(base->pre_base, mmid, untilid);                        \
    }                                                                                           \
    return null;                                                                                \
}                                                                                               \
                                                                                                \
plat_inline mmObj find_obj_##stru_name(mmBase base) {                                           \
    struct MM__##stru_name* ptr = ((void*)base) - (uint)&((struct MM__##stru_name*)0)->isb;     \
    return (void*)ptr;                                                                          \
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
plat_inline mmClass get_##stru_name##_class(void) {                                             \
    static struct mmClass cls;                                                                  \
    if (!cls.oid) {                                                                             \
        cls.oid = oid_of_##stru_name();                                                         \
        cls.find = find_##stru_name;                                                            \
        cls.find_obj = find_obj_##stru_name;                                                    \
        cls.name = name_##stru_name;                                                            \
    };                                                                                          \
    return &cls;                                                                                \
}                                                                                               \
plat_inline void* init_##stru_name(mgn_memory_pool* pool, void* p,                              \
                                        mmBase last_child_base, Unpacker unpkr) {               \
    /* init super first, then init self */                                                      \
    if (init_##sup_name(pool, p, last_child_base, unpkr) == null) {                             \
        return null;                                                                            \
    }                                                                                           \
    struct MM__##stru_name* ptr = p;                                                            \
    ptr->isb.cls = get_##stru_name##_class();                                                   \
    ptr->isb.pre_base = pos_b_##sup_name(p);                                                    \
    ptr->isb.destroy = destroy_##stru_name;                                                     \
    ptr->isb.pack = pack_##stru_name;                                                           \
    struct stru_name* (*init_impl)(struct stru_name*, Unpacker) = fn_init;                      \
    if (init_impl != null) {                                                                    \
        call_f(unpkr, unpackNextContext, &ptr->iso);                                            \
        if (init_impl(&ptr->iso, unpkr) == null) {                                              \
            /*Init fail, destroy super.*/                                                       \
            void (*destroy_super_impl)(mmBase base) = ptr->isb.pre_base->destroy;               \
            destroy_super_impl(ptr->isb.pre_base);                                              \
            return null;                                                                        \
        }                                                                                       \
    }                                                                                           \
    return p;                                                                                   \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* alloc##stru_name(mgn_memory_pool* pool) {                         \
    struct MM__##stru_name* ptr =                                                               \
          mgn_mem_alloc(pool, sizeof(struct MM__##stru_name));                                  \
    if (init_##stru_name(pool, ptr, &ptr->isb, null) == null) {                                 \
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
    if (init_##stru_name(pool, ptr, &ptr->isb, unpkr) == null) {                                \
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
    if (pack_impl != null) {                                                                    \
        call_f(pkr, packNextContext, &ptr->iso);                                                \
        pack_impl(&ptr->iso, pkr);                                                              \
    }                                                                                           \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* to##stru_name(void* stru) {                                       \
    if (stru == null) return null;                                                              \
    mmBase base = base_of_mmobj(stru);                                                          \
    oidtyp oid = oid_of_##stru_name();                                                          \
    return (struct stru_name*)(base->cls->find(base, oid, oid));                                \
}                                                                                               \
                                                                                                \
plat_inline struct stru_name* baseTo##stru_name(mmBase base) {                                  \
    oidtyp oid = oid_of_##stru_name();                                                          \
    return (struct stru_name*)(base->cls->find(base, oid, oid));                                \
}                                                                                               \
                                                                                                \
plat_inline void register##stru_name##ToUnpacker(Unpacker unpkr) {                              \
    register##sup_name##ToUnpacker(unpkr);                                                      \
    register_allocator(name_##stru_name(), (void*)&unpack##stru_name, unpkr);                   \
}


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

plat_inline void* __mem_addr_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    return base->cls->find_obj(base);
}
#define mem_addr_of_mmobj(stru) __mem_addr_of_mmobj(stru)

plat_inline mmBase __base_of_first_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    struct {
        struct mmObj isa;
        struct mmBase isb;
    } *ptr;
    ptr = (void*)base->cls->find_obj(base);
    base = &ptr->isb;

    return base;
}
#define base_of_first_mmobj(stru) __base_of_first_mmobj(stru)

plat_inline void* __last_mmobj(void* stru) {
    if (stru == null) return null;
    struct {
        struct mmObj a;
        struct mmBase b;
        struct {
            uint8 _dummy;
        } c;
    } obj;
    return ((void*)base_of_first_mmobj(stru)->pre_base) + ((uint)&obj.c) - (((uint)&obj.b));
}
#define last_mmobj(stru) __last_mmobj(stru)

plat_inline mgn_memory_pool* __pool_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->cls->find_obj(base);
    return obj->_pool;
}
#define pool_of_mmobj(stru) __pool_of_mmobj(stru)

plat_inline void* __retain_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->cls->find_obj(base);
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
    mmObj obj = base->cls->find_obj(base);                   /*find first obj*/
    mgn_mem_release_w_cb(obj->_pool, obj, __trigger_release_mmobj);
}
#define release_mmobj(stru) __release_mmobj(stru)

plat_inline void* __autorelease_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->cls->find_obj(base);
    mgn_mem_autorelease_w_cb(obj->_pool, obj, __trigger_release_mmobj);
    return stru;
}
#define autorelease_mmobj(stru) ((typeof(stru))__autorelease_mmobj(stru))

plat_inline uint __retain_count_of_mmobj(void* stru) {
    if (stru == null) return 0;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->cls->find_obj(base);
    return (uint)mgn_mem_retained_count(obj->_pool, obj);
}
#define retain_count_of_mmobj(stru) __retain_count_of_mmobj(stru)

plat_inline void* __set_function_for_mmobj(void* stru, const char* fn_type_name, void* fn) {
    if (stru == null) return 0;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->cls->find_obj(base);
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
#define set_function_for_mmobj(stru, fn_type, fn)  ((fn_type)__set_function_for_mmobj(stru, "" #fn_type, fn))

plat_inline void* __get_function_for_mmobj(void* stru, const char* fn_type_name) {
    if (stru == null) return 0;
    mmBase base = base_of_mmobj(stru);
    mmObj obj = base->cls->find_obj(base);
    mmObj_fn fncb;
    HASH_FIND_STR(obj->_fns, fn_type_name, fncb);
    return fncb?fncb->fn:null;
}
#define get_function_for_mmobj(stru, fn_type) ((fn_type)__get_function_for_mmobj(stru, "" #fn_type))
// call_f only supports the function which first argument is structure/obj itself.
#define call_f(stru, fn_type, ...)  ({fn_type _f = get_function_for_mmobj(stru, fn_type); _f?_f(stru, ##__VA_ARGS__):0;})

plat_inline const char* __name_of_mmobj(void* stru) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    return base->cls->name();
}
#define name_of_mmobj(stru) __name_of_mmobj(stru)

plat_inline const char* __name_of_last_mmobj(void* stru) {
    if (stru == null) return null;
    return base_of_first_mmobj(stru)->pre_base->cls->name();
}
#define name_of_last_mmobj(stru) __name_of_last_mmobj(stru)

plat_inline uint __oid_of_last_mmobj(void* stru) {
    if (stru == null) return null;
    return base_of_first_mmobj(stru)->pre_base->cls->oid;
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
    call_f(stru, mmobj_hash, key, key_len);
}
#define hash_of_mmobj(stru, key, key_len) __hash_of_mmobj(stru, key, key_len)

plat_inline bool __is_mmobj_kind_of_oid(void* stru, uint oid) {
    if (stru == null) return null;
    mmBase base = base_of_mmobj(stru);
    return base->cls->find(base, oid, oid)==null?true:false;
}
#define is_mmobj_kind_of_oid(stru, oid) __is_mmobj_kind_of_oid(stru, oid)

/**
 * Implementation notice:
 * 1. mmobj_compare(a, b) should be equal to -mmobj_compare(b, a) in runtime.
 * 2. this_stru and that_stru are always the same object type.
 * 3. For a mmobj supported serialization should have same result
 *    after object unpacked.
 */
typedef int (*mmobj_compare)(void* this_stru, void* that_stru);
plat_inline int compare_mmobj_this_with_that(void* this_stru, void* that_stru) {
    return (int)this_stru-(int)that_stru;
}
plat_inline int single_instance_comparison(void* this_stru, void* that_stru) {
    (void)this_stru;
    (void)that_stru;
    return 0;
}
plat_inline void __set_compare_for_mmobj(void* stru, mmobj_compare cmp) {
    if (stru == null) return;
    set_function_for_mmobj(stru, mmobj_compare, cmp);
}
#define set_compare_for_mmobj(stru, cmp) __set_compare_for_mmobj(stru, cmp)
#define set_single_instance_comparison_for_mmobj(stru) __set_compare_for_mmobj(stru, single_instance_comparison)

plat_inline int __compare_mmobjs(void* this_stru, void* that_stru) {
    if (this_stru == that_stru) return 0;
    if (this_stru == null) return -1;
    if (that_stru == null) return 1;
    if (oid_of_last_mmobj(this_stru) != oid_of_last_mmobj(that_stru)) {
        const char* this_name = name_of_last_mmobj(this_stru);
        const char* that_name = name_of_last_mmobj(that_stru);
        while((*this_name) == (*that_name)) {
            this_name++;
            that_name++;
        }
        return (*this_name) - (*that_name);
    }
    // only call if two mmobjs are same type
    int diff = call_f(this_stru, mmobj_compare, that_stru);
    if (diff) {
        //plat_io_printf_err("%s != %s\n", name_of_last_mmobj(this_stru), name_of_last_mmobj(that_stru));
    }
    return diff;
}
#define compare_mmobjs(this_stru, that_stru) __compare_mmobjs(this_stru, that_stru)
#define are_equal_mmobjs(this_stru, that_stru) (__compare_mmobjs(this_stru, that_stru) == 0?true:false)


/**
 *  Serialization, packer/unpacker, v1
 *  Use uint as key, each object maintain key itself.
 *  Key can be any number, but keep it smaller is better.
 */
#define PACKER_VERSION_V1       (0x01)
typedef uint (*packerVersion)(Packer pkr);
typedef void (*packVarInt64)(Packer pkr, const uint key, int64 value);
typedef void (*packFloat)(Packer pkr, const uint key, float value);
typedef void (*packDouble)(Packer pkr, const uint key, double value);
typedef void (*packData)(Packer pkr, const uint key, uint8* value, uint len);
typedef void (*packObject)(Packer pkr, const uint key, void* value);
typedef void (*packArray)(Packer pkr, const uint key, uint len);
typedef void (*packArrayEnd)(Packer pkr, const uint key);
typedef void (*packNextContext)(Packer pkr, void* stru);

plat_inline uint __packer_version(Packer pkr) {
    return call_f(pkr, packerVersion);
}
#define packer_version(pkr) __packer_version(pkr)
#define is_packer_v1(pkr) (packer_version(pkr)==PACKER_VERSION_V1)

plat_inline void __pack_varint(int64 value, const uint key, Packer pkr) {
    call_f(pkr, packVarInt64, key, value);
}
#define pack_varint(key, value, pkr) __pack_varint(value, key, pkr)
#define pack_bool(key, value, pkr) __pack_varint((value)?1:0, key, pkr)

plat_inline void __pack_float(float value, const uint key, Packer pkr) {
    call_f(pkr, packFloat, key, value);
}
#define pack_float(key, value, pkr) __pack_float(value, key, pkr)

plat_inline void __pack_double(double value, const uint key, Packer pkr) {
    call_f(pkr, packDouble, key, value);
}
#define pack_double(key, value, pkr) __pack_double(value, key, pkr)

plat_inline void __pack_data(void* value, uint len, const uint key, Packer pkr) {
    call_f(pkr, packData, key, value, len);
}
#define pack_data(key, value, len, pkr) __pack_data(value, len, key, pkr)

plat_inline void __pack_array(uint len, const uint key, Packer pkr) {
    call_f(pkr, packArray, key, len);
}
#define pack_array(key, len, pkr) __pack_array(len, key, pkr)

plat_inline void __pack_array_end(const uint key, Packer pkr) {
    call_f(pkr, packArrayEnd, key);
}
#define pack_array_end(key, pkr) __pack_array_end(key, pkr)

plat_inline void __pack_mmobj(void* stru, const uint key, Packer pkr) {
    call_f(pkr, packObject, key, stru);
}
#define pack_mmobj(key, stru, pkr) __pack_mmobj(stru, key, pkr)


#define UNPACKER_VERSION_V1       (PACKER_VERSION_V1)
typedef uint (*unpackerVersion)(Unpacker unpkr);
typedef int64 (*unpackVarInt64)(Unpacker unpkr, const uint key);
typedef float (*unpackFloat)(Unpacker unpkr, const uint key);
typedef double (*unpackDouble)(Unpacker unpkr, const uint key);
typedef uint8* (*unpackData)(Unpacker unpkr, const uint key, uint* p_len);
typedef void* (*unpackObject)(Unpacker unpkr, const uint key);
typedef uint (*unpackArray)(Unpacker unpkr, const uint key);
typedef void (*unpackArrayEnd)(Unpacker unpkr, const uint key);
typedef void (*unpackNextContext)(Unpacker unpkr, void* stru);
typedef void (*registerAllocator)(Unpacker unpkr, const char* obj_name, void* fn);

plat_inline uint __unpacker_version(Unpacker unpkr) {
    return call_f(unpkr, unpackerVersion);
}
#define unpacker_version(unpkr) __unpacker_version(unpkr)
#define is_unpacker_v1(unpkr) (unpacker_version(unpkr)==UNPACKER_VERSION_V1)

plat_inline int64 __unpack_varint(const uint key, Unpacker unpkr) {
    return call_f(unpkr, unpackVarInt64, key);
}
#define unpack_varint(key, unpkr) __unpack_varint(key, unpkr)
#define unpack_bool(key, unpkr) (__unpack_varint(key, unpkr)?true:false)

plat_inline float __unpack_float(const uint key, Unpacker unpkr) {
    return call_f(unpkr, unpackFloat, key);
}
#define unpack_float(key, unpkr) __unpack_float(key, unpkr)

plat_inline double __unpack_double(const uint key, Unpacker unpkr) {
    return call_f(unpkr, unpackDouble, key);
}
#define unpack_double(key, unpkr) __unpack_double(key, unpkr)

plat_inline uint8* __unpack_data(const uint key, uint* p_len, Unpacker unpkr) {
    return call_f(unpkr, unpackData, key, p_len);
}
#define unpack_data(key, p_len, unpkr) __unpack_data(key, p_len, unpkr)

plat_inline uint __unpack_array(const uint key, Unpacker unpkr) {
    return call_f(unpkr, unpackArray, key);
}
#define unpack_array(key, unpkr) __unpack_array(key, unpkr)

plat_inline void __unpack_array_end(const uint key, Unpacker unpkr) {
    call_f(unpkr, unpackArrayEnd, key);
}
#define unpack_array_end(key, unpkr) __unpack_array_end(key, unpkr)

plat_inline void* __unpack_mmobj(Unpacker unpkr, const uint key) {
    return call_f(unpkr, unpackObject, key);
}
#define unpack_mmobj(key, unpkr) __unpack_mmobj(unpkr, key)

plat_inline void __register_allocator(Unpacker unpkr, const char* obj_name, void* fn) {
    call_f(unpkr, registerAllocator, obj_name, fn);
}
#define register_allocator(obj_name, fn, unpkr) __register_allocator(unpkr, obj_name, fn)

/*
 * ============= Samples ================
 * Three objects: MMObj, MMChild, MMSon
 *
 * ======================================
 */
// ===== Root =====

typedef struct MMRoot {

}*MMRoot;

plat_inline MMRoot initMMRoot(MMRoot obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMRoot(MMRoot obj) {

}

MMRootObject(MMRoot, initMMRoot, destroyMMRoot, null);

/// ====== Child =====
typedef struct MMChild {

}*MMChild;

plat_inline MMChild initMMChild(MMChild obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMChild(MMChild obj) {

}

MMSubObject(MMChild, MMRoot, initMMChild, destroyMMChild, null);

/// ===== Son =====
typedef struct MMSon {

}*MMSon;

plat_inline MMSon initMMSon(MMSon obj, Unpacker unpkr) {
    return obj;
}

plat_inline void destroyMMSon(MMSon obj) {

}

MMSubObject(MMSon, MMChild, initMMSon, destroyMMSon, null);




#endif //PROC_LA_MM_OBJ_H_H
