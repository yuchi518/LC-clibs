# **mmo** library
Provide an objective architecture, this architecture includes many features:
life cycle management, serialization, inheritance, virtual function, runtime type casting, etc.

### Version
0.0.1

### Integrate **mmo** library into your project
1. Most functions are implemented in macros or static inline functions, you should set `platform`, `dybuf` and `mmo`
   folders as include directories.

2. Set `mmo_pack.c` as source file if you want to use object serialization feature, it means if your project includes
   `mmo_pack.h`, your project also needs `mmo_pack.c`.
   
3. Set `mmo_unittest.c` as source file if you also want to execute unittest in your project.

### Use default hierarchy
The default hierarchy includes many useful objects, include: 
* `MMObject` is the root object in default hierarchy
* `MMPrimary` inherits from `MMObject` and all primitive objects inherit from this object.
    * `MMInt`
    * `MMLong`
    * `MMFloat`
    * `MMDouble`
    * `MMString`
    * `MMData`
* `MMContainer` inherits from `MMObject` and all collection objects inherit from this object.
    * `MMList`
    * `MMMap`
* `MMpacker` and `MMUnpacker` inherit from `MMObject` and these objects take care about object serialization.

Sample code for map manipulation.
```C
#include "mmo_ext.h"

// Prepare an object pool
mgn_memory_pool pool = null;

// Create key and value objects (key='hi5', value=5)
MMString hi5 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi5"));
MMInt v5 = autorelease_mmobj(allocMMIntWithValue(&pool, 5));

// Create key and value objects (key='hi6', value=6.0)
MMString hi6 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi6"));
MMDouble v6 = autorelease_mmobj(allocMMDoubleWithValue(&pool, 6.0));

// Create a map object
MMMap map = allocMMMap(&pool);

// Put two pairs
addMMMapItem(map, toMMPrimary(hi5), toMMObject(v5));
addMMMapItem(map, toMMPrimary(hi6), toMMObject(v6));

// Get value for key 'hi5'
MMObject obj = getMMMapItemValue(map, toMMPrimary(hi5));
if (oid_of_last_mmobj(obj) == oid_of_MMInt()) {
    plat_io_printf_dbg("Got it!! (%d)\n", toMMInt(obj)->value);
} else {
    plat_io_printf_err("What is this?(%u)\n", oid_of_last_mmobj(obj));
}

// Get value for key 'hi6'
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

```

You can refer more samples from `mmo_unitest.c` file.


### Implement a new hierarchy
If you don't want to use default hierarchy, you can implement a new hierarchy. In this situation, your project should
just include `mmo.h` file only. You should be careful about two things:
1. Your project should not used multiple hierarchies as same time. (In fact, you can do that, but you need to be very
   careful about objects maintenance.)
2. You should also need to implement your serialization approach if you want to serialize your new objects into file.

Sample code for object definition:
```c
#include "mmo.h"

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

```

You should not use MMRoot, MMChild, MMSon as object names in new hierarchy, because this sample code also put in `mmo.h` file.

Sample code for object usage:
```C
mgn_memory_pool pool = null;
MMSon son = allocMMSon(&pool);
MMChild child = toMMChild(son);
MMRoot root = toMMRoot(child);

plat_io_printf_dbg("Son's name = %s\n", name_of_mmobj(son));
plat_io_printf_dbg("Child's name = %s\n", name_of_mmobj(child));
plat_io_printf_dbg("Root's name = %s\n", name_of_mmobj(root));

release_mmobj(child);
```

If you want to learn more object implementation, you can refer the implementation of default hierarchy.

### Reference
* [**mmo** library guides](./GUIDES.md)

### ToDo
- Document

License
---
GPLv2
