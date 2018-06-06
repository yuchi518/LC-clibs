# **platform** library
Provide reusable and cross platform functions, include type definitions, string manipulation, memory management,
simple input/output manipulation.

### Version
0.0.1

### Tech
This library uses one open source project to maintain memory allocation, if your project doesn't include `plat_mgn_mem.h` 
header, you don't need to care about it.

* [uthash](https://troydhanson.github.io/uthash/)  C macros for hash tables and arrays


### Integrate **platform** library into your project
1. All functions are implemented in macros or static inline functions, you should set `platform` folder as one of
   include directories.

2. Include the header files which you want to use, ex:

```C
#include "plat_mgn_mem.h"
```

3. Use memory management API

```C
// create a pool for memory management
mgn_memory_pool pool = NULL;

void *m = NULL, *m2, *m3;
// create three memory spaces
m = mgn_mem_alloc(&pool, 100);
m2 = mgn_mem_alloc(&pool, 100);
m3 = mgn_mem_alloc(&pool, 100);

// ...
// use memory
// ...

// retain or release memory spaces
mgn_mem_retain(&pool, m3);
mgn_mem_release(&pool, m);
mgn_mem_release(&pool, m3);

// apply auto release, the memory will not be released until 
// mgn_mem_release_unused is called next time.
mgn_mem_autorelease(&pool, m3);

// release unused memory spaces
mgn_mem_release_unused(&pool);

// force to release all memory spaces
mgn_mem_release_all(&pool);
```


### ToDo
- More platform test
- Document

License
---
GPLv2
