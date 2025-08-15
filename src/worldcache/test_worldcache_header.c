#include <stdio.h>
#include <stddef.h>
#include "worldcache_format.h"

int main(void) {
    WorldCacheHeader_t h;
    worldcache_header_init(&h);
    printf("WorldCacheHeader_t size: %zu bytes\n", sizeof(WorldCacheHeader_t));
    printf("offset(magic) = %zu\n", offsetof(WorldCacheHeader_t, magic));
    printf("offset(format_version) = %zu\n", offsetof(WorldCacheHeader_t, format_version));
    printf("offset(flags) = %zu\n", offsetof(WorldCacheHeader_t, flags));
    printf("offset(sample_rate) = %zu\n", offsetof(WorldCacheHeader_t, sample_rate));
    printf("offset(frame_period_ms) = %zu\n", offsetof(WorldCacheHeader_t, frame_period_ms));
    printf("offset(wav_hash) = %zu\n", offsetof(WorldCacheHeader_t, wav_hash));
    printf("offset(wav_mtime) = %zu\n", offsetof(WorldCacheHeader_t, wav_mtime));
    printf("offset(num_frames) = %zu\n", offsetof(WorldCacheHeader_t, num_frames));
    printf("offset(fft_size) = %zu\n", offsetof(WorldCacheHeader_t, fft_size));
    printf("offset(sp_size) = %zu\n", offsetof(WorldCacheHeader_t, sp_size));
    printf("offset(ap_size) = %zu\n", offsetof(WorldCacheHeader_t, ap_size));
    printf("offset(voiced_mask_size) = %zu\n", offsetof(WorldCacheHeader_t, voiced_mask_size));
    return 0;
}
