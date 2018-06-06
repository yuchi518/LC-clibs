# LC-clibs
C libraries include **platform**, **dybuf**, **mmo**, etc.

### Version
0.0.1

### Libraries Descriptions

* [platform](./platform/README.md) library provides reusable and cross platform functions, include type definitions,
  string manipulation, memory management, simple input/output manipulation.
* [dybuf](./dybuf/README.md) library provides a simple way to serialize variable, structure, etc.
* [mmo](./mmo/README.md) library provides an objective architecture, this architecture includes many features:
  life cycle management, serialization, inheritance, virtual function, runtime type casting, etc.
 
 
### System Requirements

If you want to run the test code, your system should meet the following requirements.
- CMake (https://cmake.org)
- Mac OS, Linux, etc.

### Run test code

Build test code:
```sh
# cd /path/to/LC-clibs
# cmake --build ./cmake-build-debug --target unit_test
```
Run test code:
```sh
# ./cmake-build-debug/unit_test
```

### ToDo
- Unit test

License
---
GPLv2
