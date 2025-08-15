#include "worldcache_manager.h"
#include "worldcache_serialize.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* Simple 64-bit rolling hash for file content - placeholder for OTO hash */
static uint64_t simple_file_hash(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    uint64_t h = 1469598103934665603ULL;
    int c;
    while ((c = fgetc(f)) != EOF) {
        h ^= (uint64_t)c;
        h *= 1099511628211ULL;
    }
    fclose(f);
    return h;
}

static uint64_t get_file_mtime(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return (uint64_t)st.st_mtime;
}

/* Placeholder analysis: creates small dummy arrays based on wav_path length */
static int world_analyze_wav_stub(const char* wav_path, WORLD_AnalysisData* out) {
    if (!out) return -1;
    out->sample_rate = 44100.0;
    out->frame_period_ms = 5.0;
    out->num_frames = 10;
    out->fft_size = 512;
    out->sp = malloc( (size_t)out->num_frames * out->fft_size * sizeof(float) ); /* pretend */
    out->ap = malloc((size_t)out->num_frames * 2);
    out->voiced_mask = malloc((size_t)out->num_frames);
    if (!out->sp || !out->ap || !out->voiced_mask) { worldcache_free_analysis(out); return -1; }
    memset(out->sp, 1, (size_t)out->num_frames * out->fft_size * sizeof(float));
    memset(out->ap, 2, (size_t)out->num_frames * 2);
    memset(out->voiced_mask, 3, (size_t)out->num_frames);
    return 0;
}

int worldcache_get_analysis(const char* wav_path, WORLD_AnalysisData* out_data) {
    if (!wav_path || !out_data) return -1;
    char cache_path[4096];
    snprintf(cache_path, sizeof(cache_path), "%s.worldcache", wav_path);

    /* If cache exists and is valid, load it. If it's stale, remove it. */
    {
        FILE* f = fopen(cache_path, "rb");
        if (f) {
            WorldCacheHeader_t h;
            if (fread(&h, sizeof(h), 1, f) == 1) {
                uint64_t mtime = get_file_mtime(wav_path);
                uint64_t h_mtime = h.wav_mtime;
                uint64_t h_hash = h.wav_hash;
                uint64_t cur_hash = simple_file_hash(wav_path);
                if (h_mtime == mtime && h_hash == cur_hash) {
                    /* cache valid - read entire file to buffer and deserialize */
                    fseek(f, 0, SEEK_END);
                    long fsz = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    if (fsz > (long)sizeof(WorldCacheHeader_t)) {
                        size_t full_sz = (size_t)fsz;
                        uint8_t* full = (uint8_t*)malloc(full_sz);
                        if (full && fread(full, 1, full_sz, f) == full_sz) {
                            fclose(f);
                            uint8_t *sp=NULL,*ap=NULL,*vm=NULL;
                            WorldCacheHeader_t rh; /* read header */
                            if (worldcache_deserialize(full, full_sz, &rh, &sp, &ap, &vm) == 0) {
                                out_data->sample_rate = rh.sample_rate;
                                out_data->frame_period_ms = rh.frame_period_ms;
                                out_data->num_frames = rh.num_frames;
                                out_data->fft_size = rh.fft_size;
                                out_data->sp = sp;
                                out_data->ap = ap;
                                out_data->voiced_mask = vm;
                                free(full);
                                return 0;
                            }
                            free(full);
                        } else {
                            if (full) free(full);
                            fclose(f);
                        }
                    } else {
                        fclose(f);
                    }
                } else {
                    /* stale cache - remove it */
                    fclose(f);
                    remove(cache_path);
                }
            } else {
                fclose(f);
            }
        }
    }

    /* No valid cache - perform analysis */
    if (world_analyze_wav_stub(wav_path, out_data) != 0) return -1;

    /* create header and serialize/save */
    WorldCacheHeader_t h;
    worldcache_header_init(&h);
    h.sample_rate = out_data->sample_rate;
    h.frame_period_ms = out_data->frame_period_ms;
    h.num_frames = out_data->num_frames;
    h.fft_size = out_data->fft_size;
    /* sizes - this is simplified, real code should calculate bytes precisely */
    h.sp_size = (uint32_t)(out_data->num_frames * out_data->fft_size * sizeof(float));
    h.ap_size = (uint32_t)(out_data->num_frames * 2);
    h.voiced_mask_size = (uint32_t)(out_data->num_frames);
    h.wav_mtime = get_file_mtime(wav_path);
    h.wav_hash = simple_file_hash(wav_path);

#if defined(USE_ZSTD)
    /* Enable compression when library support is compiled in */
    h.flags |= WORLDCACHE_FLAG_COMPRESSED;
#endif

    uint8_t* buf = NULL; size_t buf_size = 0;
    if (worldcache_serialize(&h, out_data->sp, out_data->ap, out_data->voiced_mask, &buf, &buf_size) != 0) {
        return -1;
    }
    FILE* wf = fopen(cache_path, "wb");
    if (!wf) { free(buf); return -1; }
    fwrite(buf, 1, buf_size, wf);
    fclose(wf);
    free(buf);
    return 0;
}

void worldcache_free_analysis(WORLD_AnalysisData* d) {
    if (!d) return;
    if (d->sp) free(d->sp);
    if (d->ap) free(d->ap);
    if (d->voiced_mask) free(d->voiced_mask);
    d->sp = d->ap = d->voiced_mask = NULL;
}

int worldcache_invalidate_if_changed(const char* wav_path) {
    if (!wav_path) return -1;
    char cache_path[4096];
    snprintf(cache_path, sizeof(cache_path), "%s.worldcache", wav_path);
    FILE* f = fopen(cache_path, "rb");
    if (!f) return 0; /* no cache */
    WorldCacheHeader_t h;
    if (fread(&h, sizeof(h), 1, f) != 1) {
        fclose(f);
        remove(cache_path);
        return 0;
    }
    fclose(f);
    uint64_t cur_mtime = get_file_mtime(wav_path);
    uint64_t cur_hash = simple_file_hash(wav_path);
    if (h.wav_mtime == cur_mtime && h.wav_hash == cur_hash) return 1; /* still valid */
    /* stale */
    remove(cache_path);
    return 0;
}
