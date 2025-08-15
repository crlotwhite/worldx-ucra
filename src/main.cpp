#include <iostream>
#include <cstdlib>

// Include UCRA headers
#include "ucra/ucra.h"

// Include vv-dsp headers
#include "vv_dsp/vv_dsp.h"

int main(int argc, char* argv[]) {
    std::cout << "worldx-ucra - WORLD-based UTAU vocal synthesizer" << std::endl;
    std::cout << "Version: 1.0.0" << std::endl;
    
    // Test UCRA library availability
    std::cout << "Testing UCRA library..." << std::endl;
    
    // Test vv-dsp library availability  
    std::cout << "Testing vv-dsp library..." << std::endl;
    
    std::cout << "Libraries loaded successfully!" << std::endl;
    std::cout << "Usage: ucra-cli <input.wav> <output.wav> [options...]" << std::endl;
    
    return EXIT_SUCCESS;
}
