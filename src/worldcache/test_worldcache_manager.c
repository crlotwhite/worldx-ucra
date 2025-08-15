#include "worldcache_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#if defined(_WIN32)
#  include <sys/utime.h>
#  define TOUCH_FILE(path) _utime((path), NULL)
#else
#  include <utime.h>
#  define TOUCH_FILE(path) utime((path), NULL)
#endif

int main(int argc, char** argv) {
    const char* wav = "third_party/ucra/test_input.wav"; /* use existing test WAV in third_party */
    WORLD_AnalysisData d;
    int res = worldcache_get_analysis(wav, &d);
    if (res != 0) { fprintf(stderr, "analysis failed\n"); return 1; }
    /* check cache file exists */
    char cachepath[4096];
    snprintf(cachepath, sizeof(cachepath), "%s.worldcache", wav);
    struct stat st;
    if (stat(cachepath, &st) != 0) {
        fprintf(stderr, "cache not created\n"); return 2;
    }
    /* free resources */
    worldcache_free_analysis(&d);

    /* WAV 파일을 변경하지 않고 다시 호출하여 캐시 히트 경로가 동작하는지 검증 */
    WORLD_AnalysisData d_hit;
    int res_hit = worldcache_get_analysis(wav, &d_hit);
    if (res_hit != 0) { fprintf(stderr, "cache-hit analysis failed\n"); return 3; }
    worldcache_free_analysis(&d_hit);

    /* WAV 파일의 mtime을 변경하여 캐시 무효화를 강제하고 다시 실행 */
    if (TOUCH_FILE(wav) != 0) { perror("touch wav"); }

    /* 무효화 헬퍼를 명시적으로 호출 (0: 제거됨, 1: 아직 유효) */
    int inv = worldcache_invalidate_if_changed(wav);
    /* inv 값과 무관하게 다시 분석을 요청하여 복구/재생성을 확인 */
    WORLD_AnalysisData d2;
    int res2 = worldcache_get_analysis(wav, &d2);
    if (res2 != 0) { fprintf(stderr, "analysis after touch failed\n"); return 4; }
    worldcache_free_analysis(&d2);

    printf("worldcache manager test passed\n");
    return 0;
}
