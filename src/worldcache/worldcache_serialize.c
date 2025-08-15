#include "worldcache_serialize.h"
#include <stdlib.h>
#include <string.h>
#if defined(USE_ZSTD)
#include <zstd.h>
#endif

int worldcache_serialize(const WorldCacheHeader_t* h,
                         const uint8_t* sp, const uint8_t* ap, const uint8_t* voiced_mask,
                         uint8_t** out_buf, size_t* out_size) {
    if (!h || !out_buf || !out_size) return -1;
    /* If compression requested and compiled with zstd, compress concatenated payload */
#if defined(USE_ZSTD)
    if (h->flags & WORLDCACHE_FLAG_COMPRESSED) {
        size_t payload_size = (size_t)h->sp_size + (size_t)h->ap_size + (size_t)h->voiced_mask_size;
        uint8_t* payload = (uint8_t*)malloc(payload_size);
        if (!payload) return -1;
        uint8_t* q = payload;
        if (h->sp_size) { memcpy(q, sp, h->sp_size); q += h->sp_size; }
        if (h->ap_size) { memcpy(q, ap, h->ap_size); q += h->ap_size; }
        if (h->voiced_mask_size) { memcpy(q, voiced_mask, h->voiced_mask_size); q += h->voiced_mask_size; }

        size_t max_csize = ZSTD_compressBound(payload_size);
        uint8_t* cbuf = (uint8_t*)malloc(max_csize);
        if (!cbuf) { free(payload); return -1; }
        size_t csize = ZSTD_compress(cbuf, max_csize, payload, payload_size, 1);
        free(payload);
        if (ZSTD_isError(csize)) { free(cbuf); return -1; }

        size_t total = sizeof(WorldCacheHeader_t) + csize + sizeof(uint64_t); /* store uncompressed size */
        uint8_t* buf = (uint8_t*)malloc(total);
        if (!buf) { free(cbuf); return -1; }
        uint8_t* p = buf;
        memcpy(p, h, sizeof(WorldCacheHeader_t)); p += sizeof(WorldCacheHeader_t);
        /* write uncompressed payload size as u64 */
        uint64_t ulen = (uint64_t)payload_size;
        memcpy(p, &ulen, sizeof(ulen)); p += sizeof(ulen);
        memcpy(p, cbuf, csize); p += csize;
        free(cbuf);
        *out_buf = buf; *out_size = total; return 0;
    }
#endif

    /* default: no compression */
    size_t total = sizeof(WorldCacheHeader_t) + (size_t)h->sp_size + (size_t)h->ap_size + (size_t)h->voiced_mask_size;
    uint8_t* buf = (uint8_t*)malloc(total);
    if (!buf) return -1;
    uint8_t* p = buf;
    memcpy(p, h, sizeof(WorldCacheHeader_t)); p += sizeof(WorldCacheHeader_t);
    if (h->sp_size) { memcpy(p, sp, h->sp_size); p += h->sp_size; }
    if (h->ap_size) { memcpy(p, ap, h->ap_size); p += h->ap_size; }
    if (h->voiced_mask_size) { memcpy(p, voiced_mask, h->voiced_mask_size); p += h->voiced_mask_size; }
    *out_buf = buf;
    *out_size = total;
    return 0;
}

int worldcache_deserialize(const uint8_t* buf, size_t buf_size,
                           WorldCacheHeader_t* out_h,
                           uint8_t** out_sp, uint8_t** out_ap, uint8_t** out_voiced_mask) {
    if (!buf || !out_h || !out_sp || !out_ap || !out_voiced_mask) return -1;
    if (buf_size < sizeof(WorldCacheHeader_t)) return -1;
    const uint8_t* p = buf;
    memcpy(out_h, p, sizeof(WorldCacheHeader_t)); p += sizeof(WorldCacheHeader_t);
    size_t remaining = buf_size - sizeof(WorldCacheHeader_t);
    /* If compressed, read ulen and decompress payload then split */
#if defined(USE_ZSTD)
    if (out_h->flags & WORLDCACHE_FLAG_COMPRESSED) {
        if (remaining < sizeof(uint64_t)) return -1;
        uint64_t ulen = 0;
        memcpy(&ulen, p, sizeof(ulen)); p += sizeof(ulen);
        size_t csize = remaining - sizeof(ulen);
        const void* cptr = p;
        size_t payload_size = (size_t)ulen;
        uint8_t* payload = (uint8_t*)malloc(payload_size);
        if (!payload) return -1;
        size_t dres = ZSTD_decompress(payload, payload_size, cptr, csize);
        if (ZSTD_isError(dres) || dres != payload_size) { free(payload); return -1; }
        /* now split payload into blocks */
        const uint8_t* q = payload;
        if (out_h->sp_size) {
            *out_sp = (uint8_t*)malloc(out_h->sp_size);
            if (!*out_sp) { free(payload); return -1; }
            memcpy(*out_sp, q, out_h->sp_size); q += out_h->sp_size;
        } else { *out_sp = NULL; }
        if (out_h->ap_size) {
            *out_ap = (uint8_t*)malloc(out_h->ap_size);
            if (!*out_ap) { free(payload); free(*out_sp); return -1; }
            memcpy(*out_ap, q, out_h->ap_size); q += out_h->ap_size;
        } else { *out_ap = NULL; }
        if (out_h->voiced_mask_size) {
            *out_voiced_mask = (uint8_t*)malloc(out_h->voiced_mask_size);
            if (!*out_voiced_mask) { free(payload); free(*out_sp); free(*out_ap); return -1; }
            memcpy(*out_voiced_mask, q, out_h->voiced_mask_size); q += out_h->voiced_mask_size;
        } else { *out_voiced_mask = NULL; }
        free(payload);
        return 0;
    }
#endif

    if (remaining < (size_t)out_h->sp_size + (size_t)out_h->ap_size + (size_t)out_h->voiced_mask_size) return -1;
    if (out_h->sp_size) {
        *out_sp = (uint8_t*)malloc(out_h->sp_size);
        if (!*out_sp) return -1;
        memcpy(*out_sp, p, out_h->sp_size); p += out_h->sp_size;
    } else {
        *out_sp = NULL;
    }
    if (out_h->ap_size) {
        *out_ap = (uint8_t*)malloc(out_h->ap_size);
        if (!*out_ap) { free(*out_sp); return -1; }
        memcpy(*out_ap, p, out_h->ap_size); p += out_h->ap_size;
    } else {
        *out_ap = NULL;
    }
    if (out_h->voiced_mask_size) {
        *out_voiced_mask = (uint8_t*)malloc(out_h->voiced_mask_size);
        if (!*out_voiced_mask) { free(*out_sp); free(*out_ap); return -1; }
        memcpy(*out_voiced_mask, p, out_h->voiced_mask_size); p += out_h->voiced_mask_size;
    } else {
        *out_voiced_mask = NULL;
    }
    return 0;
}

void worldcache_free_blocks(uint8_t* sp, uint8_t* ap, uint8_t* voiced_mask) {
    if (sp) free(sp);
    if (ap) free(ap);
    if (voiced_mask) free(voiced_mask);
}
