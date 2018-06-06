cmake_minimum_required(VERSION 3.10)
project(LC_clibs C)

set(CMAKE_C_STANDARD 99)

include_directories(dybuf)
include_directories(mmo)
include_directories(platform)

add_library(LC_clibs
        dybuf/dybuf.h
        dybuf/dypkt.h
        mmo/mmo.h
        mmo/mmo_ext.h
        mmo/mmo_pack.c
        mmo/mmo_pack.h
        mmo/mmo_unittest.c
        mmo/mmo_unittest.h
        platform/plat_io.h
        platform/plat_mem.h
        platform/plat_mgn_mem.h
        platform/plat_string.h
        platform/plat_type.h
        platform/utarray.h
        platform/uthash.h)

set(TEST_SOURCES test.c mmo/mmo_pack.c mmo/mmo_unittest.c)
add_executable(unit_test ${TEST_SOURCES})
target_compile_options(unit_test PRIVATE -DUNIT_TEST=1)
