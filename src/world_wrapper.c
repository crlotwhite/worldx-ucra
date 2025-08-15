/**
 * @file world_wrapper.c
 * @brief WORLD library wrapper implementation
 */

#include "world_wrapper.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// WORLD library headers
#include "world/harvest.h"
#include "world/cheaptrick.h"
#include "world/d4c.h"
#include "world/synthesis.h"
#include "world/common.h"

void world_analysis_data_init(WorldAnalysisData* data) {
    if (!data) return;

    data->f0 = NULL;
    data->f0_length = 0;
    data->spectrogram = NULL;
    data->sp_length = 0;
    data->fft_size = 0;
    data->aperiodicity = NULL;
    data->ap_length = 0;
    data->temporal_positions = NULL;
    data->frame_period = 0.0;
    data->sample_rate = 0;
    data->x_length = 0;
}

void world_analysis_data_free(WorldAnalysisData* data) {
    if (!data) return;

    // Free F0 array
    if (data->f0) {
        free(data->f0);
        data->f0 = NULL;
    }

    // Free temporal positions
    if (data->temporal_positions) {
        free(data->temporal_positions);
        data->temporal_positions = NULL;
    }

    // Free spectrogram 2D array
    if (data->spectrogram) {
        for (int i = 0; i < data->sp_length; i++) {
            if (data->spectrogram[i]) {
                free(data->spectrogram[i]);
            }
        }
        free(data->spectrogram);
        data->spectrogram = NULL;
    }

    // Free aperiodicity 2D array
    if (data->aperiodicity) {
        for (int i = 0; i < data->ap_length; i++) {
            if (data->aperiodicity[i]) {
                free(data->aperiodicity[i]);
            }
        }
        free(data->aperiodicity);
        data->aperiodicity = NULL;
    }

    // Reset all lengths
    data->f0_length = 0;
    data->sp_length = 0;
    data->fft_size = 0;
    data->ap_length = 0;
    data->frame_period = 0.0;
    data->sample_rate = 0;
    data->x_length = 0;
}

int world_analysis_data_allocate(WorldAnalysisData* data, int f0_length, int fft_size) {
    if (!data || f0_length <= 0 || fft_size <= 0) return -1;

    // Free any existing data first
    world_analysis_data_free(data);

    // Allocate F0 array
    data->f0 = (double*)malloc(sizeof(double) * f0_length);
    if (!data->f0) goto allocation_error;

    // Allocate temporal positions
    data->temporal_positions = (double*)malloc(sizeof(double) * f0_length);
    if (!data->temporal_positions) goto allocation_error;

    // Allocate spectrogram 2D array
    data->spectrogram = (double**)malloc(sizeof(double*) * f0_length);
    if (!data->spectrogram) goto allocation_error;

    int spectral_bins = fft_size / 2 + 1;
    for (int i = 0; i < f0_length; i++) {
        data->spectrogram[i] = (double*)malloc(sizeof(double) * spectral_bins);
        if (!data->spectrogram[i]) goto allocation_error;
    }

    // Allocate aperiodicity 2D array
    data->aperiodicity = (double**)malloc(sizeof(double*) * f0_length);
    if (!data->aperiodicity) goto allocation_error;

    for (int i = 0; i < f0_length; i++) {
        data->aperiodicity[i] = (double*)malloc(sizeof(double) * spectral_bins);
        if (!data->aperiodicity[i]) goto allocation_error;
    }

    // Set dimensions
    data->f0_length = f0_length;
    data->sp_length = f0_length;
    data->ap_length = f0_length;
    data->fft_size = fft_size;

    return 0;

allocation_error:
    world_analysis_data_free(data);
    return -1;
}

int world_analyze(const double* x, int x_length, int fs,
                  double frame_period, double f0_floor, double f0_ceil,
                  WorldAnalysisData* data) {
    if (!x || !data || x_length <= 0 || fs <= 0) return -1;

    // Initialize WORLD option structures
    HarvestOption harvest_option;
    InitializeHarvestOption(&harvest_option);
    harvest_option.frame_period = frame_period;
    harvest_option.f0_floor = f0_floor;
    harvest_option.f0_ceil = f0_ceil;

    CheapTrickOption cheaptrick_option;
    InitializeCheapTrickOption(fs, &cheaptrick_option);
    cheaptrick_option.f0_floor = f0_floor;

    D4COption d4c_option;
    InitializeD4COption(&d4c_option);

    // Calculate required array sizes
    int f0_length = GetSamplesForHarvest(fs, x_length, frame_period);
    int fft_size = GetFFTSizeForCheapTrick(fs, &cheaptrick_option);

    // Allocate memory for analysis data
    if (world_analysis_data_allocate(data, f0_length, fft_size) != 0) {
        return -1;
    }

    // Store analysis parameters
    data->frame_period = frame_period;
    data->sample_rate = fs;
    data->x_length = x_length;

    // Perform F0 analysis with Harvest
    Harvest(x, x_length, fs, &harvest_option, data->temporal_positions, data->f0);

    // Perform spectral envelope analysis with CheapTrick
    CheapTrick(x, x_length, fs, data->temporal_positions, data->f0, f0_length,
               &cheaptrick_option, data->spectrogram);

    // Perform aperiodicity analysis with D4C
    D4C(x, x_length, fs, data->temporal_positions, data->f0, f0_length,
        fft_size, &d4c_option, data->aperiodicity);

    return 0;
}

int world_synthesize(const WorldAnalysisData* data, double* y, int y_length) {
    if (!data || !y || y_length <= 0) return -1;
    if (!data->f0 || !data->spectrogram || !data->aperiodicity) return -1;

    // Perform WORLD synthesis
    Synthesis(data->f0, data->f0_length, data->spectrogram, data->aperiodicity,
              data->fft_size, data->frame_period, data->sample_rate,
              y_length, y);

    return 0;
}

int world_generate_dummy_data(WorldAnalysisData* data, double duration_sec,
                              int fs, double frame_period, double base_f0) {
    if (!data || duration_sec <= 0 || fs <= 0 || base_f0 <= 0) return -1;

    // Calculate array dimensions
    int f0_length = (int)(duration_sec * 1000.0 / frame_period) + 1;
    int fft_size = 2048; // Default FFT size

    // Allocate memory
    if (world_analysis_data_allocate(data, f0_length, fft_size) != 0) {
        return -1;
    }

    // Set analysis parameters
    data->frame_period = frame_period;
    data->sample_rate = fs;
    data->x_length = (int)(duration_sec * fs);

    int spectral_bins = fft_size / 2 + 1;

    // Generate dummy data
    for (int i = 0; i < f0_length; i++) {
        // Time position
        data->temporal_positions[i] = i * frame_period / 1000.0;

        // F0 with slight vibrato
        double vibrato = sin(2.0 * M_PI * 5.0 * data->temporal_positions[i]) * 10.0;
        data->f0[i] = base_f0 + vibrato;

        // Generate dummy spectral envelope (simple formant-like structure)
        for (int j = 0; j < spectral_bins; j++) {
            double freq = (double)j * fs / (2.0 * (spectral_bins - 1));
            double magnitude = 0.0;

            // Simple formant simulation
            double formant1 = 800.0, formant2 = 1200.0, formant3 = 2400.0;
            double bw = 100.0; // Bandwidth

            magnitude += exp(-0.5 * pow((freq - formant1) / bw, 2.0));
            magnitude += exp(-0.5 * pow((freq - formant2) / bw, 2.0)) * 0.8;
            magnitude += exp(-0.5 * pow((freq - formant3) / bw, 2.0)) * 0.6;

            // Apply high-frequency rolloff
            magnitude *= exp(-freq / 8000.0);

            data->spectrogram[i][j] = magnitude + 0.001; // Add small floor
        }

        // Generate dummy aperiodicity (mostly periodic)
        for (int j = 0; j < spectral_bins; j++) {
            double freq = (double)j * fs / (2.0 * (spectral_bins - 1));
            // More aperiodicity at high frequencies
            data->aperiodicity[i][j] = 0.1 + 0.3 * (freq / (fs / 2.0));
            if (data->aperiodicity[i][j] > 0.99) data->aperiodicity[i][j] = 0.99;
        }
    }

    return 0;
}
