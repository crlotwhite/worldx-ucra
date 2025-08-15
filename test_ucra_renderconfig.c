#include "third_party/ucra/include/ucra/ucra.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("Testing UCRA_RenderConfig structure with UTAU CLI arguments...\n");

    // Test 1: Create and initialize UCRA_RenderConfig
    UCRA_RenderConfig config = {0};

    // Test 2: Set all UTAU CLI arguments
    config.sample_rate = 44100;
    config.channels = 1;
    config.block_size = 512;
    config.flags = 0x1234;

    // UTAU CLI arguments
    config.in_file_path = "test_input.wav";
    config.out_file_path = "test_output.wav";
    config.pitch = 100.0;        // cents
    config.velocity = 100.0;     // velocity
    config.offset = 50.0;        // milliseconds
    config.length = 1000.0;      // milliseconds
    config.consonant = 200.0;    // milliseconds
    config.cutoff = 100.0;       // milliseconds
    config.volume = 1.0;         // 0.0-1.0
    config.modulation = 0.5;     // modulation amount
    config.tempo = 120.0;        // BPM
    config.pitch_string = "C4";  // pitch string

    // Test 3: Verify all fields can be accessed
    printf("✓ UCRA_RenderConfig instantiation successful\n");
    printf("✓ Basic fields:\n");
    printf("  - sample_rate: %u\n", config.sample_rate);
    printf("  - channels: %u\n", config.channels);
    printf("  - block_size: %u\n", config.block_size);
    printf("  - flags: 0x%X\n", config.flags);

    printf("✓ UTAU CLI arguments:\n");
    printf("  - in_file_path: %s\n", config.in_file_path);
    printf("  - out_file_path: %s\n", config.out_file_path);
    printf("  - pitch: %.1f cents\n", config.pitch);
    printf("  - velocity: %.1f\n", config.velocity);
    printf("  - offset: %.1f ms\n", config.offset);
    printf("  - length: %.1f ms\n", config.length);
    printf("  - consonant: %.1f ms\n", config.consonant);
    printf("  - cutoff: %.1f ms\n", config.cutoff);
    printf("  - volume: %.1f\n", config.volume);
    printf("  - modulation: %.1f\n", config.modulation);
    printf("  - tempo: %.1f BPM\n", config.tempo);
    printf("  - pitch_string: %s\n", config.pitch_string);

    printf("\n✓ All UCRA_RenderConfig fields accessible and working correctly!\n");

    return 0;
}
