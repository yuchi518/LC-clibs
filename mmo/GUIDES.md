# **mmo** library guides

**mmo** library is an objective architecture, it supports many objective features in runtime, such as runtime type
casting (recognizing), virtual functions, comparison, hashable, etc.
Many languages (ex, C++, Obj-C) had support these features and had more powerful features,
but sometimes we just want to use C language to implement portable projects easily. 

### TOC
* [Inheritance](#inheritance)
    * [Declare a root object](#declare-a-root-object)
    * [Declare sub-objects](#declare-sub-objects)
    * [Runtime type casting](#runtime-type-casting)
* [Function implementation](#function-implementation)
    * [Create a simple function](#create-a-simple-function)
    * [Create a virtual function](#create-a-virtual-function)
* [Serialization](#serialization)


### Inheritance
If you want to declare objects' relationship in C language, you can use nesting structures to do that. It likes following
sample code:
```C
strct Transport
{
    int speed;
};

struct Car
{
    struct Transport p;
    int number_of_wheels;
};
```
If you make sure you always put super-object in first line of sub-object, you can do type casting likes following way:
```C
struct Car c1;
struct Car* p_c = &c1;
// casting
struct Transport* p_t = (void*)p_c;
struct Car* p_c2 = (void*)p_t;
```
But this coding style for type casting is danger, if you change the order of structure's variables such as following,
you will get big problem.
```C
struct Car
{
    int number_of_wheels;
    struct Transport p;
};
```
If fact, you can implement conversion code to cover the problem, but it is really hard to maintain.

Another big trouble is type recognizing, we can see the trouble in following example:
```C
struct Boat
{
    struct Transport p;
    int number_of_masts;
};

Root* array[10];
int array_index = 0;

void add(Root* r)
{
    array[array_index++] = r;
}

// ...
struct Car c;
struct Boat b;

add(&c);
add(&b);

```
After you collect a lot of transport in array, how do you know what is it in each entry. A simple way is add a new 
variable in `struct Transport`, likes:
```C
enum TransportType
{
   CAR,
   BOAT,
};
strct Transport
{
    enum TransportType type;
    int speed;
};

// Set a correct type
struct Car c;
c.p.type = CAR;
struct Boat b;
b.p.type = BOAT;
```
We always need to set a correct type in transport object, but there few problems in this coding.
* We can guarantee the type is correct. When you try to maintain many objects, it is easy to make a mistake.
* If you want to release this code to many people, how to maintain transport types is a trouble.

`mmo` library provides a easy way to maintain object relationship, you can see that in following sections.
In following sections, we use objects which were implemented in default hierarchy as example.

#### Declare a root object
```C
typedef struct MMObject {
    // declare variables which are belong to this object
}* MMObject;

MMRootObject(MMObject, null, null, null);
```

The macro function `MMRootObject` was defined in `mmo.h` header file, it is used to create a root object. One hierarchy
should contain one and only one root object, so the macro function should be used one time in one hierarchy implementation.

We didn't declare any variable in `MMObject`, you will see other examples in next section.
The macro function `MMRootObject` needs four parameters: `stru_name`, `fn_init`, `fn_destroy`, `fn_pack`.

* `stru_name`: structure/object name, the macro will use this name to generate many functions, I suggest the name should
start with a capital letter. 
* `fn_init`: an initialization function pointer, it will be called after a object is allocated.
* `fn_destroy`: a de-initialization function pointer, it will be called before a object is ready to release.
* `fn_pack`: a serialization function pointer, it will be called if we want to serialize the object.

`MMObject` is a simple object, it doesn't need specific initialization, de-initialization and serialization
procedure, so the last three parameters are `null`.

Now, you can create a `MMObject` easily.
```C
mgn_memory_pool pool = null;
MMObject obj = allocMMObject(&pool);
/// ... do something
release_mmobj(obj);
```

#### Declare sub-objects
`MMPrimary` is a sub-object inherit from `MMObject`, the declaration likes following code:
```C
typedef struct MMPrimary {
}*MMPrimary;

MMSubObject(MMPrimary, MMObject, null, null, null);
```
The macro function `MMSubObject` is used to create a sub-object, it needs five parameters: `stru_name`, `sup_name`, 
`fn_init`, `fn_destroy`, `fn_pack`.
* `sup_name`: the name of super-object, the new object inherit from this super-object.

The following example is `MMInt`, `MMInt` is more complex than `MMPrimary`, I omit many functions from real implementation. 
```C
typedef struct MMInt {
    int32 value;
}*MMInt;

plat_inline MMInt initMMInt(MMInt obj, Unpacker unpkr) {
    // ... do something
    return obj;
}

plat_inline void packMMInt(MMInt obj, Packer pkr) {
    // ... do something
}

MMSubObject(MMInt, MMPrimary, initMMInt, null, packMMInt);

plat_inline MMInt allocMMIntWithValue(mgn_memory_pool* pool, int32 value) {
    MMInt obj = allocMMInt(pool);
    if (obj) {
        obj->value = value;
    }
    return obj;
}
```
*on-going*

#### Runtime type casting
*on-going*

### Function implementation
 
#### Create a simple function
*on-going*

#### Create a virtual function
*on-going*

### Serialization
*on-going*
