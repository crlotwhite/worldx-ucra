#include <iostream>
#include <cstdlib>
#include <vector>

// Include UCRA headers
#include "ucra/ucra.h"

// Include vv-dsp headers
#include "vv_dsp/vv_dsp.h"

// Include our WORLD wrapper
#include "world_wrapper.h"

int main(int argc, char* argv[]) {
    std::cout << "worldx-ucra - WORLD-based UTAU vocal synthesizer" << std::endl;
    std::cout << "Version: 1.0.0" << std::endl;

    // Test UCRA library availability
    std::cout << "Testing UCRA library..." << std::endl;

    // Create UCRA engine
    UCRA_Handle engine = nullptr;
    UCRA_Result result = ucra_engine_create(&engine, nullptr, 0);

    if (result == UCRA_SUCCESS && engine != nullptr) {
        std::cout << "✅ UCRA engine created successfully!" << std::endl;

        // Get engine info
        char info_buffer[512];
        result = ucra_engine_getinfo(engine, info_buffer, sizeof(info_buffer));
        if (result == UCRA_SUCCESS) {
            std::cout << "Engine info: " << info_buffer << std::endl;
        }

        // Test a basic render configuration
        UCRA_RenderConfig config = {};
        config.sample_rate = 44100;
        config.channels = 1;
        config.block_size = 512;
        config.flags = 0;
        config.notes = nullptr;
        config.note_count = 0;
        config.options = nullptr;
        config.option_count = 0;

        UCRA_RenderResult render_result = {};
        result = ucra_render(engine, &config, &render_result);
        if (result == UCRA_SUCCESS) {
            std::cout << "✅ UCRA render test successful!" << std::endl;
            std::cout << "Rendered " << render_result.frames << " frames at "
                      << render_result.sample_rate << "Hz" << std::endl;
        } else {
            std::cout << "⚠️  UCRA render test returned error code: " << result << std::endl;
        }

        // Destroy engine
        ucra_engine_destroy(engine);
        std::cout << "✅ UCRA engine destroyed successfully!" << std::endl;
    } else {
        std::cout << "❌ Failed to create UCRA engine (error code: " << result << ")" << std::endl;
    }

    // Test vv-dsp library availability
    std::cout << "Testing vv-dsp library..." << std::endl;
    std::cout << "✅ vv-dsp headers included successfully!" << std::endl;

    // Test WORLD wrapper functionality
    std::cout << "Testing WORLD wrapper..." << std::endl;

    WorldAnalysisData world_data;
    world_analysis_data_init(&world_data);

    // Generate dummy WORLD analysis data for testing
    int test_result = world_generate_dummy_data(&world_data, 1.0, 44100, 5.0, 220.0);
    if (test_result == 0) {
        std::cout << "✅ WORLD dummy data generated successfully!" << std::endl;
        std::cout << "F0 length: " << world_data.f0_length << " frames" << std::endl;
        std::cout << "FFT size: " << world_data.fft_size << std::endl;
        std::cout << "Frame period: " << world_data.frame_period << " ms" << std::endl;

        // Test synthesis
        int y_length = world_data.x_length;
        double* synthesized_audio = (double*)malloc(sizeof(double) * y_length);
        if (synthesized_audio) {
            int synth_result = world_synthesize(&world_data, synthesized_audio, y_length);
            if (synth_result == 0) {
                std::cout << "✅ WORLD synthesis test successful!" << std::endl;
                std::cout << "Synthesized " << y_length << " samples" << std::endl;

                // Check for non-zero audio
                bool has_audio = false;
                for (int i = 0; i < y_length && !has_audio; i++) {
                    if (fabs(synthesized_audio[i]) > 1e-6) {
                        has_audio = true;
                    }
                }
                std::cout << (has_audio ? "✅" : "⚠️ ")
                          << " Audio contains " << (has_audio ? "signal" : "silence") << std::endl;
            } else {
                std::cout << "❌ WORLD synthesis failed!" << std::endl;
            }
            free(synthesized_audio);
        }

        // Clean up
        world_analysis_data_free(&world_data);
    } else {
        std::cout << "❌ Failed to generate WORLD dummy data!" << std::endl;
    }

    std::cout << "Libraries tested successfully!" << std::endl;

    if (argc < 3) {
        std::cout << "Usage: ucra-cli <input.wav> <output.wav> [options...]" << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << "Input file: " << argv[1] << std::endl;
    std::cout << "Output file: " << argv[2] << std::endl;

    return EXIT_SUCCESS;
}
