# mmo - C library
Provide an C object architecture, the architecture includes many features: life cycle management, 
serialization, inheritance, etc.

### Version
0.0.1

### Integrate `mmo` library into your project
1. Most functions are implemented in macros or static inline functions, you should set `platform`, `dybuf` and `mmo`
   folders as include directories.

2. Set `mmo_pack.c` as source file if you want to use object serialization feature, it means if your project includes
   `mmo_pack.h`, your project also needs `mmo_pack.c`.
   
3. Set `mmo_unittest.c` as source file if you also want to execute unittest in your project.

### Use `MMXxxxx` series based on `MMObject`
`mmo` library includes many useful objects which based on `MMObject`, these objects include: 
* `MMObject`, the root object for this series
* `MMPrimary`, inherits from `MMObject` and all primitive wrappers inherit from this object.
    * `MMInt`
    * `MMLong`
    * `MMFloat`
    * `MMDouble`
    * `MMString`
    * `MMData`
* `MMContainer`, inherits from `MMObject` and all collection wrappers inherit from this object.
    * `MMList`
    * `MMMap`
* `MMpacker` and `MMUnpacker`, inherit from `MMObject` and these two objects take care object serialization.

Sample code for map manipulation.
```C
#include "mmo_ext.h"

// ...

mgn_memory_pool pool = null;

MMString hi5 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi5"));
MMInt v5 = autorelease_mmobj(allocMMIntWithValue(&pool, 5));

MMString hi6 = autorelease_mmobj(allocMMStringWithCString(&pool, "hi6"));
MMDouble v6 = autorelease_mmobj(allocMMDoubleWithValue(&pool, 6.0));

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

```

You can refer more sample from `mmo_unitest.c` file and I will explain `mmo` design on other document.


### Implement a new root object
If you don't want to use `MMXxxxx` series, you can implement a new series. In this case, your project should just
include `mmo.h` file only.
Two things you should know:
1. Your project should not used multiple hierarchy as same time. (In fact, you can do that, but you need to be very
   careful about objects maintenance.)
2. You should also need to implement your serialization scheme if you want to serialize your new objects into file.

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

You should not use MMRoot, MMChild, MMSon as new object names, because this sample code also put in `mmo.h` file.

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

If you want to learn more object implementation, you can refer the implementation of `MMXxxx` series based on `MMObject`.

### ToDo
- Document

License
---
GPLv2
