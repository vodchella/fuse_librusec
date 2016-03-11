cmake_minimum_required(VERSION 3.3)
project(fuse_librusec)


INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindFUSE.cmake)
IF(FUSE_FOUND)
    MESSAGE(STATUS "Found fuse library -- " ${FUSE_LIBRARIES})
    INCLUDE_DIRECTORIES(${FUSE_INCLUDE_DIR})
ELSE(FUSE_FOUND)
    MESSAGE(FATAL_ERROR "The fuse library is required.")
ENDIF(FUSE_FOUND)


INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindSQLite3.cmake)
IF(SQLITE3_FOUND)
    MESSAGE(STATUS "Found sqlite3 library -- " ${SQLITE3_LIBRARIES})
    INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIRS})
ELSE(SQLITE3_FOUND)
    MESSAGE(FATAL_ERROR "The sqlite3 library is required.")
ENDIF(SQLITE3_FOUND)


INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindLibZip.cmake)
IF(LIBZIP_FOUND)
    MESSAGE(STATUS "Found libzip library -- " ${LIBZIP_LIBRARIES})
    INCLUDE_DIRECTORIES(${LIBZIP_INCLUDE_DIRS})
ELSE(LIBZIP_FOUND)
    MESSAGE(FATAL_ERROR "The libzip library is required.")
ENDIF(LIBZIP_FOUND)


set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES src/main.c src/main.h src/utils/sqlite.c src/utils/sqlite.h src/utils/str.c src/utils/str.h src/utils/list.h src/file_cache.c src/file_cache.h)
add_executable(fuse_librusec ${SOURCE_FILES})
target_link_libraries(fuse_librusec ${FUSE_LIBRARIES})
target_link_libraries(fuse_librusec ${SQLITE3_LIBRARIES})
target_link_libraries(fuse_librusec ${LIBZIP_LIBRARIES})