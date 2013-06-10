CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Lua51 REQUIRED)
find_package(ICU REQUIRED)

ADD_DEFINITIONS(-D_POSIX_PTHREAD_SEMANTICS)
ADD_DEFINITIONS(-D_GNU_SOURCE=1)
ADD_DEFINITIONS(-D__EXTENSIONS__=1)

# @todo move this to the toplevel Makefile when we're done
#
SET(CB_GNU_CPPFLAGS "-std=gnu99 -Wall -Werror -pedantic -Wformat -Wundef -Wshadow -fdiagnostics-show-option -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fno-strict-aliasing -fvisibility=hidden -Wstrict-aliasing=2 -pthread")
SET(CB_CLANG_CPPFLAGS "-std=gnu99 -Wall -pedantic -Wformat -Wundef -Wshadow -fdiagnostics-show-option -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fno-strict-aliasing -fvisibility=hidden -Wstrict-aliasing=2 -pthread")
SET(CB_MSVC_CPPFLAGS "")
SET(CB_SPRO_CPPFLAGS "-errfmt=error -errwarn -errshort=tags -xldscope=hidden -mt -v")

SET(CB_GNU_LDFLAGS "-pthread")

#
# Set flags for the C Compiler
#
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CB_GNU_CPPFLAGS}")
  SET(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} ${CB_GNU_LDFLAGS}")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CB_CLANG_CPPFLAGS}")
  IF (!APPLE)
     SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  ENDIF(!APPLE)
  SET(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} ${CB_GNU_LDFLAGS}")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CB_MSVC_CPPFLAGS}")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "SunPro")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CB_SPRO_CPPFLAGS} -xc99=all")
endif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")

IF (WIN32)
    INCLUDE_DIRECTORIES(BEFORE ${CMAKE_SOURCE_DIR}/include
                               ${CMAKE_SOURCE_DIR}
                               ${CMAKE_SOURCE_DIR}/win32)
    SET(COUCHSTORE_OS_FILE src/os_win.c)
    SET(COUCHSTORE_OS_LINK ws2_32)
ELSE (WIN32)
    INCLUDE_DIRECTORIES(BEFORE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR})
    SET(COUCHSTORE_OS_FILE src/os.c)
    SET(COUCHSTORE_OS_LINK stdc++ pthread)
ENDIF (WIN32)

INCLUDE_DIRECTORIES(${ICU_INCLUDE_DIR})

ADD_LIBRARY(mapreduce SHARED
            src/views/mapreduce/mapreduce_c.cc
            src/views/mapreduce/mapreduce.cc)
TARGET_LINK_LIBRARIES(mapreduce v8)
SET_TARGET_PROPERTIES(mapreduce PROPERTIES COMPILE_FLAGS -DLIBMAPREDUCE_INTERNAL=1)

ADD_LIBRARY(couchstore SHARED
            src/arena.c src/btree_modify.c src/btree_read.c
            src/byteswap.c src/couch_db.c src/couch_save.c src/crc32.c
            src/couch_file_read.c src/couch_file_write.c
            src/db_compact.c src/file_merger.c src/file_name_utils.c
            src/file_sorter.c src/iobuffer.c src/llmsort.c
            src/tree_writer.c src/mergesort.c src/node_types.c
            src/reduces.c src/strerror.c src/util.c
            src/views/collate_json.c src/views/file_sorter.c
            src/views/file_merger.c src/views/bitmap.c
            src/views/index_header.c src/views/sorted_list.c
            src/views/reductions.c src/views/keys.c src/views/values.c
            src/views/util.c src/views/reducers.c
            src/rfc1321/md5c.c
            ${COUCHSTORE_OS_FILE})

SET_TARGET_PROPERTIES(couchstore PROPERTIES SOVERSION 1.0.0)
SET_TARGET_PROPERTIES(couchstore PROPERTIES COMPILE_FLAGS "-DLIBCOUCHSTORE_INTERNAL=1 -DPROTOTYPES=1")
TARGET_LINK_LIBRARIES(couchstore mapreduce snappy ${ICU_LIBRARIES} ${COUCHSTORE_OS_LINK})

ADD_EXECUTABLE(couch_dbdump src/dbdump.c)
TARGET_LINK_LIBRARIES(couch_dbdump couchstore ${COUCHSTORE_OS_LINK})
SET_TARGET_PROPERTIES(couch_dbdump PROPERTIES COMPILE_FLAGS -D__STDC_FORMAT_MACROS)

ADD_EXECUTABLE(couch_dbinfo src/dbinfo.c src/util.c)
TARGET_LINK_LIBRARIES(couch_dbinfo couchstore ${COUCHSTORE_OS_LINK})
SET_TARGET_PROPERTIES(couch_dbinfo PROPERTIES COMPILE_FLAGS -D__STDC_FORMAT_MACROS)

ADD_EXECUTABLE(couch_compact src/compactor.c)
TARGET_LINK_LIBRARIES(couch_compact couchstore ${COUCHSTORE_OS_LINK})
SET_TARGET_PROPERTIES(couch_compact PROPERTIES COMPILE_FLAGS -D__STDC_FORMAT_MACROS)

ADD_EXECUTABLE(couch_view_file_sorter src/views/bin/couch_view_file_sorter.c)
TARGET_LINK_LIBRARIES(couch_view_file_sorter couchstore ${COUCHSTORE_OS_LINK})
SET_TARGET_PROPERTIES(couch_view_file_sorter PROPERTIES COMPILE_FLAGS -D__STDC_FORMAT_MACROS)

ADD_EXECUTABLE(couch_view_file_merger src/views/bin/couch_view_file_merger.c)
TARGET_LINK_LIBRARIES(couch_view_file_merger couchstore ${COUCHSTORE_OS_LINK})
SET_TARGET_PROPERTIES(couch_view_file_merger PROPERTIES COMPILE_FLAGS -D__STDC_FORMAT_MACROS)

IF (LUA_LIBRARY)
ADD_EXECUTABLE(couchscript src/couchscript.cc)
TARGET_LINK_LIBRARIES(couchscript couchstore ${LUA_LIBRARIES} ${COUCHSTORE_OS_LINK})
SET_TARGET_PROPERTIES(couchscript PROPERTIES COMPILE_FLAGS -I${LUA_INCLUDE_DIR})
ENDIF (LUA_LIBRARY)

ADD_EXECUTABLE(couchstore_testapp
               tests/testapp.c src/util.c tests/file_merger_tests.c
               tests/file_sorter_tests.c tests/mapreduce/tests.c
               tests/mapreduce/map.c tests/mapreduce/reduce.c
               tests/mapreduce/builtin.c tests/views/tests.c
               tests/views/collate_json_test.c
               tests/views/index_headers.c tests/views/bitmaps.c
               tests/views/sorted_lists.c tests/views/reductions.c
               tests/views/keys.c tests/views/values.c
               tests/views/reducers.c src/node_types.c
               src/file_sorter.c src/file_merger.c
               src/file_name_utils.c src/views/collate_json.c
               src/views/file_sorter.c src/views/file_merger.c
               src/views/bitmap.c src/views/index_header.c
               src/views/sorted_list.c src/views/reductions.c
               src/views/keys.c src/views/values.c src/views/util.c
               src/views/reducers.c )
TARGET_LINK_LIBRARIES(couchstore_testapp couchstore ${COUCHSTORE_OS_LINK})

ENABLE_TESTING()
ADD_TEST(unittests couchstore_testapp)
IF (LUA_LIBRARY)
ADD_TEST(localdoc couchscript ${CMAKE_SOURCE_DIR}/tests/localdoc.lua)
ADD_TEST(corrupt couchscript ${CMAKE_SOURCE_DIR}/tests/corrupt.lua)
ADD_TEST(bulk couchscript ${CMAKE_SOURCE_DIR}/tests/bulk.lua)
ADD_TEST(changes-since-filter couchscript ${CMAKE_SOURCE_DIR}/tests/changessincefilter.lua)
ADD_TEST(compact couchscript ${CMAKE_SOURCE_DIR}/tests/compact.lua)
ADD_TEST(dropdel couchscript ${CMAKE_SOURCE_DIR}/tests/dropdel.lua)
ADD_TEST(largefile couchscript ${CMAKE_SOURCE_DIR}/tests/largefile.lua)
ADD_TEST(large couchscript ${CMAKE_SOURCE_DIR}/tests/large.lua)
ENDIF (LUA_LIBRARY)

INSTALL(FILES
        include/libcouchstore/couch_db.h
        include/libcouchstore/couch_index.h
        include/libcouchstore/couch_common.h
        include/libcouchstore/error.h
        include/libcouchstore/file_ops.h
        include/libcouchstore/visibility.h
        DESTINATION include/libcouchstore)

INSTALL(TARGETS
        couch_dbdump
	couch_dbinfo
	couch_compact
	couch_view_file_sorter
	couch_view_file_merger
        RUNTIME DESTINATION bin)

INSTALL(TARGETS mapreduce couchstore
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
