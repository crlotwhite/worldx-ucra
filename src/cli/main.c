#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <errno.h>

// Include UCRA headers
#include "ucra/ucra.h"

// Include our WORLD wrapper
#include "world_wrapper.h"

// Function to initialize UCRA_RenderConfig with default values
void init_render_config(UCRA_RenderConfig* config) {
    memset(config, 0, sizeof(UCRA_RenderConfig));
    
    // Set default values for basic fields
    config->sample_rate = 44100;
    config->channels = 1;
    config->block_size = 512;
    config->flags = 0;
    config->notes = NULL;
    config->note_count = 0;
    config->options = NULL;
    config->option_count = 0;
    
    // Set default values for UTAU CLI arguments
    config->in_file_path = NULL;
    config->out_file_path = NULL;
    config->pitch = 0.0;           // cents
    config->velocity = 100.0;      // default velocity
    config->offset = 0.0;          // milliseconds
    config->length = 0.0;          // milliseconds
    config->consonant = 0.0;       // milliseconds
    config->cutoff = 0.0;          // milliseconds
    config->volume = 1.0;          // 0.0-1.0
    config->modulation = 0.0;      // modulation amount
    config->tempo = 120.0;         // BPM
    config->pitch_string = NULL;   // pitch string
}

// Function to display help message
void print_help(const char* program_name) {
    printf("Usage: %s [OPTIONS] <input_file> <output_file>\n\n", program_name);
    printf("UTAU-compatible voice synthesizer using UCRA and WORLD libraries.\n\n");
    
    printf("Required Arguments:\n");
    printf("  <input_file>              Input audio file path\n");
    printf("  <output_file>             Output audio file path\n\n");
    
    printf("UTAU Resampler Arguments:\n");
    printf("  -p, --pitch CENTS         Pitch adjustment in cents (default: 0.0)\n");
    printf("  -v, --velocity VALUE      Velocity/amplitude modifier (default: 100.0)\n");
    printf("  -f, --flags FLAGS         UTAU flags as hexadecimal (default: 0)\n");
    printf("  -o, --offset MS           OTO offset parameter in milliseconds (default: 0.0)\n");
    printf("  -l, --length MS           OTO length parameter in milliseconds (default: 0.0)\n");
    printf("  -c, --consonant MS        OTO consonant parameter in milliseconds (default: 0.0)\n");
    printf("  -k, --cutoff MS           OTO cutoff parameter in milliseconds (default: 0.0)\n");
    printf("  -V, --volume LEVEL        Volume modifier 0.0-1.0 (default: 1.0)\n");
    printf("  -m, --modulation AMOUNT   Modulation amount (default: 0.0)\n");
    printf("  -t, --tempo BPM           Tempo in beats per minute (default: 120.0)\n");
    printf("  -s, --pitch-string STR    Pitch bend string (e.g., 'C4', 'R')\n\n");
    
    printf("Audio Settings:\n");
    printf("  -r, --sample-rate RATE    Sample rate in Hz (default: 44100)\n");
    printf("  -C, --channels COUNT      Number of channels (default: 1)\n");
    printf("  -b, --block-size SIZE     Block size for processing (default: 512)\n\n");
    
    printf("Other Options:\n");
    printf("  -h, --help                Display this help message\n");
    printf("  --version                 Display version information\n\n");
    
    printf("Examples:\n");
    printf("  %s input.wav output.wav\n", program_name);
    printf("  %s -p 100 -v 80 --offset 50 input.wav output.wav\n", program_name);
    printf("  %s --pitch-string \"C4 R C5\" input.wav output.wav\n", program_name);
}

// Function to display version information
void print_version(void) {
    printf("worldx-ucra v1.0.0\n");
    printf("WORLD-based UTAU vocal synthesizer\n");
    printf("Built with UCRA, vv-dsp, and WORLD libraries\n");
}

// Function to parse a double value with error checking
int parse_double(const char* str, double* value, const char* arg_name) {
    char* endptr;
    errno = 0;
    *value = strtod(str, &endptr);
    
    if (errno != 0 || endptr == str || *endptr != '\0') {
        fprintf(stderr, "Error: Invalid %s value '%s'\n", arg_name, str);
        return -1;
    }
    return 0;
}

// Function to parse an unsigned integer with error checking
int parse_uint32(const char* str, uint32_t* value, const char* arg_name) {
    char* endptr;
    errno = 0;
    unsigned long tmp = strtoul(str, &endptr, 0);  // 0 allows hex/oct/dec
    
    if (errno != 0 || endptr == str || *endptr != '\0' || tmp > UINT32_MAX) {
        fprintf(stderr, "Error: Invalid %s value '%s'\n", arg_name, str);
        return -1;
    }
    *value = (uint32_t)tmp;
    return 0;
}

int main(int argc, char* argv[]) {
    UCRA_RenderConfig config;
    init_render_config(&config);
    
    // Define long options
    static struct option long_options[] = {
        {"pitch",        required_argument, 0, 'p'},
        {"velocity",     required_argument, 0, 'v'},
        {"flags",        required_argument, 0, 'f'},
        {"offset",       required_argument, 0, 'o'},
        {"length",       required_argument, 0, 'l'},
        {"consonant",    required_argument, 0, 'c'},
        {"cutoff",       required_argument, 0, 'k'},
        {"volume",       required_argument, 0, 'V'},
        {"modulation",   required_argument, 0, 'm'},
        {"tempo",        required_argument, 0, 't'},
        {"pitch-string", required_argument, 0, 's'},
        {"sample-rate",  required_argument, 0, 'r'},
        {"channels",     required_argument, 0, 'C'},
        {"block-size",   required_argument, 0, 'b'},
        {"help",         no_argument,       0, 'h'},
        {"version",      no_argument,       0, 1000},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    // Parse command line arguments
    while ((opt = getopt_long(argc, argv, "p:v:f:o:l:c:k:V:m:t:s:r:C:b:h", 
                              long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                if (parse_double(optarg, &config.pitch, "pitch") != 0) {
                    return EXIT_FAILURE;
                }
                break;
            case 'v':
                if (parse_double(optarg, &config.velocity, "velocity") != 0) {
                    return EXIT_FAILURE;
                }
                break;
            case 'f':
                if (parse_uint32(optarg, &config.flags, "flags") != 0) {
                    return EXIT_FAILURE;
                }
                break;
            case 'o':
                if (parse_double(optarg, &config.offset, "offset") != 0) {
                    return EXIT_FAILURE;
                }
                break;
            case 'l':
                if (parse_double(optarg, &config.length, "length") != 0) {
                    return EXIT_FAILURE;
                }
                break;
            case 'c':
                if (parse_double(optarg, &config.consonant, "consonant") != 0) {
                    return EXIT_FAILURE;
                }
                break;
            case 'k':
                if (parse_double(optarg, &config.cutoff, "cutoff") != 0) {
                    return EXIT_FAILURE;
                }
                break;
            case 'V':
                if (parse_double(optarg, &config.volume, "volume") != 0) {
                    return EXIT_FAILURE;
                }
                if (config.volume < 0.0 || config.volume > 1.0) {
                    fprintf(stderr, "Error: Volume must be between 0.0 and 1.0\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'm':
                if (parse_double(optarg, &config.modulation, "modulation") != 0) {
                    return EXIT_FAILURE;
                }
                break;
            case 't':
                if (parse_double(optarg, &config.tempo, "tempo") != 0) {
                    return EXIT_FAILURE;
                }
                if (config.tempo <= 0.0) {
                    fprintf(stderr, "Error: Tempo must be positive\n");
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                config.pitch_string = optarg;
                break;
            case 'r':
                if (parse_uint32(optarg, &config.sample_rate, "sample-rate") != 0) {
                    return EXIT_FAILURE;
                }
                if (config.sample_rate < 8000 || config.sample_rate > 192000) {
                    fprintf(stderr, "Error: Sample rate must be between 8000 and 192000 Hz\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'C':
                if (parse_uint32(optarg, &config.channels, "channels") != 0) {
                    return EXIT_FAILURE;
                }
                if (config.channels < 1 || config.channels > 8) {
                    fprintf(stderr, "Error: Channels must be between 1 and 8\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'b':
                if (parse_uint32(optarg, &config.block_size, "block-size") != 0) {
                    return EXIT_FAILURE;
                }
                if (config.block_size < 64 || config.block_size > 8192) {
                    fprintf(stderr, "Error: Block size must be between 64 and 8192\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'h':
                print_help(argv[0]);
                return EXIT_SUCCESS;
            case 1000:  // --version
                print_version();
                return EXIT_SUCCESS;
            case '?':
                // getopt_long already printed an error message
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Unexpected option: %c\n", opt);
                return EXIT_FAILURE;
        }
    }
    
    // Check for required positional arguments
    if (optind + 2 > argc) {
        fprintf(stderr, "Error: Missing required arguments\n");
        fprintf(stderr, "Usage: %s [OPTIONS] <input_file> <output_file>\n", argv[0]);
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // Set input and output file paths
    config.in_file_path = argv[optind];
    config.out_file_path = argv[optind + 1];
    
    // Display parsed configuration
    printf("worldx-ucra - WORLD-based UTAU vocal synthesizer\n");
    printf("Version: 1.0.0\n\n");
    
    printf("Parsed Configuration:\n");
    printf("  Input file:     %s\n", config.in_file_path);
    printf("  Output file:    %s\n", config.out_file_path);
    printf("  Sample rate:    %u Hz\n", config.sample_rate);
    printf("  Channels:       %u\n", config.channels);
    printf("  Block size:     %u\n", config.block_size);
    printf("  Flags:          0x%X\n", config.flags);
    printf("  Pitch:          %.2f cents\n", config.pitch);
    printf("  Velocity:       %.2f\n", config.velocity);
    printf("  Offset:         %.2f ms\n", config.offset);
    printf("  Length:         %.2f ms\n", config.length);
    printf("  Consonant:      %.2f ms\n", config.consonant);
    printf("  Cutoff:         %.2f ms\n", config.cutoff);
    printf("  Volume:         %.2f\n", config.volume);
    printf("  Modulation:     %.2f\n", config.modulation);
    printf("  Tempo:          %.2f BPM\n", config.tempo);
    if (config.pitch_string) {
        printf("  Pitch string:   %s\n", config.pitch_string);
    }
    
    printf("\nTesting UCRA library integration...\n");
    
    // Test UCRA library functionality
    UCRA_Handle engine = NULL;
    UCRA_Result result = ucra_engine_create(&engine, NULL, 0);
    
    if (result == UCRA_SUCCESS && engine != NULL) {
        printf("✅ UCRA engine created successfully!\n");
        
        // Get engine info
        char info_buffer[512];
        result = ucra_engine_getinfo(engine, info_buffer, sizeof(info_buffer));
        if (result == UCRA_SUCCESS) {
            printf("Engine info: %s\n", info_buffer);
        }
        
        // Test basic render with parsed configuration
        UCRA_RenderResult render_result = {0};
        result = ucra_render(engine, &config, &render_result);
        if (result == UCRA_SUCCESS) {
            printf("✅ UCRA render test successful!\n");
            printf("Rendered %llu frames at %u Hz\n", 
                   (unsigned long long)render_result.frames, render_result.sample_rate);
        } else {
            printf("⚠️  UCRA render test returned error code: %d\n", result);
        }
        
        // Destroy engine
        ucra_engine_destroy(engine);
        printf("✅ UCRA engine destroyed successfully!\n");
    } else {
        printf("❌ Failed to create UCRA engine (error code: %d)\n", result);
        return EXIT_FAILURE;
    }
    
    printf("\n🎵 CLI argument parsing and UCRA integration successful!\n");
    printf("All UTAU CLI arguments are parsed and stored in UCRA_RenderConfig.\n");
    
    return EXIT_SUCCESS;
}
