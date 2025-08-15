#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "worldcache_format.h"
#include "worldcache_serialize.h"

int main(void) {
    WorldCacheHeader_t h;
    worldcache_header_init(&h);
    /* small dummy blocks */
    h.num_frames = 2;
    h.fft_size = 4;
    h.sp_size = 16; /* bytes */
    h.ap_size = 8;
    h.voiced_mask_size = 4;

    uint8_t* sp = malloc(h.sp_size);
    uint8_t* ap = malloc(h.ap_size);
    uint8_t* vm = malloc(h.voiced_mask_size);
    if (!sp || !ap || !vm) { perror("alloc"); return 2; }
    for (size_t i=0;i<h.sp_size;i++) sp[i] = (uint8_t)(i+1);
    for (size_t i=0;i<h.ap_size;i++) ap[i] = (uint8_t)(i+11);
    for (size_t i=0;i<h.voiced_mask_size;i++) vm[i] = (uint8_t)(i+21);

    uint8_t* buf = NULL; size_t buf_size = 0;
    if (worldcache_serialize(&h, sp, ap, vm, &buf, &buf_size) != 0) {
        fprintf(stderr, "serialize failed\n"); return 3;
    }
    printf("serialized %zu bytes\n", buf_size);

    WorldCacheHeader_t h2;
    uint8_t *sp2=NULL, *ap2=NULL, *vm2=NULL;
    if (worldcache_deserialize(buf, buf_size, &h2, &sp2, &ap2, &vm2) != 0) {
        fprintf(stderr, "deserialize failed\n"); free(buf); return 4;
    }
    /* verify */
    if (h2.sp_size != h.sp_size || h2.ap_size != h.ap_size || h2.voiced_mask_size != h.voiced_mask_size) {
        fprintf(stderr, "size mismatch\n"); return 5;
    }
    if (memcmp(sp, sp2, h.sp_size)!=0) { fprintf(stderr, "sp mismatch\n"); return 6; }
    if (memcmp(ap, ap2, h.ap_size)!=0) { fprintf(stderr, "ap mismatch\n"); return 7; }
    if (memcmp(vm, vm2, h.voiced_mask_size)!=0) { fprintf(stderr, "vm mismatch\n"); return 8; }

    printf("serialize/deserialize roundtrip OK\n");

    free(sp); free(ap); free(vm);
    worldcache_free_blocks(sp2, ap2, vm2);
    free(buf);
    return 0;
}
