#ifndef WORLDCACHE_SERIALIZE_H
#define WORLDCACHE_SERIALIZE_H

#include <stddef.h>
#include <stdint.h>
#include "worldcache_format.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Serialize header + data blocks into a contiguous buffer. Caller must free *out_buf. */
int worldcache_serialize(const WorldCacheHeader_t* h,
                         const uint8_t* sp, const uint8_t* ap, const uint8_t* voiced_mask,
                         uint8_t** out_buf, size_t* out_size);

/* Deserialize buffer into header and allocate memory for each block. Caller must free blocks with worldcache_free_blocks. */
int worldcache_deserialize(const uint8_t* buf, size_t buf_size,
                           WorldCacheHeader_t* out_h,
                           uint8_t** out_sp, uint8_t** out_ap, uint8_t** out_voiced_mask);

void worldcache_free_blocks(uint8_t* sp, uint8_t* ap, uint8_t* voiced_mask);

#ifdef __cplusplus
}
#endif

#endif /* WORLDCACHE_SERIALIZE_H */
