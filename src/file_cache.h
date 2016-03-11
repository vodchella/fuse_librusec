//
// Created by twister on 11.03.16.
//

#ifndef FUSE_LIBRUSEC_FILE_CACHE_H
#define FUSE_LIBRUSEC_FILE_CACHE_H

void fcache_init();
void fcache_append(struct path_info* pi);
void fcache_append_with_copy(struct path_info* pi);
struct path_info* fcache_find_by_path(const char* path);
void fcache_unlink_and_free(struct path_info* pi);

#endif //FUSE_LIBRUSEC_FILE_CACHE_H
