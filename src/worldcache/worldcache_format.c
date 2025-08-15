#include "worldcache_format.h"
#include <string.h>

void worldcache_header_init(WorldCacheHeader_t* h) {
    if (!h) return;
    memset(h, 0, sizeof(*h));
    h->magic = WORLDCACHE_MAGIC;
    h->format_version = 1;
    h->flags = 0;
    h->sample_rate = 44100.0;
    h->frame_period_ms = 5.0;
    h->wav_hash = 0;
    h->wav_mtime = 0;
    h->num_frames = 0;
    h->fft_size = 0;
    h->sp_size = 0;
    h->ap_size = 0;
    h->voiced_mask_size = 0;
}
