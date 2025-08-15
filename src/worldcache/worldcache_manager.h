#ifndef WORLDCACHE_MANAGER_H
#define WORLDCACHE_MANAGER_H

#include "worldcache_format.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal placeholder for WORLD_AnalysisData - real project may define a richer struct. */
typedef struct {
    uint32_t num_frames;
    uint32_t fft_size;
    double sample_rate;
    double frame_period_ms;
    uint8_t* sp; /* raw bytes */
    uint8_t* ap;
    uint8_t* voiced_mask;
} WORLD_AnalysisData;

/* Orchestrate analysis + cache: fills out_data and returns 0 on success. */
int worldcache_get_analysis(const char* wav_path, WORLD_AnalysisData* out_data);

/* Free a WORLD_AnalysisData allocated by analyze or deserialize */
void worldcache_free_analysis(WORLD_AnalysisData* d);

/* Remove or mark cache invalid if WAV changed. Returns 0 if cache removed or not present, 1 if cache still valid, -1 on error. */
int worldcache_invalidate_if_changed(const char* wav_path);

#ifdef __cplusplus
}
#endif

#endif /* WORLDCACHE_MANAGER_H */
