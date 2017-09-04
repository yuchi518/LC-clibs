# dybuf, dypkt - C library
This is C library of dybuf and dypkt.

### Version
0.0.1

### Tech
C lib version uses a number of open source projects to generate json parser, if you just use dybuf or dypkt lib, you don't care about it.

* [bison] - A parser generator. (https://www.gnu.org/software/bison/)
* [flex] - The Fast Lexical Analyzer. (http://flex.sourceforge.net)


### System Requirements

If you want to run the test code, your system should meet the following requirements.
- CMake (https://cmake.org)
- Mac OS, Linux, etc.

### Run test code

Build test code:
```sh
# cd /path/to/dybuf/c
# cmake .
# make install
```
Run test code:
```sh
# ./dybuf_c
```

### Integrate dypkt with your project
1. Copy following files to your project's include path.
   * dybuf.h
   * dypkt.h
   * platform/plat_type.h
   * platform/plat_mem.h
   * platform/plat_string.h

2. Include the header file

        #include "dypkt.h"

3. Use dypkt API for pack. (Write to memory)

        // Create a dypkt structure pointer, refer to a 1024 bytes memory
        uint8 mem[1024];
        dypkt* dyp = dyp_pack(null, mem, 1024);
        // Option, append version & protocol information
        dyp_append_protocol(dyp, "bjson);               // binary json
        dyp_append_protocol_version(dyp, 0x010001);     // version 1.0.1
        // Append your data
        dyp_append_int(dyp, 0, 123);        // append a int with value 123
        dyp_append_string(dyp, 1, "hi");    // append a string with value "hi"
        // Option, append EOF
        dyp_append_eof(dyp);
        // Release
        uint size = dyp_get_position(dyp0);
        dyp_release(dyp0);
        // Now, you can send mem to remote or save to file.
        fwrite(mem, size, 0, fid);          // write to fid

4. Use dypkt API for unpack. (Read from memory)

        // Receive the data form remote or read from file.
        uint8 mem[1024];
        uint size;
        size = fread(mem, 1024, 1, fid);
        // Create a dypkt structure pointer, refer to data memory
        dyp = dyp_unpack(null, mem, size, false);
        // Read all data
        while(dyp_get_remainder(dyp)>0)             // read all
        {
            dype type;
            uint index;
            type = dyp_next_type(dyp, &index);      // get type first
            if (type==dype_f)
            {
                // function
                dype_fid func_id = (dype_fid)index;
                if (func_id==dype_f_eof) 
                {
                    dyp_next_eof(dyp);
                    break; // eof
                }
                else if (func_id==dype_f_protocol)
                {
                    char* protocol_name =
                        dyp_next_protocol(dyp, null);
                    // You should copy the name to other memory if you want to keep it.
                }
                else if (func_id==dype_f_proto_version)
                {
                    uint64 ver =
                        dyp_next_protocol_version(dyp);  // read version
                }
            }
            else if (type==dype_int)
            {
                int64 value =
                    dyp_next_int(dyp)
            }
            else if (type==dype_string)
            {
                char* str =
                    dyp_next_cstring(dyp, null);
                // You should copy the str to other memory if you want ot keep it.
            }
            // other types
        }
        // Release
        dyp_release(dyp);
        // If you know the order of appended data, you can just call 
        // dyp_next_xxx in order to get the data.



### ToDo
- JSON and binary JSON converter

License
---
GPLv2
