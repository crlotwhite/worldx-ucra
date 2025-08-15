/**
 * @file world_wrapper.h
 * @brief WORLD library wrapper for UCRA integration
 * @author worldx-ucra development team
 * @date 2025
 *
 * This header defines wrapper structures and functions for integrating
 * the WORLD vocoder library with the UCRA rendering engine.
 */
#ifndef WORLDX_UCRA_WORLD_WRAPPER_H
#define WORLDX_UCRA_WORLD_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/**
 * @brief WORLD analysis data container
 *
 * Encapsulates the results of WORLD analysis: F0 (fundamental frequency),
 * SP (spectral envelope), and AP (aperiodicity). All arrays are dynamically
 * allocated and must be freed with world_analysis_data_free().
 */
typedef struct {
    /** F0 contour in Hz */
    double* f0;
    /** Number of F0 frames */
    int f0_length;

    /** Spectral envelope (2D array: [frame][frequency_bin]) */
    double** spectrogram;
    /** Number of spectral frames */
    int sp_length;
    /** FFT size used for spectral analysis */
    int fft_size;

    /** Aperiodicity (2D array: [frame][frequency_bin]) */
    double** aperiodicity;
    /** Number of aperiodicity frames */
    int ap_length;

    /** Temporal positions for each frame (in seconds) */
    double* temporal_positions;

    /** Analysis parameters */
    double frame_period;  /**< Frame period in milliseconds */
    int sample_rate;      /**< Original sample rate */
    int x_length;         /**< Original signal length */
} WorldAnalysisData;

/**
 * @brief Initialize WorldAnalysisData structure
 *
 * Initializes all pointers to NULL and lengths to 0.
 * Must be called before using the structure.
 *
 * @param data Pointer to WorldAnalysisData structure to initialize
 */
void world_analysis_data_init(WorldAnalysisData* data);

/**
 * @brief Free all memory allocated in WorldAnalysisData
 *
 * Frees all dynamically allocated arrays and resets the structure.
 * Safe to call multiple times on the same structure.
 *
 * @param data Pointer to WorldAnalysisData structure to free
 */
void world_analysis_data_free(WorldAnalysisData* data);

/**
 * @brief Allocate memory for WorldAnalysisData arrays
 *
 * Allocates memory for F0, spectrogram, aperiodicity, and temporal_positions
 * arrays based on the provided dimensions.
 *
 * @param data Pointer to WorldAnalysisData structure
 * @param f0_length Number of F0 frames
 * @param fft_size FFT size for spectral analysis
 * @return 0 on success, -1 on memory allocation failure
 */
int world_analysis_data_allocate(WorldAnalysisData* data, int f0_length, int fft_size);

/**
 * @brief Perform WORLD analysis on input audio signal
 *
 * Performs complete WORLD analysis (Harvest + CheapTrick + D4C) on the
 * input audio signal and stores results in WorldAnalysisData structure.
 *
 * @param x Input audio signal
 * @param x_length Length of input signal
 * @param fs Sample rate
 * @param frame_period Frame period in milliseconds (default: 5.0)
 * @param f0_floor Lower F0 limit in Hz (default: 71.0)
 * @param f0_ceil Upper F0 limit in Hz (default: 800.0)
 * @param data Output WorldAnalysisData structure (must be initialized)
 * @return 0 on success, -1 on failure
 */
int world_analyze(const double* x, int x_length, int fs,
                  double frame_period, double f0_floor, double f0_ceil,
                  WorldAnalysisData* data);

/**
 * @brief Perform WORLD synthesis from analysis data
 *
 * Synthesizes audio from F0, spectrogram, and aperiodicity data using
 * the WORLD synthesis function.
 *
 * @param data WorldAnalysisData containing synthesis parameters
 * @param y Output audio buffer (must be pre-allocated)
 * @param y_length Length of output buffer
 * @return 0 on success, -1 on failure
 */
int world_synthesize(const WorldAnalysisData* data, double* y, int y_length);

/**
 * @brief Generate dummy WORLD analysis data for testing
 *
 * Creates dummy F0, spectral, and aperiodicity data for synthesis testing.
 * Useful for integration testing without requiring actual audio analysis.
 *
 * @param data WorldAnalysisData structure to fill with dummy data
 * @param duration_sec Duration of dummy audio in seconds
 * @param fs Sample rate
 * @param frame_period Frame period in milliseconds
 * @param base_f0 Base fundamental frequency in Hz
 * @return 0 on success, -1 on failure
 */
int world_generate_dummy_data(WorldAnalysisData* data, double duration_sec,
                              int fs, double frame_period, double base_f0);

#ifdef __cplusplus
}
#endif

#endif /* WORLDX_UCRA_WORLD_WRAPPER_H */
