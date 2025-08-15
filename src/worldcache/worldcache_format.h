#ifndef WORLDCACHE_FORMAT_H
#define WORLDCACHE_FORMAT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Packed header for .worldcache files. Keep layout stable across compilers/arch.
 * Fields:
 *  - magic: 4-bytes identifier
 *  - format_version: protocol version
 *  - flags: bitflags (e.g., compression)
 *  - sample_rate, frame_period_ms: doubles
 *  - wav_hash: simple 64-bit hash of source WAV (placeholder)
 *  - wav_mtime: file modification time (seconds since epoch)
 *  - num_frames, fft_size: dims used by sp/ap
 *  - sp_size, ap_size, voiced_mask_size: sizes of subsequent data blocks in bytes
 */

#pragma pack(push,1)
typedef struct {
    uint32_t magic;           /* 'WCH1' */
    uint16_t format_version;  /* format/protocol version */
    uint16_t flags;           /* bitflags: bit0 = compressed */
    double   sample_rate;     /* e.g., 44100.0 */
    double   frame_period_ms; /* hop size in ms */
    uint64_t wav_hash;        /* simple content/hash placeholder */
    uint64_t wav_mtime;       /* source file mtime */
    uint32_t num_frames;      /* number of frames */
    uint32_t fft_size;        /* fft size / num bins for sp */
    uint32_t sp_size;         /* size in bytes of spectral block */
    uint32_t ap_size;         /* size in bytes of aperiodicity block */
    uint32_t voiced_mask_size;/* size in bytes of voiced mask block */
} WorldCacheHeader_t;
#pragma pack(pop)

/* magic value 'WCH1' little-endian */
#define WORLDCACHE_MAGIC 0x31484357U

/* flag bits */
#define WORLDCACHE_FLAG_COMPRESSED 0x1

/* helpers */
void worldcache_header_init(WorldCacheHeader_t* h);

/* helper to test if header indicates compression */
static inline int worldcache_header_is_compressed(const WorldCacheHeader_t* h) { return (h->flags & WORLDCACHE_FLAG_COMPRESSED) != 0; }

#ifdef __cplusplus
}
#endif

#endif /* WORLDCACHE_FORMAT_H */
